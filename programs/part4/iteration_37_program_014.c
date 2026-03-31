/* This file is designed to trigger the OMP_ARRAY_SECTION pretty-printer
   logic in GCC's tree-pretty-print.cc. It compiles as both C and C++.
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
    /* 1D array for simple array sections */
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
    
    int start = 5;
    int chunk_size = 15;
    
    /* 1. OpenMP Target Data Mapping with Array Sections */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
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
    
    /* 2. OpenMP Target Enter/Exit Data with Array Sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 1.5;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP Task Depend with Array Sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with array section in depend clause */
        #pragma omp task depend(inout: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++) {
                buffer[i] = i * 3;
            }
        }
        
        /* Another task depending on the same section */
        #pragma omp task depend(inout: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++) {
                buffer[i] += 7;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* 4. Multi-dimensional Array Sections */
    /* Full 2D array section */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] -= 0.5;
            }
        }
    }
    
    /* 5. Array Sections with Complex Base Expressions */
    /* Using pointer arithmetic in base expression */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        /* Computation using the pointer section */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] = offset + i;
        }
    }
    
    /* Array section with subscripted base (arr[i][0:N]) */
    int idx = 25;
    #pragma omp target data map(tofrom: matrix[idx][0:30])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 30; j++) {
            matrix[idx][j] = idx * 100.0 + j * 2.0;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("arr1D[0] = %d, arr1D[10] = %d\n", arr1D[0], arr1D[10]);
    printf("matrix[5][10] = %.2f, matrix[25][15] = %.2f\n", 
           matrix[5][10], matrix[25][15]);
    printf("buffer[%d] = %d, buffer[%d] = %d\n", 
           start, buffer[start], start + chunk_size - 1, buffer[start + chunk_size - 1]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
