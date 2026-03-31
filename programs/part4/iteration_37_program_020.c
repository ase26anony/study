/* This file is designed to trigger the OMP_ARRAY_SECTION pretty-printer
   logic in GCC's tree-pretty-print.cc. It compiles as both valid C and C++.
   Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c [this_file.c] */

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
            matrix[i][j] = i * 100.0 + j;
    
    /* Buffer for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 1.5f;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    int sum = 0;
    
    /* REQUIREMENT 1: OpenMP target data mapping with array sections */
    /* Simple whole array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;  /* Trivial computation to prevent optimization */
        }
    }
    
    /* Subsection with non-zero lower bound */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* REQUIREMENT 5: Complex base expression with pointer arithmetic */
    /* Note: Some GCC versions may require the base expression to be wrapped */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* REQUIREMENT 4: Multi-dimensional array sections */
    #pragma omp target data map(tofrom: matrix[5:10][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.0;
            }
        }
    }
    
    /* REQUIREMENT 2: OpenMP target enter/exit data with array sections */
    int section_rows = 8;
    int section_cols = 12;
    
    #pragma omp target enter data map(to: matrix[20:section_rows][10:section_cols])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 20; i < 20 + section_rows; i++) {
        for (int j = 10; j < 10 + section_cols; j++) {
            matrix[i][j] *= 2.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[20:section_rows][10:section_cols])
    
    /* REQUIREMENT 3: OpenMP task depend with array sections */
    int start = 40;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with array section dependency */
            #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
            {
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    buffer[i] = buffer[i] * 2.0f - 1.0f;
                }
            }
            
            /* Another task depending on the same section */
            #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
            {
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    buffer[i] += 0.5f;
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    /* Additional complex case: array section with subscripted base */
    int idx = 5;
    #pragma omp target data map(tofrom: matrix[idx][0:20])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 20; j++) {
            matrix[idx][j] -= 50.0;
        }
    }
    
    /* Compute checksums to ensure data is live and used */
    for (int i = 0; i < N; i++) sum += arr1D[i];
    
    double matrix_sum = 0.0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix_sum += matrix[i][j];
    
    float buffer_sum = 0.0f;
    for (int i = 0; i < N * 2; i++) buffer_sum += buffer[i];
    
    /* Print results to prevent dead code elimination */
    printf("Checksums - arr1D: %d, matrix: %.2f, buffer: %.2f\n", 
           sum, matrix_sum, buffer_sum);
    printf("Sample values - arr1D[0]=%d, matrix[5][5]=%.2f, buffer[40]=%.2f\n",
           arr1D[0], matrix[5][5], buffer[40]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
