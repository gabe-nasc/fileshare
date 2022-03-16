#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>

#include "biblioteca.h"

#define BUFFER_SIZE 500

typedef struct DownloadArgument_t
{

    int socket;
    int option;
    char *filename;

} DownloadArguments;

unsigned char *get_array_slice(unsigned char *array, int start, int end)
{
    unsigned char *slice = (unsigned char *)calloc(BUFFER_SIZE, sizeof(unsigned char));
    for (int i = start; i < end; i++)
    {
        slice[i - start] = array[i];
    }

    return slice;
};

void sendInt(int number, int socket)
{
    int bytes_sent = send(socket, &number, sizeof(int), 0);
    while (bytes_sent < sizeof(int))
    {
        bytes_sent += send(socket, &number + bytes_sent, sizeof(int) - bytes_sent, 0);
    }

    // printf("Sent %d bytes\n", bytes_sent);
};

int recvInt(int socket)
{
    int number;
    int bytes_received = recv(socket, &number, sizeof(int), 0);
    while (bytes_received < sizeof(int))
    {
        bytes_received += recv(socket, &number + bytes_received, sizeof(int) - bytes_received, 0);
    }

    // printf("Received %d bytes\n", bytes_received);
    return number;
};

void sendDouble(double number, int socket)
{
    int bytes_sent = send(socket, &number, sizeof(double), 0);
    while (bytes_sent < sizeof(double))
    {
        bytes_sent += send(socket, &number + bytes_sent, sizeof(double) - bytes_sent, 0);
    }

    // printf("Sent %d bytes\n", bytes_sent);
};

double recvDouble(int socket)
{

    double number;

    int bytes_sent = recv(socket, &number, sizeof(double), 0);
    while (bytes_sent < sizeof(double))
    {
        bytes_sent += recv(socket, &number + bytes_sent, sizeof(double) - bytes_sent, 0);
    }

    // printf("Received %d bytes\n", bytes_sent);
    return number;
};

void sendString(char *string, int socket)
{
    int size = strlen(string);
    sendInt(size, socket);

    int bytes_sent = send(socket, string, sizeof(char) * size, 0);
    while (bytes_sent < sizeof(char) * size)
    {
        bytes_sent += send(socket, string + bytes_sent, sizeof(char) * size - bytes_sent, 0);
    }

    // printf("Sent %d bytes\n", bytes_sent);
};

char *recvString(int socket)
{
    int size = recvInt(socket);
    char *string = (char *)calloc(size, sizeof(char));

    int bytes_received = recv(socket, string, size, 0);
    // printf("Received %d bytes\n", bytes_received);
    while (bytes_received < size)
    {
        bytes_received += recv(socket, string + bytes_received, size - bytes_received, 0);
        // printf("Received %d bytes\n", bytes_received);
    }

    string[size] = '\0';

    return string;
};

void sendVoid(void *voidPointer, int nbytes, int socket)
{
    sendInt(nbytes, socket);
    int bytes_sent = send(socket, voidPointer, nbytes, 0);
    while (bytes_sent < nbytes)
    {
        bytes_sent += send(socket, voidPointer + bytes_sent, nbytes - bytes_sent, 0);
    }

    // printf("Sent %d bytes\n", bytes_sent);
    // send(socket, voidPointer, nbytes, 0);
};

void *recvVoid(int socket)
{
    int nbytes = recvInt(socket);
    void *voidPointer = (void *)calloc(nbytes, sizeof(void));

    int bytes_received = recv(socket, voidPointer, nbytes, 0);
    // printf("Received %d bytes\n", bytes_received);
    while (bytes_received < nbytes)
    {
        bytes_received += recv(socket, voidPointer + bytes_received, nbytes - bytes_received, 0);
        // printf("Received %d bytes\n", bytes_received);
    }

    return voidPointer;
};

void list_directory(char *path)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            printf("%s\n", ent->d_name);
        }
        closedir(dir);
    }
    else
    {
        perror("");
        return;
    }
};

