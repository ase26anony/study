/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-omp.c */
/* Or with: g++ -O2 -fopenmp -fdump-tree-omplower -c tree-pretty-print-omp.cpp */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define ROWS 50
#define COLS 50
#define CHUNK_SIZE 25

void test_omp_array_sections() {
    /* 1D array with simple and complex sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 100.0 + j;
    
    /* Buffer for task depend clauses */
    int buffer[CHUNK_SIZE * 4];
    for (int i = 0; i < CHUNK_SIZE * 4; i++) buffer[i] = 0;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 20;
    
    /* ========== OpenMP Target Data Mapping with Array Sections ========== */
    
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
    
    /* ========== Complex Base Expression with Parentheses ========== */
    /* This triggers op_prio checks in the pretty-printer */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        /* Access through pointer offset */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* ========== Multi-dimensional Array Sections ========== */
    
    /* Full 2D array section */
    #pragma omp target data map(to: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.5;
            }
        }
    }
    
    /* Partial 2D array section */
    #pragma omp target data map(to: matrix[5:10][10:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 10; j < 30; j++) {
                matrix[i][j] *= 2.0;
            }
        }
    }
    
    /* ========== Target Enter/Exit Data with Array Sections ========== */
    
    /* Enter data with array section */
    #pragma omp target enter data map(to: matrix[20:5][0:COLS])
    
    /* Exit data with array section */
    #pragma omp target exit data map(from: matrix[20:5][0:COLS])
    
    /* ========== Task Depend with Array Sections ========== */
    
    int start = CHUNK_SIZE;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with array section in depend clause */
            #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
            {
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    buffer[i] = i * 2;
                }
            }
            
            /* Another task depending on different section */
            #pragma omp task depend(inout: buffer[start+CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = start + CHUNK_SIZE; i < start + 2*CHUNK_SIZE; i++) {
                    buffer[i] = i * 3;
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    /* ========== Complex Nested Array Access ========== */
    /* Array of pointers for additional complexity */
    int* ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &arr1D[i * 10];
    }
    
    /* Array section with subscripted base (arr[i][0:N]) pattern) */
    #pragma omp target data map(to: ptr_array[2][0:15])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < 15; i++) {
            ptr_array[2][i] += 5;
        }
    }
    
    /* ========== Verification Output ========== */
    /* Print checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < CHUNK_SIZE * 4; i++) sumBuffer += buffer[i];
    
    printf("Checksums - 1D: %d, 2D: %.2f, Buffer: %d\n", 
           sum1D, sum2D, sumBuffer);
    printf("Sample values - arr1D[0]=%d, matrix[5][10]=%.2f, buffer[%d]=%d\n",
           arr1D[0], matrix[5][10], start, buffer[start]);
}

int main() {
    test_omp_array_sections();
    return 0;
}

#ifdef __cplusplus
}
#endif
