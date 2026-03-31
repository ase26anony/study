/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 *
 * Compile with:
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -o test_simt test_simt_lowering.c
 *
 * Or for broader coverage analysis:
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fprofile-arcs -ftest-coverage -o test_simt test_simt_lowering.c
 *   ./test_simt
 *   gcov omp-low.cc
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
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Create some control flow inside the loop */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_complex_loop(float *a, float *b, float *c, int size, int mode) {
    /* Use mode to create variation in loop structure */
    if (mode == 0) {
        #pragma omp target teams distribute parallel for \
            map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
            num_teams(16) thread_limit(256)
        for (int i = 0; i < size; ++i) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
            num_teams(8)
        for (int i = 0; i < size; ++i) {
            c[i] = sinf(a[i]) + cosf(b[i]);
        }
    }
}

__attribute__((noinline))
void target_nested_simt(float *arr, int size, int iterations) {
    /* Multiple loops in sequence to trigger different lowering contexts */
    for (int iter = 0; iter < iterations; ++iter) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: arr[0:size]) num_teams(4)
        for (int i = 0; i < size; ++i) {
            arr[i] = arr[i] + (float)iter * 0.1f;
        }
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
        array2[i] = (float)(N - i);
        array3[i] = (float)(i % 100);
        result[i] = 0.0f;
    }
    
    /* Use command-line arguments to vary execution paths */
    int mode = 0;
    int iterations = 2;
    
    if (argc > 1) {
        mode = atoi(argv[1]) % 2;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 1;
        if (iterations > 5) iterations = 5;
    }
    
    printf("Running SIMT lowering test with mode=%d, iterations=%d\n", mode, iterations);
    
    /* Call target functions multiple times to increase coverage probability */
    for (int run = 0; run < iterations; ++run) {
        /* Test 1: Basic SIMD clause with vector scaling */
        target_simt_vector_scale(array1, N, 1.5f);
        
        /* Test 2: Conditional execution within SIMT loop */
        target_simt_conditional_update(array2, N, THRESHOLD);
        
        /* Test 3: Complex loop with mode-dependent structure */
        target_simt_complex_loop(array1, array2, result, N, mode);
        
        /* Test 4: Nested loops with SIMT transformation */
        target_nested_simt(array3, N, 2);
        
        /* Alternate mode for next iteration */
        mode = 1 - mode;
    }
    
    /* Verify computation (simple checksum) */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array2[i] + array3[i] + result[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}