// Send a list of files present in PATH
void send_files_list(char *path, int socket)
{
    DIR *dir;
    struct dirent *ent;

    int files_count = 0;
    if ((dir = opendir(path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_REG)
            {
                files_count++;
            }
        }
        closedir(dir);
    }
    else
    {
        perror("");
        return;
    }

    sendInt(files_count, socket);

    if ((dir = opendir(path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            printf("%s\n", ent->d_name);
            sendVoid(ent->d_name, strlen(ent->d_name) * sizeof(char), socket);
        }
        closedir(dir);
        sendVoid(0, sizeof(int), socket);
    }
    else
    {
        perror("");
        return;
    }
};

void list_server_files(int socket)
{
    sendInt(4, socket); // 4 = list_server_files
    int nfiles = recvInt(socket);
    printf("%d files\n", nfiles);
    for (int i = 0; i < nfiles; i++)
    {
        char *filename = recvVoid(socket);
        printf("%s\n", filename);
        free(filename);
    }
};

void write_file(char *path, unsigned char *content)
{
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        perror("Oops! File could not be opened.\n");
        return;
    }

    fwrite(content, 1, strlen(content), file);

    fclose(file);
};
// Will use pread instead of fread to read file contents because fread will read the whole file into memory
// and pread will read the file contents in chunks, also, fread is not thread safe
void altSendFile(char *path, int socket)
{
    printf("Sending file %s\n", path);

    // Open file and get file size, don't know how to do this without fread/fseek
    FILE *file_tmp = fopen(path, "r");
    if (file_tmp == NULL)
    {
        perror("SHIT");
        return;
    }

    fseek(file_tmp, 0, SEEK_END);
    unsigned int nbytes = ftell(file_tmp);
    printf("Size: %d\n", nbytes);
    pclose(file_tmp);

    // Send file path to receiver
    sendString(path, socket);

    // Allocate memory for a slice of the file contents, the size of the slice is determined by BUFFER_SIZE
    void *content_slice = (void *)calloc(BUFFER_SIZE + 1, sizeof(void));

    // Calculate number of slices
    unsigned int nslices = (nbytes / BUFFER_SIZE) + 1;
    printf("Slices: %d\n", nslices);

    // Send the total file size to receiver
    sendInt(nbytes, socket);

    // Send number of slices to receiver
    sendInt(nslices, socket);

    int file = open(path, O_RDONLY);
    // For each slice, send slice to receiver
    for (size_t i = 0; i < nslices; i++)
    {
        pread(file, content_slice, BUFFER_SIZE, BUFFER_SIZE * i);
        // printf("Sending slice %ld\n", i);
        sendInt(i, socket);
        sendVoid(content_slice, BUFFER_SIZE, socket);
    }

    close(file);
}

char *altRecvFile(int socket)
{
    char *file_path = recvString(socket);
    file_path = strcat(file_path, ".tmp");
    printf("Receiving file %s\n", file_path);

    int totalFileSize = recvInt(socket);
    int nslices = recvInt(socket);

    int file = open(file_path, O_CREAT | O_RDWR, S_IRWXU);
    for (size_t i = 0; i < nslices; i++)
    {
        // printf("Receiving slice %ld\n", i);
        int slice_n = recvInt(socket);
        void *content_slice = recvVoid(socket);

        if (i == nslices - 1)
        {
            printf("Writing last slice\n");
            printf("Size: %d\n", totalFileSize % BUFFER_SIZE);
            ssize_t response = pwrite(file, content_slice, totalFileSize % BUFFER_SIZE, BUFFER_SIZE * slice_n);
            printf("Response: %ld\n", response);
            printf("Slice written\n");
        }
        else
        {
            pwrite(file, content_slice, BUFFER_SIZE, BUFFER_SIZE * slice_n);
            // printf("Slice written\n");
        }
    }
    printf("File received\n");
    printf("fd: %d\n", file);
    close(file);

    return file_path;
};

// char *path, int socket, int option
void *download_file(void *ptr)
{
    DownloadArguments *args = (DownloadArguments *)ptr;

    printf("Downloading file %s\n", args->filename);
    sendInt(args->option, args->socket);
    printf("Sent option\n");
    sendString(args->filename, args->socket);
    printf("Sent filename\n");
    printf("SOQUETE: %d\n", args->socket);
    char *file_path = altRecvFile(args->socket);
    printf("File received\n");
    printf("fd: %s\n", file_path);
    free(file_path);
    close(args->socket);

    return (void *)NULL;
};

void upload_file(char *path, int socket, int option)
{
    sendInt(option, socket);
    altSendFile(path, socket);
    close(socket);
};

// TODO: Descobrir o filesize
// TODO: Criar arquivo em branco com este tamanho
// TODO: Utilizar o pwrite (Uma thread por parte do arquivo)

/*
Faz um PREAD e ponteiro se move N Bytes
No loop sempre fazer um rewind para voltar o ponteiro ao inicio
Controlar a posição do ponteiro, movendo sempre em incrementos de BUFFER_SIZE
Usar 'slices' pra controlar que parte do arquivo será lida

Enviar numero do slice previamente para saber em qual posição dever ser escrito o pedaço recebido
    Uma forma de realizar isso seria adicionando um preambulo em cada slice, desta forma evitando uma "descincrinização"
    Ex: 000001ASBDHASUFBEUW, 010201ASBDHASUFBEUW,  320401ASBDHASUFBEUW
    Onde os primeiros 5 digitos sempre representa o slice e todos os outros o conteudo de fato

*/