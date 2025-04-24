#include "headers.h"
#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(void) {
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    FloatMessage msg;
    int port = 2000;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        port++;
        server_addr.sin_port = htons(port);
    }
    printf("Done with binding\n");

    if(listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        return -1;
    }
    printf("Listening at %d for incoming connections...\n", port);

    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, (socklen_t*)&client_size);
    if (client_sock < 0) {
        printf("Can't accept connection\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Receive the entire FloatMessage struct
    int total_bytes = sizeof(FloatMessage);
    int received_bytes = 0;
    char* buffer = (char*)&msg;

    while (received_bytes < total_bytes) {
        int chunk = recv(client_sock, buffer + received_bytes, total_bytes - received_bytes, 0);
        if (chunk <= 0) {
            printf("Error or disconnection during message receive\n");
            return -1;
        }
        received_bytes += chunk;
    }

    printf("Received message from port %d with %d rows and %d columns. Sample:\n", msg.port_number, msg.num_rows, msg.num_cols);
    for (int i = 0; i < (msg.num_rows > 2 ? 2 : msg.num_rows); i++) {
        for (int j = 0; j < (msg.num_cols > 2 ? 2 : msg.num_cols); j++) {
            printf("%.2f ", msg.floats[i][j]);
        }
        printf("\n");
    }

    // Send the received FloatMessage back to the client
    if (send(client_sock, &msg, sizeof(msg), 0) < 0) {
        printf("Error while sending the message back to the client\n");
        return -1;
    }

    printf("Sent the same message back to the client.\n");

    close(client_sock);
    close(socket_desc);

    return 0;
}
