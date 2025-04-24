#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_ROWS 1000   // Maximum number of rows in the 2D array
#define MAX_COLS 1000   // Maximum number of columns in the 2D array

typedef struct {
    int port_number;              // Port number as a prefix
    int num_rows;                 // Number of rows in the 2D array
    int num_cols;                 // Number of columns in the 2D array
    float floats[MAX_ROWS][MAX_COLS]; // The actual 2D float data
} FloatMessage;

#endif
