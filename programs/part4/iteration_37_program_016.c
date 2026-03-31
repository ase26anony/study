/* This file is designed to trigger coverage of the OMP_ARRAY_SECTION case
   in tree-pretty-print.cc (lines 2736-2748). It uses OpenMP 4.0+ array
   section syntax in various contexts to ensure the pretty-printer handles
   all code paths, including complex base expressions and multi-dimensional
   sections. Compile with -fopenmp and -fdump-tree-omplower. */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define ROWS 50
#define COLS 50
#define CHUNK 20

int main(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 100.0 + j;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Buffer for task depend clauses */
    int buffer[N];
    for (int i = 0; i < N; i++) buffer[i] = 0;
    
    /* 1. OpenMP target data mapping with simple array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* 2. Target data with subsection and complex base expression */
    /* This triggers op_prio checks for parentheses */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] *= 2;
        }
    }
    
    /* 3. Multi-dimensional array section */
    #pragma omp target data map(to: matrix[5:10][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++)
            for (int j = 0; j < COLS; j++)
                matrix[i][j] += 1.0;
    }
    
    /* 4. Target enter/exit data with array sections */
    int slice[CHUNK];
    for (int i = 0; i < CHUNK; i++) slice[i] = i * 2;
    
    #pragma omp target enter data map(to: slice[0:CHUNK])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < CHUNK; i++) {
        slice[i] += 5;
    }
    
    #pragma omp target exit data map(from: slice[0:CHUNK])
    
    /* 5. Task depend with array sections */
    int start = 25;
    int chunk_size = 25;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(inout: buffer[start:chunk_size])
            {
                for (int i = start; i < start + chunk_size; i++) {
                    buffer[i] = i * 3;
                }
            }
            
            #pragma omp task depend(inout: buffer[start:chunk_size])
            {
                for (int i = start; i < start + chunk_size; i++) {
                    buffer[i] += 7;
                }
            }
        }
    }
    
    /* 6. Additional complex case: array section with subscripted base */
    int arr2D[10][20];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            arr2D[i][j] = i + j;
    
    int idx = 3;
    #pragma omp target data map(tofrom: arr2D[idx][0:10])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 10; j++) {
            arr2D[idx][j] *= 2;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1D: %d\n", arr1D[N/2]);
    printf("Checksum matrix: %.2f\n", matrix[10][10]);
    printf("Checksum buffer: %d\n", buffer[start]);
    printf("Checksum slice: %d\n", slice[CHUNK/2]);
    printf("Checksum arr2D: %d\n", arr2D[idx][5]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
