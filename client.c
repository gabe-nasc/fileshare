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

#define MAXRCVLEN 500

int connect_to_server(int port)
{
    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    bzero(buffer, MAXRCVLEN + 1);
    int len, mysocket;
    struct sockaddr_in dest;

    mysocket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&dest, 0, sizeof(dest)); /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* set destination IP number - localhost, 127.0.0.1*/
    dest.sin_port = htons(port);                   /* set destination port number */

    int connectResult = connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

    if (connectResult == -1)
    {

        printf("CLIENT ERROR: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    return mysocket;
}

int user_menu(int port)
{
    printf("Choose an option:\n3 - List Local Files\n4 - List Cloud Files\n5 - Download File\n6 - Upload File\n7 - Exit\n10 - Shutdown Server\n");
    int option;
    scanf("%d", &option);

    pthread_t threads[10];
    int thread_counter = 0;

    // Close client
    if (option == 7)
    {
        printf("Exiting...\n");
    }
    // Send integer to server
    else if (option == 1)
    {
        int n;
        printf("Enter an integer: ");
        scanf("%d", &n);

        int connection = connect_to_server(port);
        sendInt(option, connection);
        sendInt(n, connection);
        close(connection);

        printf("Sent %d\n", n);
    }
    // Send string to server
    else if (option == 2)
    {
        char str[MAXRCVLEN + 1];
        printf("Enter a string: ");
        scanf(" %[^\n]%*c", str);

        int connection = connect_to_server(port);
        sendInt(option, connection);
        sendString(str, connection);
        close(connection);

        printf("Sent %s\n", str);
    }
    // Show local files
    else if (option == 3)
    {
        printf("Listing local files...\n");
        list_directory(".");
    }
    // Receive a list of files available in the server
    else if (option == 4)
    {
        printf("Listing server files...\n");
        int connection = connect_to_server(port);
        list_server_files(connection);
    }
    // Download file from the server
    else if (option == 5)
    {
        char filename[MAXRCVLEN + 1];
        printf("Enter a filename: ");
        scanf("%s", filename);
        int connection = connect_to_server(port);

        DownloadArguments arg;
        arg.socket = connection;
        arg.option = option;
        arg.filename = filename;

        pthread_create(&threads[thread_counter], NULL, download_file, (void *)&arg);
        thread_counter += 1;
    }
    // Upload file from the server
    else if (option == 6)
    {
        char filename[MAXRCVLEN + 1];
        printf("Enter a filename: ");
        scanf(" %[^\n]%*c", filename);

        int connection = connect_to_server(port);
        upload_file(filename, connection, option);

        close(connection);
    }
    // Send Void to server
    else if (option == 8)
    {
        int n;
        printf("Enter an integer: ");
        scanf("%d", &n);

        int connection = connect_to_server(port);
        sendInt(option, connection);
        sendVoid(&n, sizeof(int), connection);
        close(connection);
    }
    // Shutdown server
    else if (option == 10)
    {
        int connection = connect_to_server(port);
        sendInt(option, connection);
        close(connection);
    }
    else
    {
        printf("Invalid option\n");
        return -1;
    }

    return option;
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

    int option = 0;
    while (option != 7)
    {
        option = user_menu(port);
    }

    return EXIT_SUCCESS;
}
