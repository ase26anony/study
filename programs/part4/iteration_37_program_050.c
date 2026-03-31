/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax [lower:length] in various contexts. */

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
    int buffer[2 * CHUNK_SIZE];
    for (int i = 0; i < 2 * CHUNK_SIZE; i++) buffer[i] = 0;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 20;
    
    int checksum = 0;
    
    /* ============================================================
       OpenMP Target Data Mapping with Array Sections
       ============================================================ */
    
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;  /* Trivial computation */
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* ============================================================
       Multi-dimensional Array Sections
       ============================================================ */
    
    /* Map a slice of a 2D array */
    #pragma omp target data map(tofrom: matrix[5:10][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.0;
            }
        }
    }
    
    /* ============================================================
       Array Sections with Complex Base Expressions
       ============================================================ */
    
    /* Using pointer arithmetic in base expression - triggers op_prio checks */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 5;
        }
    }
    
    /* Array section with subscripted base: arr[i][0:N] pattern */
    int (*arr2D)[COLS] = matrix;
    int row_idx = 20;
    #pragma omp target data map(tofrom: arr2D[row_idx][0:30])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 30; j++) {
            arr2D[row_idx][j] += 3.0;
        }
    }
    
    /* ============================================================
       OpenMP Target Enter/Exit Data with Array Sections
       ============================================================ */
    
    /* Enter data with multi-dimensional section */
    #pragma omp target enter data map(to: matrix[30:5][10:20])
    
    /* ... hypothetical computation on device ... */
    
    /* Exit data with same section */
    #pragma omp target exit data map(from: matrix[30:5][10:20])
    
    /* ============================================================
       OpenMP Task Depend with Array Sections
       ============================================================ */
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to first chunk */
        #pragma omp task depend(out: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] = i * 2;
            }
        }
        
        /* Second task reads first chunk, writes second chunk */
        #pragma omp task depend(in: buffer[0:CHUNK_SIZE]) \
                         depend(out: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[CHUNK_SIZE + i] = buffer[i] + 1;
            }
        }
        
        /* Third task reads second chunk */
        #pragma omp task depend(in: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                checksum += buffer[CHUNK_SIZE + i];
            }
        }
    }
    
    /* ============================================================
       Final verification and output
       ============================================================ */
    
    /* Compute simple checksums to ensure data is live */
    int sum1D = 0;
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    
    double sum2D = 0.0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    
    printf("Checksum results:\n");
    printf("  arr1D sum: %d\n", sum1D);
    printf("  matrix sum: %.2f\n", sum2D);
    printf("  task buffer checksum: %d\n", checksum);
    printf("  Sample values - arr1D[0]=%d, matrix[5][5]=%.2f\n", 
           arr1D[0], matrix[5][5]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
