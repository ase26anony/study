/* This file is designed to trigger coverage of the OMP_ARRAY_SECTION case
   in tree-pretty-print.cc (lines 2736-2748). It uses OpenMP 4.0+ array
   section syntax in various contexts to ensure the pretty-printer handles
   all code paths, including complex base expressions and multi-dimensional
   sections. Compile with -fopenmp and -fdump-tree-omplower to see the
   OMP_ARRAY_SECTION nodes in the dump output. */

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
    /* 1D array for basic array section tests */
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

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 20;

    /* ---------------------------------------------------------------------
       Test 1: OpenMP target data mapping with simple and non-zero bound
               array sections. This creates OMP_ARRAY_SECTION nodes.
    --------------------------------------------------------------------- */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    #pragma omp target data map(tofrom: arr1D[10:20])  /* Subsection */
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }

    /* ---------------------------------------------------------------------
       Test 2: Multi-dimensional array sections.
    --------------------------------------------------------------------- */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] += 1.5;
    }

    /* ---------------------------------------------------------------------
       Test 3: Array sections with complex base expressions.
               The expression (ptr + offset) triggers op_prio checks.
    --------------------------------------------------------------------- */
    #pragma omp target data map(tofrom: (ptr + offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr + offset)[i] = i * 3;
        }
    }

    /* ---------------------------------------------------------------------
       Test 4: OpenMP target enter/exit data with array sections.
    --------------------------------------------------------------------- */
    #pragma omp target enter data map(to: matrix[5:rows-10][0:cols])

    /* ... some computation could happen here ... */

    #pragma omp target exit data map(from: matrix[5:rows-10][0:cols])

    /* ---------------------------------------------------------------------
       Test 5: OpenMP task depend with array sections.
    --------------------------------------------------------------------- */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with depend clause on array section */
            #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] += 100;
                }
            }

            /* Another task depending on a different section */
            #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = CHUNK_SIZE; i < 2 * CHUNK_SIZE; i++) {
                    buffer[i] -= 50;
                }
            }

            #pragma omp taskwait
        }
    }

    /* ---------------------------------------------------------------------
       Final: Print checksums to ensure data is live and computations
              are not optimized away.
    --------------------------------------------------------------------- */
    int sum_arr = 0;
    double sum_mat = 0.0;
    int sum_buf = 0;

    for (int i = 0; i < N; i++) sum_arr += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum_mat += matrix[i][j];
    for (int i = 0; i < N * 2; i++) sum_buf += buffer[i];

    printf("Checksums - arr1D: %d, matrix: %.2f, buffer: %d\n",
           sum_arr, sum_mat, sum_buf);

    /* Print a single element from each array to force live values */
    printf("Sample values: arr1D[0]=%d, matrix[0][0]=%.2f, buffer[0]=%d\n",
           arr1D[0], matrix[0][0], buffer[0]);

    return 0;
}

#ifdef __cplusplus
}
#endif
