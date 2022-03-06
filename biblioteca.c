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
#include <dirent.h>

#include "biblioteca.h"

#define BUFFER_SIZE 500

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
    send(socket, &number, sizeof(int), 0);
};

int recvInt(int socket)
{
    int number;
    recv(socket, &number, sizeof(int), 0);
    return number;
};

void sendDouble(double number, int socket)
{
    send(socket, &number, sizeof(double), 0);
};

double recvDouble(int socket)
{
    double number;
    recv(socket, &number, sizeof(double), 0);

    return number;
};

void sendString(char *string, int socket)
{
    int size = strlen(string);
    sendInt(size, socket);
    send(socket, string, sizeof(char) * size, 0);
};

char *recvString(int socket)
{
    int size = recvInt(socket);
    char *string = (char *)calloc(size, sizeof(char));

    recv(socket, string, size, 0);

    string[size] = '\0';

    return string;
};

void sendVoid(void *voidPointer, int nbytes, int socket)
{
    sendInt(nbytes, socket);
    send(socket, voidPointer, nbytes, 0);
};

void *recvVoid(int socket)
{
    int nbytes = recvInt(socket);
    void *voidPointer = (void *)calloc(nbytes, sizeof(char));
    recv(socket, voidPointer, nbytes, 0);
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

// unsigned char *read_file(char *path)
// {
//     FILE *file = fopen(path, "r");
//     if (file == NULL)
//     {
//         perror("");
//         return 0;
//     }

//     fseek(file, 0, SEEK_END);
//     int size = ftell(file);
//     printf("Size: %d\n", size);
//     rewind(file);

//     unsigned char *content = (unsigned char *)calloc(size + 1, sizeof(unsigned char));

//     fread(content, 1, size, file);

//     fclose(file);

//     return content;
// };

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

void sendFile(char *path, int socket)
{
    printf("Sending file %s\n", path);

    // Open file
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("");
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    unsigned int nbytes = ftell(file);
    printf("Size: %d\n", nbytes);
    rewind(file);

    // Allocate memory for file contents
    void *content = (void *)calloc(nbytes + 1, sizeof(void));

    // Read file contents
    fread(content, 1, nbytes, file);

    // Close file
    fclose(file);

    // Send file path to receiver
    sendString(path, socket);

    // Allocate memory for a slice of the file contents, the size of the slice is determined by BUFFER_SIZE
    void *content_slice = (void *)calloc(BUFFER_SIZE, sizeof(void));

    // Calculate number of slices
    unsigned int nslices = (nbytes / BUFFER_SIZE) + 1;
    if (nslices == 0)
    {
        nslices = 1;
    }

    printf("Slices: %d\n", nslices);

    // Send number of slices to receiver
    sendInt(nslices, socket);

    // For each slice, send slice to receiver
    for (size_t i = 0; i < nslices; i++)
    {
        printf("Sending slice %ld\n", i);
        content_slice = get_array_slice(content, i * BUFFER_SIZE, (i + 1) * BUFFER_SIZE);
        sendVoid(content_slice, BUFFER_SIZE, socket);
    }
};

char *recvFile(int socket)
{
    char *string = recvString(socket);
    printf("Receiving file %s\n", string);

    int nslices = recvInt(socket);

    // TODO: Mudar para void *
    void *content_slice = (void *)calloc(BUFFER_SIZE, sizeof(void));
    void *content = (void *)calloc(BUFFER_SIZE * nslices, sizeof(void));

    for (size_t i = 0; i < nslices; i++)
    {
        printf("Receiving slice %ld\n", i);
        content_slice = recvVoid(socket);
        memcpy(content + i * BUFFER_SIZE, content_slice, BUFFER_SIZE);
    }

    unsigned int content_size = (unsigned int)strlen((char *)content);
    printf("File size: %d\n", content_size);
    write_file(string, content);

    return string;
};

// TODO: Descobrir o filesize
// TODO: Criar arquivo em branco com este tamanho
// TODO: Utilizar o pwrite (Uma thread por parte do arquivo)
