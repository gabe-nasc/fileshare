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

// void reply_client(int option, int socket)
void *reply_client(void *ptr)
{
    printf("HEY\n");
    ReplyClientArguments *arguments = (ReplyClientArguments *)ptr;

    int option = arguments->option;
    int socket = arguments->socket;
    printf("DUDE %d\n", option);
    printf("YEAH %d\n", socket);
    if (option == 1)
    {
        int n = recvInt(socket);
        printf("Received: %d\n", n);
    }
    else if (option == 2)
    {
        char *str = recvString(socket);
        printf("Received: %s\n", str);
    }
    else if (option == 4)
    {
        printf("Listing cloud files...\n");
        send_files_list(".", socket);
    }
    else if (option == 5)
    {
        // printf("INSIDE\n");
        char *filename = recvString(socket);
        printf("Sending %s\n", filename);
        altSendFile(filename, socket);
    }
    else if (option == 6)
    {
        char *path = altRecvFile(socket);
        printf("Received: %s\n", path);
        // printf("Gabriel\n");
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

    memset(&dest, 0, sizeof(dest)); /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_ANY); /* set destination IP number - localhost, */
    dest.sin_port = htons(port);              /* set destination port number */

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

    while (consocket)
    {
        printf("Incoming connection from %s - sending welcome\n", inet_ntoa(dest.sin_addr));

        // reply_client(recvInt(consocket), consocket);
        ReplyClientArguments arguments;
        arguments.socket = consocket;
        arguments.option = recvInt(consocket);

        pthread_create(&thread[thread_counter], NULL, reply_client, (void *)&arguments);
        thread_counter += 1;

        // close(consocket);
        consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
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

    char *msg[5] = {"Msg1\n", "Msg2\n", "Msg3\n", "Msg4\n", "Msg5\n"};

    srand(time(NULL));

    printf("Server is waiting for connections on port: %d\n", port);

    start_listening(port);

    return EXIT_SUCCESS;
}
