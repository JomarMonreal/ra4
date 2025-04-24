#include "headers.h"

#define MAX_FLOATS 30000

int main(void)
{
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;

    float client_floats[MAX_FLOATS]; // buffer to hold received floats
    int num_elements = 0;            // how many floats client will send
    int port = 2000;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        port++;
        server_addr.sin_port = htons(port);
    }
    printf("Done with binding\n");

    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("Listening at %d for incoming connections...\n", port);

    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, (socklen_t*)&client_size);
    if (client_sock < 0){
        printf("Can't accept connection\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Receive number of floats first
    if (recv(client_sock, &num_elements, sizeof(int), 0) <= 0) {
        printf("Failed to receive float count\n");
        return -1;
    }

    if (num_elements > MAX_FLOATS) {
        printf("Client attempted to send too many floats: %d\n", num_elements);
        return -1;
    }

    int bytes_expected = num_elements * sizeof(float);
    int bytes_received = 0;
    char* float_ptr = (char*)client_floats;

    // Loop to ensure all bytes are received (TCP is a stream!)
    while (bytes_received < bytes_expected) {
        int chunk = recv(client_sock, float_ptr + bytes_received, bytes_expected - bytes_received, 0);
        if (chunk <= 0) {
            printf("Connection closed or error during float reception\n");
            return -1;
        }
        bytes_received += chunk;
    }

    printf("Received %d floats from client. Sample:\n", num_elements);
    for(int i = 0; i < (num_elements > 10 ? 10 : num_elements); i++) {
        printf("%.2f ", client_floats[i]);
    }
    printf("\n");

    // Send response
    char server_message[] = "Float array received!";
    send(client_sock, server_message, strlen(server_message), 0);

    close(client_sock);
    close(socket_desc);

    return 0;
}
