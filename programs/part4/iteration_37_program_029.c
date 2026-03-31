/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using array section syntax [lower:length] in various OpenMP constructs. */

#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
extern "C" {
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#define N 100
#define ROWS 50
#define COLS 50
#define CHUNK_SIZE 20

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
    int buffer[CHUNK_SIZE * 3];
    for (int i = 0; i < CHUNK_SIZE * 3; i++) buffer[i] = 0;
    
    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    int checksum = 0;
    
    /* ============================================================
       1. OpenMP Target Data Mapping with Array Sections
       ============================================================ */
    
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
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
    
    /* ============================================================
       2. OpenMP Target Enter/Exit Data with Array Sections
       ============================================================ */
    
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] += 1.0;
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* ============================================================
       3. OpenMP Task Depend with Array Sections
       ============================================================ */
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) buffer[i] = 1;
        }
        
        /* Second task reads/writes overlapping section */
        #pragma omp task depend(inout: buffer[CHUNK_SIZE/2:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE/2; i < CHUNK_SIZE + CHUNK_SIZE/2; i++) 
                buffer[i] += 2;
        }
        
        /* Third task reads from middle section */
        #pragma omp task depend(in: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            int local_sum = 0;
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) 
                local_sum += buffer[i];
            checksum += local_sum;
        }
    }
    
    /* ============================================================
       4. Multi-dimensional Array Sections
       ============================================================ */
    
    /* Full 2D array section */
    #pragma omp target data map(tofrom: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                matrix[i][j] -= 0.5;
    }
    
    /* ============================================================
       5. Array Sections with Complex Base Expressions
       ============================================================ */
    
    /* Using pointer arithmetic in base expression - this may trigger 
       parentheses in the pretty-printer due to op_prio checks */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr+offset)[i] = i * 3;
        }
    }
    
    /* Array section with subscripted base (arr[i]) - using 2D access */
    int idx = 5;
    #pragma omp target data map(tofrom: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] = j * 10.0;
        }
    }
    
    /* ============================================================
       Final verification and output
       ============================================================ */
    
    /* Compute final checksum to ensure all computations happened */
    int final_sum = 0;
    for (int i = 0; i < N; i++) final_sum += arr1D[i];
    
    double matrix_sum = 0.0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix_sum += matrix[i][j];
    
    printf("Array checksum: %d\n", final_sum);
    printf("Matrix sum: %.2f\n", matrix_sum);
    printf("Buffer checksum: %d\n", checksum);
    printf("Sample values - arr1D[0]=%d, matrix[5][5]=%.2f, buffer[10]=%d\n",
           arr1D[0], matrix[5][5], buffer[10]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
