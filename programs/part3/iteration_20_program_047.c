/* test_omp_simt_lowering.c
 * Test program to cover SIMT lowering in omp-low.cc
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o test_omp_simt test_omp_simt_lowering.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
static void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
static void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough body to generate interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / 3.0f;
        }
    }
}

__attribute__((noinline))
static void target_nested_simt(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        /* Vectorizable operation with function call */
        float temp = sinf(a[i]) + cosf(b[i]);
        c[i] = temp * temp - 1.0f;
    }
}

__attribute__((noinline))
static void target_multi_clause_simt(int *arr, int size, int offset) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) collapse(2) \
        num_teams(4) thread_limit(64) reduction(+:offset)
    for (int i = 0; i < size/2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            arr[idx] = (arr[idx] + offset) * (i + 1);
        }
    }
}

static float verify_sum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

static int verify_sum_int(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command line to vary execution paths */
    int test_case = 1;
    int iterations = 2;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (test_case < 1 || test_case > 4) test_case = 1;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 1;
    }
    
    printf("Running test case %d for %d iterations\n", test_case, iterations);
    
    /* Allocate and initialize test data */
    float *data_f = (float *)malloc(N * sizeof(float));
    float *data_f2 = (float *)malloc(N * sizeof(float));
    float *data_f3 = (float *)malloc(N * sizeof(float));
    int *data_i = (int *)malloc(N * sizeof(int));
    
    if (!data_f || !data_f2 || !data_f3 || !data_i) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int iter = 0; iter < iterations; ++iter) {
        /* Initialize with different patterns each iteration */
        for (int i = 0; i < N; ++i) {
            data_f[i] = (float)(i + iter * 100);
            data_f2[i] = (float)(i * 2 + iter * 50);
            data_f3[i] = (float)(i * 3 + iter * 25);
            data_i[i] = i + iter * 10;
        }
        
        float checksum = 0.0f;
        int int_checksum = 0;
        
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with vector scaling */
                target_simt_vector_scale(data_f, N, 3.14159f);
                checksum = verify_sum(data_f, N);
                printf("Iteration %d, Case 1 checksum: %f\n", iter, checksum);
                break;
                
            case 2:
                /* Conditional update without explicit simd clause */
                target_simt_conditional_update(data_f, N, THRESHOLD);
                checksum = verify_sum(data_f, N);
                printf("Iteration %d, Case 2 checksum: %f\n", iter, checksum);
                break;
                
            case 3:
                /* Multiple arrays with math functions */
                target_nested_simt(data_f, data_f2, data_f3, N);
                checksum = verify_sum(data_f3, N);
                printf("Iteration %d, Case 3 checksum: %f\n", iter, checksum);
                break;
                
            case 4:
                /* Multiple clauses including collapse and reduction */
                target_multi_clause_simt(data_i, N, iter * 5);
                int_checksum = verify_sum_int(data_i, N);
                printf("Iteration %d, Case 4 checksum: %d\n", iter, int_checksum);
                break;
        }
        
        /* Mix test cases across iterations to trigger different paths */
        if (iterations > 1) {
            test_case = (test_case % 4) + 1;
        }
    }
    
    /* Cleanup */
    free(data_f);
    free(data_f2);
    free(data_f3);
    free(data_i);
    
    printf("Test completed successfully\n");
    return 0;
}
