/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 *
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * For coverage: add -fdump-tree-omplower to see the transformation
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
        /* Simple vectorizable operation */
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        /* More complex control flow to create interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_nested_if(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size], b[0:size]) \
        map(from: c[0:size]) \
        num_teams(16) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        /* Nested conditionals for complex GIMPLE sequence */
        if (i % 2 == 0) {
            if (a[i] > b[i]) {
                c[i] = a[i] - b[i];
            } else {
                c[i] = b[i] - a[i];
            }
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

__attribute__((noinline))
void target_simt_multiple_clauses(float *x, float *y, int size, int offset) {
    #pragma omp target teams distribute parallel for \
        map(to: x[0:size]) \
        map(from: y[0:size]) \
        private(offset) \
        num_teams(2) thread_limit(512)
    for (int i = 0; i < size; ++i) {
        /* Operation with private variable */
        y[i] = x[i] + (float)offset;
    }
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(i * 2);
        array3[i] = (float)(i * 3);
        result[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different kernels */
    int kernel_selector = 0;
    if (argc > 1) {
        kernel_selector = atoi(argv[1]) % 4;
    }
    
    /* Execute target regions multiple times with different parameters */
    for (int iter = 0; iter < 3; ++iter) {
        switch ((kernel_selector + iter) % 4) {
            case 0:
                printf("Running vector scale kernel (iter %d)\n", iter);
                target_simt_vector_scale(array1, N, 2.5f);
                break;
                
            case 1:
                printf("Running conditional update kernel (iter %d)\n", iter);
                target_simt_conditional_update(array2, N, THRESHOLD);
                break;
                
            case 2:
                printf("Running nested if kernel (iter %d)\n", iter);
                target_simt_nested_if(array1, array2, result, N);
                break;
                
            case 3:
                printf("Running multiple clauses kernel (iter %d)\n", iter);
                target_simt_multiple_clauses(array3, result, N, iter * 10);
                break;
        }
        
        /* Verify computation by computing checksum (prevents dead code elimination) */
        float checksum = 0.0f;
        for (int i = 0; i < N; ++i) {
            checksum += array1[i] + array2[i] + array3[i] + result[i];
        }
        printf("Iteration %d checksum: %f\n", iter, checksum);
    }
    
    /* Additional test with dynamic loop bounds */
    int dynamic_size = 512;
    if (argc > 2) {
        dynamic_size = atoi(argv[2]);
        if (dynamic_size > N) dynamic_size = N;
    }
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: array1[0:dynamic_size]) \
        num_teams(4)
    for (int i = 0; i < dynamic_size; ++i) {
        array1[i] = sinf(array1[i]) + cosf(array1[i]);
    }
    
    /* Final verification */
    float final_sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        final_sum += array1[i] + array2[i] + array3[i] + result[i];
    }
    printf("Final sum: %f\n", final_sum);
    
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}
