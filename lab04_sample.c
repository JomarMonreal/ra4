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
 
 typedef struct ARG{
     float ** matrix;
     int cols;
     int rows;
 }args;
 
 float generateRandomNumber(int min, int max) {
     return min + rand() % (max + 1 - min) ;
 }
 
 void printSquareMatrix(float ** matrix, int size) {
     for (int i = 0; i < size; i++)
     {
         for (int j = 0; j < size; j++)
         {
             printf("%f ", matrix[j][i]);
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
 
 float ** generateMatrix(int size) {
     float ** matrix = (float **) malloc(sizeof(float*) * size);
     for (int i = 0; i < size; i++)
     {
         matrix[i] = (float *) malloc(sizeof(float) * size);
         for (int j = 0; j < size; j++)
         {
             matrix[i][j] = generateRandomNumber(1,10);
         }   
     }
     return matrix;
 }
 
 void * computeColumns(void * arguments) {
     args * temp = (args *) arguments;
 
     for (int j = 0; j < temp->cols; j++) {
         float a_j = 0;
         float d_j = 0;
 
         for (int i = 0; i < temp->rows; i++) {
             a_j += temp->matrix[j][i];
         }
         a_j /= temp->rows;
 
         for (int i = 0; i < temp->rows; i++) {
             d_j += pow(temp->matrix[j][i] - a_j, 2);
         }
         d_j = sqrt(d_j / temp->rows);
 
         for (int i = 0; i < temp->rows; i++) {
             temp->matrix[j][i] = (temp->matrix[j][i] - a_j) / d_j;
         }
     }
 
     pthread_exit(NULL);
 }
 
 
 void zsn(float **matrix, int m, int n, int t) {
     pthread_t *tid = (pthread_t *)malloc(t * sizeof(pthread_t));
     
     int columnsPerThread = m / t;
     int extraColumns = m % t;
     int start = 0;
 
     for (int i = 0; i < t; i++) {
         int threadCols = columnsPerThread + (i < extraColumns ? 1 : 0);
 
         // Allocate submatrix with threadCols columns
         float **submatrix = (float **)malloc(threadCols * sizeof(float *));
         for (int j = 0; j < threadCols; j++) {
             submatrix[j] = matrix[start + j]; // point to original column
         }
 
         args *arguments = (args *)malloc(sizeof(args));
         arguments->matrix = submatrix;
         arguments->cols = threadCols;
         arguments->rows = n;  // number of rows stays the same
 
         pthread_create(&tid[i], NULL, computeColumns, (void *)arguments);
         start += threadCols;
     }
 
     for (int i = 0; i < t; i++) {
         pthread_join(tid[i], NULL);
     }
 
     free(tid);
 }
 
 int main() {
     srand(time(NULL));
 
     /* use this for testing the example 3x3 matrix with input size 3 */
     float **defaultMatrix = (float **)malloc(3 * sizeof(float *));
     for (int i = 0; i < 3; i++) {
         defaultMatrix[i] = (float *)malloc(3 * sizeof(float));
         for (int j = 0; j < 3; j++) {
             defaultMatrix[i][j] = (11 + i) + (j * 3);
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
     float ** matrix = generateMatrix(size);
 
     zsn(matrix, size, size, t);
 
     clock_gettime(CLOCK_MONOTONIC, &end);
     float time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
 
     printf("time elapsed: %f seconds\n", time_elapsed);
 
     for (int i = 0; i < size; i++) {
         free(matrix[i]);
     }
     free(matrix);
     return 0;
 }