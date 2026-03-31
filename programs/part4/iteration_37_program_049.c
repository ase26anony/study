/* This file is designed to trigger coverage of the OMP_ARRAY_SECTION case
   in tree-pretty-print.cc (lines 2736-2748). It uses OpenMP 4.0+ array
   section syntax in various contexts to ensure the pretty-printer handles
   all forms of OMP_ARRAY_SECTION nodes. Compile with -fopenmp and
   -fdump-tree-omplower to see the generated tree dumps. */

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
            matrix[i][j] = i * 1.0 + j * 0.01;
    
    /* Pointer for complex base expressions */
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
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 5.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 4. OpenMP task depend with array section */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[0:CHUNK])
        {
            for (int i = 0; i < CHUNK; i++) {
                buffer[i] = 100 + i;
            }
        }
        
        #pragma omp task depend(inout: buffer[CHUNK:CHUNK])
        {
            for (int i = CHUNK; i < 2*CHUNK; i++) {
                buffer[i] = 200 + i;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* 5. Array section with complex base expression (pointer arithmetic) */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] = -ptr[offset + i];
        }
    }
    
    /* 6. Multi-dimensional array section in target data */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] *= 0.5;
            }
        }
    }
    
    /* Print checksums to prevent dead code elimination */
    int sum1 = 0, sum2 = 0;
    double sum3 = 0.0;
    
    for (int i = 0; i < N; i++) sum1 += arr1D[i];
    for (int i = 0; i < 200; i++) sum2 += buffer[i];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            sum3 += matrix[i][j];
    
    printf("Checksums: arr1D=%d, buffer=%d, matrix[0:10][0:20]=%.2f\n",
           sum1, sum2, sum3);
    printf("Sample values: arr1D[0]=%d, matrix[5][10]=%.2f, buffer[50]=%d\n",
           arr1D[0], matrix[5][10], buffer[50]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
