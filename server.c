#define _GNU_SOURCE
#include "headers.h"     // Assume this includes standard system headers
#include "message.h"     // Updated header definitions below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sched.h> 
#include <math.h> 
#include <time.h>
 
 typedef struct ARG{
     float * matrix;
     int cols;
     int rows;
     int start_row; 
 }args;

 void printMatrix(float * matrix, int m, int n) {
     for (int i = 0; i < m; i++)
     {
         for (int j = 0; j < n; j++)
         {
            printf("%.2f ", matrix[(j * m) + i]);
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

 
 void * computeRows(void * arguments) {
     args * temp = (args *) arguments;

     printf("Rows: %d\n", temp->rows);
 
     for (int j = 0; j < temp->rows; j++) {
         float a_j = 0;
         float d_j = 0;
 
         for (int i = 0; i < temp->cols; i++) {
             a_j += temp->matrix[j * temp->cols + i];
         }
         a_j /= temp->cols;
 
         for (int i = 0; i < temp->cols; i++) {
             d_j += pow(temp->matrix[j * temp->cols + i] - a_j, 2);
         }
         d_j = sqrt(d_j / temp->cols);
 
         for (int i = 0; i < temp->cols; i++) {
             temp->matrix[j * temp->cols + i] = (temp->matrix[j * temp->cols + i] - a_j) / d_j;
         }
     }
 }
 
int main(int argc, char *argv[]) {

    // Ask user for a core to bind the server to
    if (argc > 1) {
        int core = atoi(argv[1]);
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(core, &set);

        if (sched_setaffinity(0, sizeof(cpu_set_t), &set) != 0) {
            perror("Failed to set CPU affinity");
        } else {
            printf("Server bound to CPU core %d\n", core);
        }
    } else {
        printf("No core binding applied. Running on default.\n");
    }

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
    
    char ip_str[INET_ADDRSTRLEN];
    printf("Enter IP address to bind the server to: ");
    scanf("%15s", ip_str);

    server_addr.sin_addr.s_addr = inet_addr(ip_str);

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

        printf("Received %d floats from %d x %d matrix at index %d\n",
               header.num_of_floats, header.matrix_size, header.matrix_size, header.start_index);


        printf("\n");

        send(client_sock, &header, sizeof(header), 0);
        
        args * temp = malloc(sizeof(args));
        temp->matrix = floats;
        temp->cols = header.matrix_size;
        temp->rows = header.num_of_floats / header.matrix_size;
        temp->start_row = header.start_index;

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        computeRows(temp);

        clock_gettime(CLOCK_MONOTONIC, &end);
        float time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("\n");
        printf("\ntime elapsed: %f seconds\n", time_elapsed);   
        send(client_sock, floats, float_bytes, 0);

        free(floats);

        end_connection:
            close(client_sock);
            printf("Closed connection with client.\n");
    }

    return 0;
}
