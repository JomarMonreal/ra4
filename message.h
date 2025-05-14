#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_COUNT 100000   // Maximum number of columns in the 2D array

typedef struct {
    int port_number;
    int num_of_floats;               
    float floats[MAX_COUNT]; 
} FloatMessage;

typedef struct {
    int matrix_size;
    int num_of_floats;
    int start_index;
} FloatMessageHeader;

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
} ServerInfo;

typedef struct {
    ServerInfo server;
    float *data;
    int num_floats;
    int matrix_size;
    int start_index;
    float *matrix;
} ThreadArgs;

#endif
