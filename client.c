#include "headers.h"
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time() to seed rand()

#define MAX_FLOATS 30000

int main(void) {
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000];

    int n_servers;  // Number of servers to connect to
    int port_base = 2000;  // Starting port for the servers
    int num_floats;  // Number of random floats to send
    float client_floats[MAX_FLOATS];  // Buffer to hold the floats array

    // Ask user for the number of servers and the number of floats to send
    printf("Enter the number of servers to connect to: ");
    scanf("%d", &n_servers);
    
    printf("Enter the number of floats to send to each server: ");
    scanf("%d", &num_floats);
    
    if (num_floats > MAX_FLOATS) {
        printf("The number of floats exceeds the maximum allowed (%d)\n", MAX_FLOATS);
        return -1;
    }

    // Seed the random number generator
    srand(time(NULL));

    // Generate random floats
    for (int i = 0; i < num_floats; i++) {
        client_floats[i] = (float)rand() / RAND_MAX * 100.0;  // Random float between 0 and 100
    }

    // Clean buffer
    memset(server_message, '\0', sizeof(server_message));

    int responses_received = 0;  // Counter for successful responses

    // Create socket and connect to each server
    for (int i = 0; i < n_servers; i++) {
        // Create socket
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_desc < 0) {
            printf("Unable to create socket for server %d\n", i + 1);
            return -1;
        }
        printf("Socket %d created successfully\n", i + 1);

        // Set up server address for the current server
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_base + i);  // Increment port for each server
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Connect to the server
        if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            printf("Unable to connect to server %d\n", i + 1);
            return -1;
        }
        printf("Connected to server %d successfully\n", i + 1);

        // Send number of floats to the server first
        if (send(socket_desc, &num_floats, sizeof(int), 0) < 0) {
            printf("Unable to send number of floats to server %d\n", i + 1);
            return -1;
        }

        // Send the array of floats
        if (send(socket_desc, client_floats, num_floats * sizeof(float), 0) < 0) {
            printf("Unable to send float array to server %d\n", i + 1);
            return -1;
        }

        // Receive response from the server
        if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
            printf("Error while receiving message from server %d\n", i + 1);
            return -1;
        }
        printf("Server %d's response: %s\n", i + 1, server_message);

        // Close the socket after communication with the server
        close(socket_desc);

        // Increment responses received
        responses_received++;
    }

    // Check if all servers responded
    if (responses_received == n_servers) {
        printf("Congrats! All servers have responded successfully.\n");
    } else {
        printf("Some servers did not respond.\n");
    }

    return 0;
}
