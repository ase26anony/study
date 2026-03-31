/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 * 
 * Compilation (for NVIDIA offloading):
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * 
 * For coverage analysis with gcov:
 *   gcc -O0 -fprofile-arcs -ftest-coverage -fopenmp -foffload=nvptx-none -fno-inline -o test_simt_coverage test_simt_lowering.c
 *   ./test_simt_coverage
 *   gcov -b omp-low.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) \
        num_teams(2) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow to create interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) + 1.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) \
        map(from: c[0:size]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create complex GIMPLE sequence */
        if (i % 2 == 0) {
            c[i] = a[i] + b[i];
        } else if (i % 3 == 0) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = a[i] * b[i];
        }
        
        /* Additional operation to increase complexity */
        c[i] = fminf(c[i], 1000.0f);
    }
}

__attribute__((noinline))
void target_multi_clause_simt(float *arr, int size, int offset) {
    /* Multiple clauses to test clause processing during lowering */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) \
        num_teams(4) thread_limit(128) \
        private(offset) \
        firstprivate(size) \
        collapse(1)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] + (i + offset) * 0.1f;
    }
}

float compute_checksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *array4 = (float *)malloc(N * sizeof(float));
    
    /* Initialize arrays with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(i * 2);
        array3[i] = (float)(i * 3);
        array4[i] = (float)(i * 4);
    }
    
    /* Use command-line arguments to select different execution paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < 3; ++iter) {
        printf("Iteration %d:\n", iter + 1);
        
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with vector scaling */
                target_simt_vector_scale(array1, N, 2.5f);
                printf("  Case 1 checksum: %f\n", compute_checksum(array1, N));
                break;
                
            case 2:
                /* Conditional update without explicit SIMD clause */
                target_simt_conditional_update(array2, N, THRESHOLD);
                printf("  Case 2 checksum: %f\n", compute_checksum(array2, N));
                break;
                
            case 3:
                /* Nested control flow with multiple arrays */
                target_simt_nested_control(array1, array2, array3, N);
                printf("  Case 3 checksum: %f\n", compute_checksum(array3, N));
                break;
                
            default:
                /* Multiple clauses including SIMD */
                target_multi_clause_simt(array4, N, iter * 100);
                printf("  Default case checksum: %f\n", compute_checksum(array4, N));
                break;
        }
        
        /* Vary the problem size slightly each iteration */
        int current_size = N - iter * 100;
        if (current_size < 100) current_size = 100;
        
        /* Execute all functions to maximize coverage */
        if (iter == 1) {
            target_simt_vector_scale(array1, current_size, 1.1f);
            target_simt_conditional_update(array2, current_size, 200.0f);
        }
    }
    
    /* Final verification */
    float final_sum = compute_checksum(array1, N) +
                     compute_checksum(array2, N) +
                     compute_checksum(array3, N) +
                     compute_checksum(array4, N);
    printf("Final combined checksum: %f\n", final_sum);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
