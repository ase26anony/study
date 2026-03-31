/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
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
        /* Create some control flow inside the loop body to make
         * the GIMPLE sequence more complex for copy_gimple_seq_and_replace_locals */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_nested_teams(float *a, float *b, float *c, int size) {
    /* Multiple map clauses create a richer data environment */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] + b[i] * 2.0f;
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
    
    /* Initialize with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(i * 2);
        array3[i] = (float)(i % 100);
    }
    
    /* Use command-line arguments to select different kernels
     * This may cause the compiler to generate multiple lowering paths */
    int kernel_mode = 0;
    if (argc > 1) {
        kernel_mode = atoi(argv[1]) % 3;
    }
    
    /* Loop over different sizes to increase coverage chances */
    for (int iter = 0; iter < 3; ++iter) {
        int current_size = N / (1 << iter);
        if (current_size < 32) current_size = 32;
        
        switch (kernel_mode) {
            case 0:
                /* Explicit SIMD clause - most likely to trigger SIMT transformation */
                target_simt_vector_scale(array1, current_size, 3.14159f);
                break;
                
            case 1:
                /* Conditional update with control flow */
                target_simt_conditional_update(array2, current_size, THRESHOLD);
                break;
                
            case 2:
                /* Multiple arrays with different mappings */
                target_simt_nested_teams(array1, array3, result, current_size);
                break;
        }
        
        /* Change kernel mode for next iteration */
        kernel_mode = (kernel_mode + 1) % 3;
    }
    
    /* Verify computation by computing a checksum (prevents dead code elimination) */
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
