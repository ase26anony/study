/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP array section syntax [lower:length] in various contexts. */

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    
    /* Another buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 2;
    
    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* 1. OpenMP Target Data Mapping with Array Sections */
    /* Simple whole array section */
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
            arr1D[i] += 2;
        }
    }
    
    /* 2. OpenMP Target Enter/Exit Data with Array Sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] *= 2.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP Task Depend with Array Sections */
    /* Task with array section in depend clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] *= 3;
                }
            }
            
            #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                    buffer[i] *= 4;
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    /* 4. Multi-dimensional Array Sections in target data */
    /* Full slice of 2D array */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] += 100.0;
            }
        }
    }
    
    /* 5. Array Sections with Complex Base Expressions */
    /* Using pointer arithmetic as base - this triggers op_prio checks */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr+offset)[i] *= 2;
        }
    }
    
    /* Array section with subscripted base */
    int idx = 5;
    #pragma omp target data map(tofrom: matrix[idx][0:10])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 10; j++) {
            matrix[idx][j] -= 50.0;
        }
    }
    
    /* 6. Print results to ensure data is live and computations occurred */
    int checksum1 = 0;
    double checksum2 = 0.0;
    
    for (int i = 0; i < N; i++) checksum1 += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            checksum2 += matrix[i][j];
    
    printf("Array checksum: %d\n", checksum1);
    printf("Matrix checksum: %.2f\n", checksum2);
    printf("Buffer[0] = %d, Buffer[%d] = %d\n", 
           buffer[0], CHUNK_SIZE, buffer[CHUNK_SIZE]);
    printf("arr1D[10] = %d, arr1D[20] = %d\n", arr1D[10], arr1D[20]);
    printf("matrix[5][10] = %.2f, matrix[10][15] = %.2f\n", 
           matrix[5][10], matrix[10][15]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
