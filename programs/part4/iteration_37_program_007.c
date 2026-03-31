/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.c */
/* Also works with: g++ -O2 -fopenmp -fdump-tree-omplower -c tree-pretty-print-test.cpp */

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
    
    int start = 5;
    int rows_slice = 10;
    int cols_slice = 20;
    
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
    
    /* Array section with non-zero lower bound and computed length */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* 2. Multi-dimensional Array Sections */
    /* Full slice of 2D array */
    #pragma omp target data map(tofrom: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.0;
            }
        }
    }
    
    /* Partial slice of 2D array */
    #pragma omp target data map(tofrom: matrix[5:10][10:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 10; j < 30; j++) {
                matrix[i][j] *= 2.0;
            }
        }
    }
    
    /* 3. Array Sections with Complex Base Expressions */
    /* Using pointer arithmetic as base (triggers op_prio checks) */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr+offset)[i] += 5;
        }
    }
    
    /* Subscripted access as base */
    int idx = 2;
    #pragma omp target data map(tofrom: arr1D[idx*10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = idx*10; i < idx*10 + 20; i++) {
            arr1D[i] -= 3;
        }
    }
    
    /* 4. OpenMP Task Depend with Array Sections */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with depend clause using array section */
            #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] = i * 2;
                }
            }
            
            /* Another task depending on different section */
            #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                    buffer[i] = i * 3;
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    /* 5. OpenMP Target Enter/Exit Data with Array Sections */
    /* Enter data with array section */
    #pragma omp target enter data map(to: matrix[start:rows_slice][0:cols_slice])
    
    /* Use the data on target */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = start; i < start + rows_slice; i++) {
        for (int j = 0; j < cols_slice; j++) {
            matrix[i][j] /= 2.0;
        }
    }
    
    /* Exit data with same array section */
    #pragma omp target exit data map(from: matrix[start:rows_slice][0:cols_slice])
    
    /* Print results to prevent dead code elimination */
    printf("arr1D[0] = %d, arr1D[10] = %d\n", arr1D[0], arr1D[10]);
    printf("matrix[5][10] = %.2f\n", matrix[5][10]);
    printf("buffer[0] = %d, buffer[%d] = %d\n", buffer[0], CHUNK_SIZE, buffer[CHUNK_SIZE]);
    
    free(buffer);
}

int main() {
    test_omp_array_sections();
    return 0;
}

#ifdef __cplusplus
}
#endif
