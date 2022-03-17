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
#include <pthread.h>
#include "biblioteca.h"

typedef struct ReplyClientArgument_t
{

    int socket;
    int option;

} ReplyClientArguments;

// Receives a message from the client and replies appropriately
void *reply_client(void *ptr)
{
    ReplyClientArguments *arguments = (ReplyClientArguments *)ptr;

    int option = arguments->option;
    int socket = arguments->socket;

    if (option == 1)
    {
        int n = recvInt(socket);
        printf("Received: %d\n", n);
    }
    else if (option == 2)
    {
        char *str = recvString(socket);
        printf("Received: %s\n", str);
        free(str);
    }
    else if (option == 4)
    {
        printf("Listing server files...\n");
        send_files_list(".", socket);
    }
    else if (option == 5)
    {
        // printf("INSIDE\n");
        char *filename = recvString(socket);
        altSendFile(filename, socket);
        free(filename);
    }
    else if (option == 6)
    {
        char *path = altRecvFile(socket);
        printf("Received: %s\n", path);
        free(path);
    }
    else if (option == 8)
    {
        int n = *(int *)recvVoid(socket);
        printf("Received: %d\n", n);
    }
    else
    {
        printf("Invalid option\n");
    }
}

int start_listening(int port)
{
    int mysocket;
    struct sockaddr_in dest;

    pthread_t thread[50];
    int thread_counter = 0;

    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_ANY);
    dest.sin_port = htons(port);

    mysocket = socket(AF_INET, SOCK_STREAM, 0);

    int bindResult = bind(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

    if (bindResult == -1)
    {

        printf("SERVER ERROR: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    int listenResult = listen(mysocket, 1);

    if (listenResult == -1)
    {

        printf("SERVER ERROR: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);

    // For each client, create a thread to handle the client
    // Keep listening for clients
    while (consocket)
    {
        printf("Incoming connection from %s - sending welcome\n", inet_ntoa(dest.sin_addr));

        ReplyClientArguments arguments;
        arguments.socket = consocket;
        arguments.option = recvInt(consocket);

        if (arguments.option == 10)
        {
            printf("Server Shutdown Requested\n");
            break;
        }

        pthread_create(&thread[thread_counter], NULL, reply_client, (void *)&arguments);
        thread_counter += 1;

        consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
    }

    for (size_t i = 0; i < thread_counter; i++)
    {
        pthread_join(thread[i], NULL);
    }

    close(mysocket);
}

int main(int argc, char *argv[])
{
    int port = 5050;
    if (argc != 2)
    {

        printf("Using default port: 5050\n");
    }
    else
    {
        port = atoi(argv[1]);
    }

    srand(time(NULL));

    printf("Server is waiting for connections on port: %d\n", port);

    start_listening(port);

    return EXIT_SUCCESS;
}
