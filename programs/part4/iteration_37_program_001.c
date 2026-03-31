/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.c */
/* Or with: g++ -O2 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.cpp */

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
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Buffer for task dependencies */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = 0;
    
    int start = 5;
    
    /* ====== REQUIREMENT 1: OpenMP Target Data Mapping with Array Sections ====== */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    int subsection_start = 10;
    int subsection_len = 20;
    #pragma omp target data map(tofrom: arr1D[subsection_start:subsection_len])
    {
        #pragma omp target teams distribute parallel for
        for (int i = subsection_start; i < subsection_start + subsection_len; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* ====== REQUIREMENT 2: OpenMP Target Enter/Exit Data with Array Sections ====== */
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
    
    /* ====== REQUIREMENT 3: OpenMP Task Depend with Array Sections ====== */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with array section dependency */
        #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
        {
            for (int i = start; i < start + CHUNK_SIZE; i++) {
                buffer[i] = i * 3;
            }
        }
        
        /* Another task depending on different section */
        #pragma omp task depend(inout: buffer[start+CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = start + CHUNK_SIZE; i < start + 2*CHUNK_SIZE; i++) {
                buffer[i] = i * 5;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* ====== REQUIREMENT 4: Multi-dimensional Array Sections ====== */
    /* Full slice of 2D array */
    #pragma omp target data map(tofrom: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] -= 0.5;
            }
        }
    }
    
    /* ====== REQUIREMENT 5: Array Sections with Complex Base Expressions ====== */
    /* Complex base: pointer arithmetic */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 100;
        }
    }
    
    /* Complex base: subscripted access in multi-dim array */
    int idx = 20;
    #pragma omp target data map(tofrom: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] *= 2.0;
        }
    }
    
    /* ====== Final verification output ====== */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < CHUNK_SIZE * 2; i++) sumBuffer += buffer[start + i];
    
    printf("Checksums - 1D array: %d, 2D array: %.2f, Buffer: %d\n", 
           sum1D, sum2D, sumBuffer);
    printf("Sample values - arr1D[0]=%d, matrix[5][5]=%.2f, buffer[%d]=%d\n",
           arr1D[0], matrix[5][5], start, buffer[start]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
