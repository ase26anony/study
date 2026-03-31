/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -o test_simt test_simt_lowering.c
 * 
 * The program uses multiple OpenMP target regions with teams-distribute-parallel-for
 * constructs that should trigger SIMT transformation when offloading to NVIDIA GPUs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_vector_scale(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor;
    }
}

__attribute__((noinline))
void target_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
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
void target_nested_if(float *a, float *b, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size]) map(from: b[0:size]) \
        num_teams(2) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        /* Nested conditionals to create more complex GIMPLE sequences */
        if (i % 2 == 0) {
            if (a[i] > 0) {
                b[i] = a[i] * 2.0f;
            } else {
                b[i] = -a[i];
            }
        } else {
            b[i] = a[i] + i;
        }
    }
}

__attribute__((noinline))
void target_mixed_types(int *int_arr, float *float_arr, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: int_arr[0:size]) map(tofrom: float_arr[0:size])
    for (int i = 0; i < size; ++i) {
        /* Type conversions and mixed operations */
        float_arr[i] = float_arr[i] + (float)int_arr[i] * 0.1f;
    }
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    int *int_array = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i * 1.5f;
        array2[i] = (float)(N - i) * 0.5f;
        int_array[i] = i % 100;
    }
    
    /* Use command-line arguments to select different execution paths */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < 3; ++iter) {
        int size = N / (iter + 1);
        
        switch (mode) {
            case 0:
                printf("Iteration %d: Running vector scale\n", iter);
                target_vector_scale(array1, size, 2.0f + iter * 0.1f);
                break;
                
            case 1:
                printf("Iteration %d: Running conditional update\n", iter);
                target_conditional_update(array1, size, THRESHOLD + iter * 10.0f);
                break;
                
            case 2:
                printf("Iteration %d: Running nested if\n", iter);
                target_nested_if(array1, array2, size);
                break;
                
            case 3:
                printf("Iteration %d: Running mixed types\n", iter);
                target_mixed_types(int_array, array1, size);
                break;
        }
        
        /* Change mode for next iteration to test different paths */
        mode = (mode + 1) % 4;
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    double checksum = 0.0;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array2[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(int_array);
    
    return 0;
}
