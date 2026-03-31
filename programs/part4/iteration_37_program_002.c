/* This file is designed to trigger the OMP_ARRAY_SECTION pretty-printer
   logic in GCC's tree-pretty-print.cc. It compiles as both valid C and C++.
   Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-array-section.c
   or:           g++ -O2 -fopenmp -fdump-tree-omplower -c tree-pretty-print-array-section.cpp
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define ROWS 50
#define COLS 50
#define CHUNK_SIZE 25

int main(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 1.0 + j * 0.01;
    
    /* Buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N*2; i++) buffer[i] = i % 10;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* 1. OpenMP target data mapping with simple array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* 2. OpenMP target data mapping with subsection */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* 3. OpenMP target enter/exit data with multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Computation on the device using the mapped section */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 100.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 4. OpenMP task depend with array sections */
    int start = 40;
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* First task writing to buffer section */
            #pragma omp task depend(out: buffer[start:CHUNK_SIZE])
            {
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    buffer[i] = 999;
                }
            }
            
            /* Second task reading the same buffer section */
            #pragma omp task depend(in: buffer[start:CHUNK_SIZE])
            {
                int sum = 0;
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    sum += buffer[i];
                }
                /* Use sum to prevent dead code elimination */
                buffer[0] = sum % 100;
            }
        }
    }
    
    /* 5. Array section with complex base expression (pointer arithmetic) */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 5;
        }
    }
    
    /* 6. Multi-dimensional array section with non-zero lower bounds */
    #pragma omp target data map(tofrom: matrix[2:8][3:10])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 2; i < 10; i++) {
            for (int j = 3; j < 13; j++) {
                matrix[i][j] -= 50.0;
            }
        }
    }
    
    /* Print checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sum2D = 0.0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    
    printf("Checksum 1D: %d\n", sum1D);
    printf("Checksum 2D: %f\n", sum2D);
    printf("Buffer[%d] = %d\n", start, buffer[start]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
