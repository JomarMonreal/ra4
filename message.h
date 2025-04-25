#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_COUNT 100000   // Maximum number of columns in the 2D array

typedef struct {
    int port_number;              // Port number as a prefix
    int num_of_floats;                 // Number of rows in the 2D array
    float floats[MAX_COUNT]; // The actual 2D float data
} FloatMessage;

typedef struct {
    int port_number;
    int num_of_floats;
} FloatMessageHeader;

#endif
