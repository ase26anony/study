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
    
    /* Buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 2;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 20;
    
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
            matrix[i][j] /= 2.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP Task Depend with Array Sections */
    int start = 30;
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with array section in depend clause */
            #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
            {
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    buffer[i] = buffer[i] * 3 + 1;
                }
            }
            
            /* Another task depending on different section */
            #pragma omp task depend(inout: buffer[start+CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = start + CHUNK_SIZE; i < start + 2*CHUNK_SIZE; i++) {
                    buffer[i] = buffer[i] / 2 - 1;
                }
            }
        }
    }
    
    /* 4. Multi-dimensional Array Sections in target data */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] += 1.5;
            }
        }
    }
    
    /* 5. Array Sections with Complex Base Expressions */
    /* Using pointer arithmetic as base expression */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] = ptr[offset + i] * 3;
        }
    }
    
    /* Array section with subscripted base (arr[i]) - requires careful construction */
    int idx = 5;
    #pragma omp target data map(tofrom: arr1D[idx:10])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < 10; i++) {
            arr1D[idx + i] += 100;
        }
    }
    
    /* 6. Print results to prevent dead code elimination and verify execution */
    int checksum = 0;
    checksum += arr1D[0] + arr1D[N-1];
    checksum += (int)matrix[0][0] + (int)matrix[ROWS-1][COLS-1];
    checksum += buffer[start] + buffer[start + CHUNK_SIZE - 1];
    
    printf("Checksum: %d\n", checksum);
    printf("Sample values - arr1D[0]=%d, matrix[5][10]=%.2f, buffer[30]=%d\n",
           arr1D[0], matrix[5][10], buffer[30]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
