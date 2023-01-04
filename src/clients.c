#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#define max_str 4096

int main(int argc, char const *argv[])
{
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_descriptor == -1)
    {
        perror("Error while creating socket:");

        exit(0);
    }

    else
    {

        printf("Socket created with socket descriptor:%d\n", socket_descriptor);
    }
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;

    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    server_address.sin_port = htons(1811);

    int client_connect = connect(socket_descriptor, (void *)&server_address, sizeof(server_address));

    if (client_connect == -1)
    {

        perror("Error while connecting to server");

        exit(0);
    }

    printf("connected to server\n");

    ssize_t rbytes, wbytes;

    char read_buffer[max_str], write_buffer[max_str];

    do
    {
        rbytes=0;
        bzero(read_buffer, sizeof(read_buffer));

        rbytes = recv(socket_descriptor, read_buffer, sizeof(read_buffer), 0);

        if (rbytes == -1)
        {

            perror("Error while reading from server");

            printf("Closing the connection to the server now!\n");

            break;
        }

        char temp[6];
        char disp[6];

        for (int i = 0; i < 5; i++)
        {

            char e = read_buffer[i];

            temp[i] = e;
            disp[i] = e;
        }

        temp[5] = '\0';
        disp[5] = '\0';
        char transac[6] = "+++++\0";
        char error[6] = "ERROR\0";
        char info[6] = "SHOW+\0";
        char *history = "\n";
        int transaction_size = 0;
        if (strcmp(temp, transac) == 0)
        {

            printf("%s\n", read_buffer + 5);
            getchar();
            strcpy(write_buffer, "dummy_send");

            send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
            continue;
        }

        system("clear");

        if (strcmp(temp, error) == 0)
        {

            printf("%s\n", read_buffer);
           
            perror("Error while reading from server");

            printf("Closing the connection to the server now!\n");

            break;
        }
        else if (strcmp(disp, info) == 0)
        {

            printf("%s\n.....Press any key to get to MENU", read_buffer + 5);

            bzero(write_buffer, sizeof(write_buffer));
            getchar();
            strcpy(write_buffer, "dummy_send");

            send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);
        }
        else
        {
            bzero(write_buffer, sizeof(write_buffer));
            printf("%s\n", read_buffer);

            //scanf("%[^\n]%*c", write_buffer);
            fgets(write_buffer,sizeof(write_buffer),stdin);
            write_buffer[strcspn(write_buffer, "\n")] = '\0';
            if(strcmp(write_buffer,"\n")==0)
            {
                 perror("Error while writing to client socket!");

                printf("Closing the connection to the server now!\n");
                break;
            }

            wbytes = send(socket_descriptor, write_buffer, sizeof(write_buffer), 0);

            if (wbytes == -1 || wbytes==0)
            {

                perror("Error while writing to client socket!");

                printf("Closing the connection to the server now!\n");

                break;
            }
            
        }

    } while (rbytes > 0);

    close(socket_descriptor);
}
