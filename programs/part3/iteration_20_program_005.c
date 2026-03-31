/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * 
 * For coverage instrumentation: add -fprofile-arcs -ftest-coverage
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
        map(tofrom: arr[0:size]) map(to: scale)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: threshold)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow to create interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_nested_if(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create more complex GIMPLE */
        if (i % 2 == 0) {
            c[i] = a[i] + b[i];
        } else if (i % 3 == 0) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = a[i] * b[i];
        }
    }
}

__attribute__((noinline))
void target_multiple_clauses(float *arr, int size, int *reduction_var) {
    int local_sum = 0;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) reduction(+:local_sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] + (float)i;
        local_sum += (int)arr[i];
    }
    *reduction_var = local_sum;
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *array4 = (float *)malloc(N * sizeof(float));
    int reduction_result = 0;
    
    /* Initialize arrays with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)(i + 1);
        array2[i] = (float)(i * 2);
        array3[i] = (float)(i * 3);
        array4[i] = (float)(i * 4);
    }
    
    /* Use command-line arguments to select different kernels */
    int kernel_selector = 0;
    if (argc > 1) {
        kernel_selector = atoi(argv[1]) % 4;
    }
    
    /* Loop over different configurations to increase coverage */
    for (int config = 0; config < 3; config++) {
        int size = N / (config + 1);
        
        switch (kernel_selector) {
            case 0:
                printf("Running vector scaling kernel\n");
                target_simt_vector_scale(array1, size, 3.14f);
                break;
            case 1:
                printf("Running conditional update kernel\n");
                target_simt_conditional_update(array2, size, THRESHOLD);
                break;
            case 2:
                printf("Running nested conditional kernel\n");
                target_simt_nested_if(array1, array2, array3, size);
                break;
            case 3:
                printf("Running reduction kernel with multiple clauses\n");
                target_multiple_clauses(array4, size, &reduction_result);
                break;
        }
    }
    
    /* Verify computations (prevent dead code elimination) */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Reduction result: %d\n", reduction_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
