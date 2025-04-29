#include "headers.h"     // Assume this includes standard system headers
#include "message.h"     // Updated header definitions below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

 void printMatrix(float * matrix, int m, int n) {
     for (int i = 0; i < m; i++)
     {
         for (int j = 0; j < n; j++)
         {
            printf("%f ", matrix[(i * n) + j]);
         }   
         printf("\n");
     }
 }
 
 void printArray(float * arr, int size) {
     for (int j = 0; j < size; j++)
     {
         printf("%f ", arr[j]);
     }   
     printf("\n");
 }

int main(void) {
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    int port = 2000;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port %d, trying %d...\n", port, port + 1);
        port++;
        server_addr.sin_port = htons(port);
    }
    printf("Done with binding on port %d\n", port);

    if (listen(socket_desc, 5) < 0) {
        printf("Error while listening\n");
        return -1;
    }
    printf("Listening at %d for incoming connections...\n", port);

    while (1) {
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, (socklen_t*)&client_size);
        if (client_sock < 0) {
            printf("Can't accept connection\n");
            continue;
        }

        printf("Client connected at IP: %s and port: %d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Step 1: Receive header
        FloatMessageHeader header;
        int received = 0;
        char* header_buf = (char*)&header;
        while (received < sizeof(header)) {
            int chunk = recv(client_sock, header_buf + received, sizeof(header) - received, 0);
            if (chunk <= 0) {
                printf("Error receiving header\n");
                close(client_sock);
                goto end_connection;
            }
            received += chunk;
        }

        // Step 2: Receive floats
        int num_floats = header.num_of_floats;
        float* floats = malloc(num_floats * sizeof(float));
        if (!floats) {
            printf("Memory allocation failed\n");
            close(client_sock);
            continue;
        }

        received = 0;
        char* float_buf = (char*)floats;
        int float_bytes = num_floats * sizeof(float);

        while (received < float_bytes) {
            int chunk = recv(client_sock, float_buf + received, float_bytes - received, 0);
            if (chunk <= 0) {
                printf("Error receiving float array\n");
                free(floats);
                close(client_sock);
                goto end_connection;
            }
            received += chunk;
        }

        printf("Received %d floats from %d x %d matrix. Sample:\n",
               header.num_of_floats, header.matrix_size, header.matrix_size);

        printMatrix(floats, header.num_of_floats / header.matrix_size, header.matrix_size);

        printf("\n");

        // (Optional) Echo header + floats back
        send(client_sock, &header, sizeof(header), 0);
        send(client_sock, floats, float_bytes, 0);
        printf("Echoed message back to client.\n");

        free(floats);
    end_connection:
        close(client_sock);
        printf("Closed connection with client.\n");
    }

    // close(socket_desc); // Add this if you implement termination later
    return 0;
}
