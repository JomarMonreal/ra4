#include "headers.h"
#include "message.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define INITIAL_PORT 2000
#define SERVER_IP "127.0.0.1"
#define CHUNK_SIZE 100  // Define the chunk size (100 floats per chunk)

int main() {
    int rows;
    printf("Enter number of data: ");
    scanf("%d", &rows);

    int num_floats = rows;
    float* data = malloc(num_floats * sizeof(float));
    if (!data) {
        perror("Failed to allocate float array");
        return 1;
    }

    // Initialize the float array with some values (for demonstration)
    for (int i = 0; i < num_floats; ++i) {
        data[i] = (float)i;
    }

    // Message header setup
    FloatMessageHeader header;
    header.port_number = 2000;
    header.num_of_floats = rows;

    int sock;
    struct sockaddr_in server_addr;
    int port = INITIAL_PORT;

    // Connect to the server
    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            free(data);
            return 1;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
            printf("Connected to server at %s:%d\n", SERVER_IP, port);
            break;
        }

        close(sock);
        port++;
    }

    // Send header first
    if (send(sock, &header, sizeof(header), 0) != sizeof(header)) {
        perror("Failed to send header");
        close(sock);
        free(data);
        return 1;
    }

    // Calculate the number of chunks and handle remainder
    int total_chunks = num_floats / CHUNK_SIZE;
    int remainder = num_floats % CHUNK_SIZE;

    // Send full chunks
    for (int i = 0; i < total_chunks; ++i) {
        if (send(sock, &data[i * CHUNK_SIZE], CHUNK_SIZE * sizeof(float), 0) != CHUNK_SIZE * sizeof(float)) {
            perror("Failed to send chunk of data");
            close(sock);
            free(data);
            return 1;
        }
        printf("Sent chunk %d of %d\n", i + 1, total_chunks);
    }

    // Send the remainder if it exists
    if (remainder > 0) {
        if (send(sock, &data[total_chunks * CHUNK_SIZE], remainder * sizeof(float), 0) != remainder * sizeof(float)) {
            perror("Failed to send remainder chunk");
            close(sock);
            free(data);
            return 1;
        }
        printf("Sent remainder chunk of size %d\n", remainder);
    }

    // Receive echoed header
    FloatMessageHeader echoed_header;
    if (recv(sock, &echoed_header, sizeof(echoed_header), 0) != sizeof(echoed_header)) {
        perror("Failed to receive echoed header");
    } else {
        printf("Received echoed header: port=%d, floats=%d\n",
               echoed_header.port_number, echoed_header.num_of_floats);
    }

    // Receive echoed float array
    float* echoed_data = malloc(num_floats * sizeof(float));
    if (recv(sock, echoed_data, num_floats * sizeof(float), 0) != num_floats * sizeof(float)) {
        perror("Failed to receive echoed float array");
    } else {
        printf("Received echoed float array (first 5 values):\n");
        for (int i = 0; i < (num_floats > 5 ? 5 : num_floats); ++i) {
            printf("%.2f ", echoed_data[i]);
        }
        printf("\n");
    }

    // Cleanup
    free(data);
    free(echoed_data);
    close(sock);
    return 0;
}
