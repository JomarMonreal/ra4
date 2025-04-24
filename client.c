#include "headers.h"

#define MAX_FLOATS 30000

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000];
    
    // Create float array to send
    float client_floats[MAX_FLOATS];
    int num_elements = MAX_FLOATS;

    for (int i = 0; i < num_elements; i++) {
        client_floats[i] = (float)i / 100.0f;  // Example values
    }

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected to server\n");

    // Send number of floats first
    if (send(socket_desc, &num_elements, sizeof(int), 0) < 0) {
        printf("Failed to send float count\n");
        return -1;
    }

    // Send all floats
    int bytes_to_send = num_elements * sizeof(float);
    char* float_ptr = (char*)client_floats;
    int bytes_sent = 0;

    while (bytes_sent < bytes_to_send) {
        int chunk = send(socket_desc, float_ptr + bytes_sent, bytes_to_send - bytes_sent, 0);
        if (chunk <= 0) {
            printf("Error sending float data\n");
            return -1;
        }
        bytes_sent += chunk;
    }

    // Receive response from server
    memset(server_message, 0, sizeof(server_message));
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
        printf("Error receiving server response\n");
        return -1;
    }

    printf("Server response: %s\n", server_message);

    // Close socket
    close(socket_desc);
    return 0;
}
