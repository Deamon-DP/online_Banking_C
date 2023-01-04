#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include<string.h>
#include "./admin_functions.h"
#include "./customer_functions.h"
int customer_handler(int socket_descriptor)
{

    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    while (1)
    {
        strcpy(write_buffer, "--------------Welcome to DP-Bank----------------\n1.Admin\n2.Customer\n");

        wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        if (wbytes == -1)
        {
            perror("Error while asking login credentials to admin");

            return false;
        }

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0); // read login ID from client

        if (rbytes == -1)
        {
            perror("ERROR while reading login ID of ADMIN");

            return false;
        }
        int choice = atoi(read_buffer);

        // if(choice !=1 && choice !=2)
        // {
        //     strcpy(write_buffer, "ERROR: INVALID OPTION\n");

        //     wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        
        //     return 0;
        // }
        switch (choice)
        {
        case 1:
            admin_handler(socket_descriptor);
            break;
        case 2:
            customer_helper(socket_descriptor);
            break;
        default:
            strcpy(write_buffer, "ERROR: INVALID OPTION\n");

            wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        
            return 0;
            break;
        }
    }
    return 0;
}

int main()
{
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    int ret = setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    if (socket_descriptor == -1)
    {
        perror("Error while creating socket:");
        exit(0);
    }
    else
    {
        printf("Socket created with socket descriptor:%d\n", socket_descriptor);
    }
    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(1811);
    int socket_binder = bind(socket_descriptor, (void *)&server_address, sizeof(server_address));

    if (socket_binder == -1)
    {
        perror("error while binding:");
        exit(0);
    }
    else
    {
        printf("socket is binded to address\n");
    }
    int server_listen = listen(socket_descriptor, 1);
    if (server_listen == -1)
    {
        perror("error while listening\n");
        exit(0);
    }
    else
    {
        printf("server is listening for some client to connect\n");
    }
    while (1)
    {
        int s = sizeof(client_address);
        int server_accept = accept(socket_descriptor, (void *)&client_address, &(s));

        if (server_accept == -1)
        {
            perror("error while accepting a new connection:");
            exit(0);
        }
        else
        {
            printf("The server is connected to client\n");
        }
        // customer_handler(server_accept);

        switch (fork())
        {
        case -1:
            perror("error while creating child prosses for client");
            break;
        case 0:
            printf("child created\n");
            customer_handler(server_accept);
            break;
        }
    }

    close(socket_descriptor);
}