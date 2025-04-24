#include "headers.h"
#include "message.h"
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time() to seed rand()

#define MAX_ROWS 1000
#define MAX_COLS 1000

int main(void) {
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000];

    int n_servers;  // Number of servers to connect to
    int port_base = 2000;  // Starting port for the servers
    int num_rows, num_cols;  // Number of rows and columns for the 2D array
    float client_floats[MAX_ROWS][MAX_COLS];  // Buffer to hold the 2D array

    FloatMessage message;

    // Ask user for the number of servers and the dimensions of the 2D array
    printf("Enter the number of servers to connect to: ");
    scanf("%d", &n_servers);
    
    printf("Enter the number of rows in the 2D array: ");
    scanf("%d", &num_rows);
    
    printf("Enter the number of columns in the 2D array: ");
    scanf("%d", &num_cols);
    
    if (num_rows > MAX_ROWS || num_cols > MAX_COLS) {
        printf("The dimensions exceed the maximum allowed (%dx%d)\n", MAX_ROWS, MAX_COLS);
        return -1;
    }

    // Seed the random number generator
    srand(time(NULL));

    // Generate random floats for the 2D array
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            message.floats[i][j] = (float)rand() / RAND_MAX * 100.0;
        }
    }
    message.num_rows = num_rows;
    message.num_cols = num_cols;

    int responses_received = 0;

    // Create socket and connect to each server
    for (int i = 0; i < n_servers; i++) {
        int current_port = port_base + i;
        message.port_number = current_port;  // Set port_number for each server

        // Create socket
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_desc < 0) {
            printf("Unable to create socket for server %d\n", i + 1);
            return -1;
        }
        printf("Socket %d created successfully\n", i + 1);

        // Set up server address for the current server
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(current_port);  // Use current_port
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Connect to the server
        if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            printf("Unable to connect to server %d\n", i + 1);
            return -1;
        }
        printf("Connected to server %d successfully\n", i + 1);

        // Send the entire message struct to the server
        if (send(socket_desc, &message, sizeof(message), 0) < 0) {
            printf("Unable to send message to server %d\n", i + 1);
            return -1;
        }

        // Receive the response from the server (same message struct)
        if (recv(socket_desc, &message, sizeof(message), 0) < 0) {
            printf("Error while receiving the message from server %d\n", i + 1);
            return -1;
        }

        printf("Received the same message back from server %d:\n", i + 1);
        printf("Port: %d, Rows: %d, Cols: %d\n", message.port_number, message.num_rows, message.num_cols);
        for (int r = 0; r < (message.num_rows > 2 ? 2 : message.num_rows); r++) {
            for (int c = 0; c < (message.num_cols > 2 ? 2 : message.num_cols); c++) {
                printf("%.2f ", message.floats[r][c]);
            }
            printf("\n");
        }

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
