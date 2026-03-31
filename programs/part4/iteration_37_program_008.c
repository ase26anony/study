/* This file is designed to trigger coverage of the OMP_ARRAY_SECTION case
   in GCC's tree-pretty-print.cc. It compiles as both valid C and C++.
   Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c [this_file.c]
   or:           g++ -O2 -fopenmp -fdump-tree-omplower -c [this_file.cpp]
*/

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
    for (int i = 0; i < N; ++i) arr1D[i] = i;
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            matrix[i][j] = i * 1.0 + j * 0.01;
    
    /* Buffer for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N*2; ++i) buffer[i] = i * 0.5f;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    int sum = 0;
    
    /* 1. OpenMP target data mapping with array sections */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; ++i) {
            arr1D[i] += 1;  /* Trivial computation */
        }
    }
    
    /* Subsection with non-zero lower bound */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; ++i) {
            arr1D[i] *= 2;
        }
    }
    
    /* 2. OpenMP target enter/exit data with array sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; ++i)
        for (int j = 0; j < COLS; ++j)
            matrix[i][j] += 1.0;
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with depend on array section */
        #pragma omp task depend(inout: buffer[0:CHUNK])
        {
            for (int i = 0; i < CHUNK; ++i)
                buffer[i] += 1.0f;
        }
        
        /* Another task with different section */
        #pragma omp task depend(inout: buffer[CHUNK:CHUNK])
        {
            for (int i = CHUNK; i < 2*CHUNK; ++i)
                buffer[i] *= 2.0f;
        }
        
        #pragma omp taskwait
    }
    
    /* 4. Multi-dimensional array sections in target data */
    int grid[20][30];
    for (int i = 0; i < 20; ++i)
        for (int j = 0; j < 30; ++j)
            grid[i][j] = i + j;
    
    #pragma omp target data map(tofrom: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 20; ++j)
                grid[i][j] += 5;
    }
    
    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; ++i) {
            (ptr+offset)[i] += 3;
        }
    }
    
    /* Using subscripted access as base */
    int arr2D[10][20];
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 20; ++j)
            arr2D[i][j] = i * 20 + j;
    
    int idx = 3;
    #pragma omp target data map(tofrom: arr2D[idx][0:15])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 15; ++j) {
            arr2D[idx][j] -= 2;
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < N; ++i) sum += arr1D[i];
    sum += (int)matrix[5][0];
    sum += (int)buffer[0];
    sum += grid[0][0];
    sum += arr2D[3][0];
    
    printf("Checksum: %d\n", sum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
