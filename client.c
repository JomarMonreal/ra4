#include "headers.h"
#include "message.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define INITIAL_PORT 2000
#define SERVER_IP "127.0.0.1"
#define CHUNK_SIZE 1000
#define MAX_SERVERS 100

float generateRandomNumber(int min, int max) {
    return min + rand() % (max + 1 - min) ;
}

float * generateMatrix(int size) {
     float * matrix = (float *) malloc(sizeof(float) * size * size);
     for (int i = 0; i < size; i++)
     {
         for (int j = 0; j < size; j++)
         {
             matrix[i* size + j] = generateRandomNumber(1,10);
         }   
     }
     return matrix;
 }

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
         printf("%.2f ", arr[j]);
     }   
     printf("\n");
 }

// Loads IP:port pairs from a text file
int load_servers(const char* filename, ServerInfo* servers, int max_servers) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open server list file");
        return -1;
    }

    int count = 0;
    while (count < max_servers && fscanf(file, "%15[^,],%d\n", servers[count].ip, &servers[count].port) == 2) {
        count++;
    }

    fclose(file);
    return count;
}

// Prompts user to enter number of floats and initializes the float array
float* prepare_data(int* out_count) {
    int count;
    printf("Enter number of data: ");
    scanf("%d", &count);

    float* data = malloc(count * sizeof(float));
    if (!data) {
        perror("Failed to allocate float array");
        exit(1);
    }

    for (int i = 0; i < count; ++i) {
        data[i] = (float)i;
    }

    *out_count = count;
    return data;
}

// Sends the data to the server and receives the processed result
void* communicate_with_server(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    printf("\n--- Connecting to %s:%d ---\n", args->server.ip, args->server.port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        free(args->data);
        free(args);
        return NULL;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(args->server.port);
    inet_pton(AF_INET, args->server.ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("Connection failed");
        close(sock);
        free(args->data);
        free(args);
        return NULL;
    }

    FloatMessageHeader header = {
        .matrix_size = args->matrix_size,
        .num_of_floats = args->num_floats,
        .start_index = args->start_index
    };

    send(sock, &header, sizeof(header), 0);

    int total_chunks = args->num_floats / CHUNK_SIZE;
    int remainder = args->num_floats % CHUNK_SIZE;

    for (int i = 0; i < total_chunks; ++i) {
        send(sock, &args->data[i * CHUNK_SIZE], CHUNK_SIZE * sizeof(float), 0);
    }
    if (remainder > 0) {
        send(sock, &args->data[total_chunks * CHUNK_SIZE], remainder * sizeof(float), 0);
    }

    FloatMessageHeader echoed_header;
    recv(sock, &echoed_header, sizeof(echoed_header), 0);

    float* echoed_data = malloc(args->num_floats * sizeof(float));
    recv(sock, echoed_data, args->num_floats * sizeof(float), 0);

    for (int i = 0; i < args->num_floats; i++) {
        args->matrix[args->start_index + i] = echoed_data[i];
    }

    free(echoed_data);
    free(args->data);
    free(args);
    close(sock);
    return NULL;
}

int main() {
    srand(time(NULL));

    ServerInfo servers[MAX_SERVERS];
    int num_servers = load_servers("servers.txt", servers, MAX_SERVERS);
    if (num_servers <= 0) {
        fprintf(stderr, "No servers loaded\n");
        return 1;
    }

    int n;
    printf("Enter matrix dimensions (n): ");
    scanf("%d", &n);

    float *matrix = generateMatrix(n);
    // float *matrix = (float *)malloc(n * n * sizeof(float));
    //  for (int i = 0; i < n; i++) {
    //      for (int j = 0; j < n; j++) {
    //          matrix[i * n + j] = (11 + i) + (j * n);
    //      }
    //  }
    // printMatrix(matrix, n, n);
    int total_floats = n * n;

    int rows_per_server = n / num_servers;
    int remainder = n % num_servers;

    pthread_t threads[MAX_SERVERS];
    int current_start = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int s = 0; s < num_servers; ++s) {
        int rows_to_send = rows_per_server + (s < remainder ? 1 : 0);
        int num_floats = rows_to_send * n;

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->server = servers[s];
        args->num_floats = num_floats;
        args->matrix_size = n;
        args->start_index = current_start;
        args->matrix = matrix;
        args->data = malloc(num_floats * sizeof(float));

        for (int i = 0; i < num_floats; ++i) {
            args->data[i] = matrix[current_start + i];
        }

        pthread_create(&threads[s], NULL, communicate_with_server, args);
        current_start += num_floats;
    }

    for (int s = 0; s < num_servers; ++s) {
        pthread_join(threads[s], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    float time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n");
    // printMatrix(matrix, n, n);
    printf("\ntime elapsed: %f seconds\n", time_elapsed);

    free(matrix);
    return 0;
}
