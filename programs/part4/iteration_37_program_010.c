/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.c */
/* Or for C++:   g++ -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.c */

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
    
    /* Dynamic buffer for task dependencies */
    int *buffer = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) buffer[i] = 0;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 20;
    
    /* =========================================== */
    /* 1. OpenMP Target Data Mapping with Array Sections */
    /*    Simple whole-array section */
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
    /* 2. Multi-dimensional Array Sections */
    /*    Full 2D array section */
    #pragma omp target data map(to: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                matrix[i][j] += 1.0;
    }
    
    /* Subsection of 2D array */
    #pragma omp target data map(to: matrix[5:10][10:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++)
            for (int j = 10; j < 30; j++)
                matrix[i][j] *= 2.0;
    }
    
    /* =========================================== */
    /* 3. Array Sections with Complex Base Expressions */
    /*    Using pointer arithmetic as base */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr+offset)[i] += 5;
        }
    }
    
    /* Using subscripted access as base */
    int idx = 5;
    #pragma omp target data map(to: arr1D[idx*2:10])
    {
        #pragma omp target teams distribute parallel for
        for (int i = idx*2; i < idx*2 + 10; i++) {
            arr1D[i] -= 3;
        }
    }
    
    /* =========================================== */
    /* 4. OpenMP Target Enter/Exit Data with Array Sections */
    /*    Enter with 2D array section */
    #pragma omp target enter data map(to: matrix[20:10][0:COLS])
    
    /* Exit with different section */
    #pragma omp target exit data map(from: matrix[25:5][10:20])
    
    /* =========================================== */
    /* 5. OpenMP Task Depend with Array Sections */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with inout dependency on array section */
            #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] = 1;
                }
            }
            
            /* Another task with out dependency on different section */
            #pragma omp task depend(out: buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                    buffer[i] = 2;
                }
            }
            
            /* Task depending on both sections */
            #pragma omp task depend(in: buffer[0:CHUNK_SIZE], \
                                     buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                int sum = 0;
                for (int i = 0; i < 2*CHUNK_SIZE; i++) {
                    sum += buffer[i];
                }
                /* Use sum to prevent dead code elimination */
                buffer[0] = sum;
            }
        }
    }
    
    /* =========================================== */
    /* Print results to ensure code has observable behavior */
    printf("arr1D[0] = %d\n", arr1D[0]);
    printf("arr1D[10] = %d\n", arr1D[10]);
    printf("arr1D[50] = %d\n", arr1D[50]);
    printf("matrix[0][0] = %.2f\n", matrix[0][0]);
    printf("matrix[5][10] = %.2f\n", matrix[5][10]);
    printf("matrix[25][15] = %.2f\n", matrix[25][15]);
    printf("buffer[0] = %d\n", buffer[0]);
    printf("buffer[CHUNK_SIZE] = %d\n", buffer[CHUNK_SIZE]);
    
    free(buffer);
    return 0;
}

#ifdef __cplusplus
}
#endif
