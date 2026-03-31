/* This file is designed to trigger coverage of OMP_ARRAY_SECTION pretty-printing
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
            matrix[i][j] = i * 1.0 + j * 0.01;
    
    /* Buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 2;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* ================================
       REQUIREMENT 1: OpenMP target data mapping with array sections
       ================================ */
    
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    int start = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* ================================
       REQUIREMENT 4: Multi-dimensional array sections
       ================================ */
    
    /* Map a slice of 2D array */
    #pragma omp target data map(to: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] += 1.5;
    }
    
    /* ================================
       REQUIREMENT 5: Complex base expressions
       ================================ */
    
    /* Array section with pointer arithmetic in base */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* ================================
       REQUIREMENT 2: Target enter/exit data with array sections
       ================================ */
    
    /* Enter data with 2D array section */
    #pragma omp target enter data map(to: matrix[5:rows-10][0:cols])
    
    /* Exit data with different section */
    #pragma omp target exit data map(from: matrix[5:rows-10][0:cols])
    
    /* ================================
       REQUIREMENT 3: Task depend with array sections
       ================================ */
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with inout depend on array section */
            #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] += 100;
                }
            }
            
            /* Another task with out depend on different section */
            #pragma omp task depend(out: buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                    buffer[i] = -buffer[i];
                }
            }
            
            /* Wait for tasks to complete */
            #pragma omp taskwait
        }
    }
    
    /* ================================
       Final verification output
       ================================ */
    
    /* Compute checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < 2*CHUNK_SIZE; i++) sumBuffer += buffer[i];
    
    printf("Checksums - 1D array: %d, 2D array: %.2f, Buffer: %d\n",
           sum1D, sum2D, sumBuffer);
    
    /* Print sample elements to prevent dead code elimination */
    printf("Sample values - arr1D[0]=%d, matrix[0][0]=%.2f, buffer[0]=%d\n",
           arr1D[0], matrix[0][0], buffer[0]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
