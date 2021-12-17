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

char *read_file(char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char *string = (char *)calloc(size + 1, sizeof(char));
    fread(string, 1, size, file);
    string[size] = '\0';

    fclose(file);

    return string;
};