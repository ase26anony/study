/* This file is designed to trigger coverage of the OMP_ARRAY_SECTION case
   in GCC's tree-pretty-print.cc (lines 2736-2748). It uses OpenMP 4.0+ array
   section syntax in various contexts to ensure the pretty-printer handles
   all code paths, including parentheses for complex base expressions.
   Compiles as both C and C++. */

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
#define CHUNK 20

void test_array_sections(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 100.0 + j;
    
    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Buffer for task depend clauses */
    int buffer[200];
    for (int i = 0; i < 200; i++) buffer[i] = 0;
    
    /* 1. OpenMP target data mapping with simple array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++)
            arr1D[i] += 1;
    }
    
    /* 2. Target data with subsection and complex base */
    /* arr1D[10] is a subscript expression -> tests op_prio parentheses */
    #pragma omp target data map(tofrom: arr1D[10:20], arr1D[offset:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++)
            arr1D[i] *= 2;
    }
    
    /* 3. Multi-dimensional array section */
    #pragma omp target data map(to: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] += 1.5;
    }
    
    /* 4. Target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[5:rows-10][0:cols])
    
    /* Use the mapped section */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < ROWS - 5; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] *= 0.5;
    
    #pragma omp target exit data map(from: matrix[5:rows-10][0:cols])
    
    /* 5. Task depend with array sections */
    int start = 50;
    int chunk_size = CHUNK;
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++)
                buffer[i] = i * 2;
        }
        
        /* Second task reads and modifies the same section */
        #pragma omp task depend(inout: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++)
                buffer[i] += 100;
        }
        
        /* Third task with different section */
        #pragma omp task depend(inout: buffer[0:25])
        {
            for (int i = 0; i < 25; i++)
                buffer[i] = -i;
        }
    }
    
    /* 6. Complex base expression: (ptr + offset)[0:size] */
    /* This triggers the op_prio check for parentheses */
    #pragma omp target data map(tofrom: (ptr + offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++)
            ptr[offset + i] = i * 3;
    }
    
    /* 7. Array section with non-zero lower bound and computed length */
    int lower = N/4;
    int length = N/2;
    #pragma omp target data map(tofrom: arr1D[lower:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < length; i++)
            arr1D[lower + i] += 5;
    }
    
    /* Print results to prevent dead code elimination */
    printf("arr1D[0]=%d, arr1D[10]=%d, arr1D[%d]=%d\n", 
           arr1D[0], arr1D[10], N-1, arr1D[N-1]);
    printf("matrix[0][0]=%.1f, matrix[5][10]=%.1f\n", 
           matrix[0][0], matrix[5][10]);
    printf("buffer[%d]=%d, buffer[%d]=%d\n", 
           start, buffer[start], start+chunk_size-1, buffer[start+chunk_size-1]);
    printf("ptr[offset]=%d, ptr[offset+%d-1]=%d\n", 
           ptr[offset], size, ptr[offset+size-1]);
}

int main(void) {
    test_array_sections();
    return 0;
}

#ifdef __cplusplus
}
#endif
