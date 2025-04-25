#include "headers.h"
#include "message.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define INITIAL_PORT 2000
#define SERVER_IP "127.0.0.1"
#define CHUNK_SIZE 1000
#define MAX_SERVERS 100

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
void communicate_with_server(const ServerInfo* server, float* data, int num_floats) {
    printf("\n--- Connecting to %s:%d ---\n", server->ip, server->port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    inet_pton(AF_INET, server->ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("Connection failed");
        close(sock);
        return;
    }

    FloatMessageHeader header = {
        .port_number = server->port,
        .num_of_floats = num_floats
    };

    send(sock, &header, sizeof(header), 0);

    int total_chunks = num_floats / CHUNK_SIZE;
    int remainder = num_floats % CHUNK_SIZE;

    for (int i = 0; i < total_chunks; ++i) {
        send(sock, &data[i * CHUNK_SIZE], CHUNK_SIZE * sizeof(float), 0);
    }
    if (remainder > 0) {
        send(sock, &data[total_chunks * CHUNK_SIZE], remainder * sizeof(float), 0);
    }

    FloatMessageHeader echoed_header;
    if (recv(sock, &echoed_header, sizeof(echoed_header), 0) == sizeof(echoed_header)) {
        printf("Received echoed header from %s:%d → port=%d, floats=%d\n",
               server->ip, server->port,
               echoed_header.port_number, echoed_header.num_of_floats);
    }

    float* echoed_data = malloc(num_floats * sizeof(float));
    if (recv(sock, echoed_data, num_floats * sizeof(float), 0) == num_floats * sizeof(float)) {
        printf("Received float array (first 5 values): ");
        for (int i = 0; i < (num_floats > 5 ? 5 : num_floats); ++i)
            printf("%.2f ", echoed_data[i]);
        printf("\n");
    }

    free(echoed_data);
    close(sock);
}

int main() {
    ServerInfo servers[MAX_SERVERS];
    int num_servers = load_servers("servers.txt", servers, MAX_SERVERS);
    if (num_servers <= 0) {
        fprintf(stderr, "No servers loaded\n");
        return 1;
    }

    int num_floats;
    float* data = prepare_data(&num_floats);

    for (int s = 0; s < num_servers; ++s) {
        communicate_with_server(&servers[s], data, num_floats);
    }

    free(data);
    return 0;
}
