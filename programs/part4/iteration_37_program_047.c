/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.c */
/* Or for C++:   g++ -O2 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.cpp */

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
    
    /* Buffer for task depend clauses */
    int buffer[CHUNK_SIZE * 2];
    for (int i = 0; i < CHUNK_SIZE * 2; i++) buffer[i] = 0;
    
    int start = 5;
    int chunk = CHUNK_SIZE;
    
    /* =========================================== */
    /* 1. OpenMP Target Data Mapping with Array Sections */
    /* =========================================== */
    
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
    
    /* =========================================== */
    /* 2. Complex Base Expression with Pointer Arithmetic */
    /* =========================================== */
    
    /* This triggers op_prio checks for parentheses */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* =========================================== */
    /* 3. Multi-dimensional Array Sections */
    /* =========================================== */
    
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
    
    /* =========================================== */
    /* 4. OpenMP Target Enter/Exit Data with Array Sections */
    /* =========================================== */
    
    #pragma omp target enter data map(to: matrix[20:5][0:COLS])
    
    /* Use the mapped section */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 20; i < 25; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] -= 5.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[20:5][0:COLS])
    
    /* =========================================== */
    /* 5. OpenMP Task Depend with Array Sections */
    /* =========================================== */
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writing to buffer section */
        #pragma omp task depend(out: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] = i * 2;
            }
        }
        
        /* Second task reading and modifying buffer section */
        #pragma omp task depend(inout: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] += 100;
            }
        }
        
        /* Third task with different section */
        #pragma omp task depend(inout: buffer[start+chunk:chunk/2])
        {
            for (int i = start + chunk; i < start + chunk + chunk/2; i++) {
                buffer[i] = buffer[i - chunk] * 3;
            }
        }
    }
    
    /* =========================================== */
    /* 6. Complex Nested Array Section */
    /* =========================================== */
    
    /* Array of pointers for additional complexity */
    int *arr_of_ptrs[10];
    for (int i = 0; i < 10; i++) {
        arr_of_ptrs[i] = &arr1D[i * 10];
    }
    
    /* Complex base: subscripted pointer array */
    #pragma omp target data map(to: arr_of_ptrs[2][0:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < 20; i++) {
            arr_of_ptrs[2][i] += 7;
        }
    }
    
    /* =========================================== */
    /* Final Verification Output */
    /* =========================================== */
    
    /* Print checksums to ensure computations happened */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < CHUNK_SIZE * 2; i++) sumBuffer += buffer[i];
    
    printf("Checksums:\n");
    printf("  arr1D: %d\n", sum1D);
    printf("  matrix: %.2f\n", sum2D);
    printf("  buffer: %d\n", sumBuffer);
    printf("  Sample values - arr1D[0]=%d, matrix[5][5]=%.2f, buffer[5]=%d\n",
           arr1D[0], matrix[5][5], buffer[5]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
