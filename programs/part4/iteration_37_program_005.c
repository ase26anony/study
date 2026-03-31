/* This file is designed to trigger the OMP_ARRAY_SECTION pretty-printer
   logic in GCC's tree-pretty-print.cc. It compiles as both valid C and C++.
   Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c [filename] -o [output]
   or:           g++ -O2 -fopenmp -fdump-tree-omplower -c [filename] -o [output]
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
            matrix[i][j] = i * 100.0 + j;
    
    /* Another array for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 1.5f;
    
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
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;  /* Trivial computation */
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
    
    /* 2. OpenMP target enter/exit data with array sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] += 1.0;
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Multiple tasks with array section dependencies */
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++)
                buffer[i] += 10.0f;
        }
        
        #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++)
                buffer[i] -= 5.0f;
        }
        
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE]) \
                         depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            float local_sum = 0.0f;
            for (int i = 0; i < 2*CHUNK_SIZE; i++)
                local_sum += buffer[i];
            /* Use result to prevent optimization */
            sum += (int)local_sum;
        }
    }
    
    /* 4. Multi-dimensional array sections in target data */
    int grid[20][30];
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 30; j++)
            grid[i][j] = i + j;
    
    #pragma omp target data map(tofrom: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                grid[i][j] += 100;
    }
    
    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic as base */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr+offset)[i] += 3;
        }
    }
    
    /* Using subscripted access as base (arr1D[i] is actually an integer,
       but we'll use a different array for this example) */
    int arr2D[10][20];
    int idx = 5;
    #pragma omp target data map(tofrom: arr2D[idx][0:15])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 15; j++) {
            arr2D[idx][j] = j * 2;
        }
    }
    
    /* Print checksums to ensure data is live and computations happen */
    printf("arr1D[0] = %d, arr1D[10] = %d\n", arr1D[0], arr1D[10]);
    printf("matrix[5][0] = %.2f, matrix[14][49] = %.2f\n", 
           matrix[5][0], matrix[14][49]);
    printf("buffer[0] = %.2f, buffer[%d] = %.2f\n", 
           buffer[0], CHUNK_SIZE, buffer[CHUNK_SIZE]);
    printf("grid[0][0] = %d, grid[9][19] = %d\n", grid[0][0], grid[9][19]);
    printf("arr2D[5][0] = %d, arr2D[5][14] = %d\n", arr2D[5][0], arr2D[5][14]);
    printf("sum = %d\n", sum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
