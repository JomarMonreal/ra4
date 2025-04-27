/**
 * Jomar Monreal
 * 2022-06508
 */

 #include<stdio.h>
 #include<stdlib.h>
 #include<time.h>
 #include<math.h>
 #include<pthread.h>
 #include<sys/types.h>
 #include <unistd.h>
 #include <syscall.h>
 #include <string.h>
 
 typedef struct ARG{
     float * matrix;
     int cols;
     int rows;
     int start_row; 
 }args;
 
 float generateRandomNumber(int min, int max) {
     return min + rand() % (max + 1 - min) ;
 }
 
 void printSquareMatrix(float * matrix, int size) {
     for (int i = 0; i < size; i++)
     {
         for (int j = 0; j < size; j++)
         {
            printf("%f ", matrix[(i * size) + j]);
         }   
         printf("\n");
     }
 }
 
 void printArray(float * arr, int size) {
     for (int j = 0; j < size; j++)
     {
         printf("%f ", arr[j]);
     }   
     printf("\n");
 }
 
 float * generateMatrix(int size) {
     float * matrix = (float *) malloc(sizeof(float) * size * size);
     for (int i = 0; i < size; i++)
     {
         for (int j = 0; j < size; j++)
         {
             matrix[i* size + j] = generateRandomNumber(1,10);
         }   
     }
     return matrix;
 }
 
 void * computeRows(void * arguments) {
     args * temp = (args *) arguments;

     printf("Rows: %d\n", temp->rows);
 
     for (int j = 0; j < temp->rows; j++) {
         float a_j = 0;
         float d_j = 0;
 
         for (int i = 0; i < temp->cols; i++) {
             a_j += temp->matrix[j * temp->cols + i];
         }
         a_j /= temp->cols;
 
         for (int i = 0; i < temp->cols; i++) {
             d_j += pow(temp->matrix[j * temp->cols + i] - a_j, 2);
         }
         d_j = sqrt(d_j / temp->cols);
 
         for (int i = 0; i < temp->cols; i++) {
             temp->matrix[j * temp->cols + i] = (temp->matrix[j * temp->cols + i] - a_j) / d_j;
         }
     }
 
     pthread_exit(NULL);
 }
 
 
 void zsn(float *matrix, int m, int n, int t) {
    pthread_t *tid = malloc(t * sizeof(pthread_t));
    args **thread_args = malloc(t * sizeof(args *));

    int rowsPerThread = m / t;
    int extraRows = m % t;
    int start_row = 0;

    for (int i = 0; i < t; i++) {
        int threadRows = rowsPerThread + (i < extraRows ? 1 : 0);
        int size = threadRows * n;

        // Allocate submatrix and copy data from original
        float *submatrix = malloc(size * sizeof(float));
        memcpy(submatrix, &matrix[start_row * n], size * sizeof(float));

        // Prepare arguments
        thread_args[i] = malloc(sizeof(args));
        thread_args[i]->matrix = submatrix;
        thread_args[i]->rows = threadRows;
        thread_args[i]->cols = n;
        thread_args[i]->start_row = start_row;

        pthread_create(&tid[i], NULL, computeRows, (void *)thread_args[i]);

        start_row += threadRows;
    }

    // Join threads and merge results
    for (int i = 0; i < t; i++) {
        pthread_join(tid[i], NULL);

        // Merge modified submatrix back into the original
        memcpy(&matrix[thread_args[i]->start_row * n],
               thread_args[i]->matrix,
               thread_args[i]->rows * thread_args[i]->cols * sizeof(float));

        free(thread_args[i]->matrix);
        free(thread_args[i]);
    }

    free(thread_args);
    free(tid);
}
 
 int main() {
     srand(time(NULL));
 
     /* use this for testing the example 3x3 matrix with input size 3 */
     float *defaultMatrix = (float *)malloc(3 * 3 * sizeof(float));
     for (int i = 0; i < 3; i++) {
         for (int j = 0; j < 3; j++) {
             defaultMatrix[i * 3 + j] = (11 + i) + (j * 3);
         }
     }
 
     /* main program flow */
     int size;
     printf("Lab 01 < ");
     scanf("%d", &size);
 
     int t;
     printf("Threads < ");
     scanf("%d", &t);
 
     struct timespec start, end;
     clock_gettime(CLOCK_MONOTONIC, &start);
     float * matrix = generateMatrix(size);
     printSquareMatrix(defaultMatrix, 3);
     printf("\n");
 
     zsn(defaultMatrix, 3, 3, t);
    //  zsn(matrix, size, size, t);

    clock_gettime(CLOCK_MONOTONIC, &end);
    float time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("time elapsed: %f seconds\n", time_elapsed);
    printSquareMatrix(defaultMatrix, 3);
 
     free(matrix);
     free(defaultMatrix);
     return 0;
 }