/* test_omp_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_omp_simt test_omp_simt_lowering.c
 * 
 * For coverage analysis, add coverage flags as needed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_region_simple(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) map(to: scale, size)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline))
void target_region_conditional(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: threshold, size)
    for (int i = 0; i < size; ++i) {
        /* Introduce conditional to create more complex GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_region_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* More complex control flow within the loop body */
        float val = a[i] + b[i];
        if (val < 0) {
            c[i] = -val;
        } else if (val > 1000.0f) {
            c[i] = val / 1000.0f;
        } else {
            c[i] = val * val;
        }
    }
}

/* Helper to verify results */
float compute_checksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to vary execution paths */
    int test_case = 1;
    int iterations = 2;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (test_case < 1 || test_case > 3) test_case = 1;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 1;
    }
    
    printf("Running test case %d for %d iteration(s)\n", test_case, iterations);
    
    /* Allocate and initialize test data */
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(N - i);
        array3[i] = 0.0f;
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < iterations; ++iter) {
        float scale = 1.5f + (float)iter * 0.1f;
        float threshold = THRESHOLD + (float)iter * 50.0f;
        
        switch (test_case) {
            case 1:
                printf("Iteration %d: Simple SIMD scaling (scale=%.2f)\n", iter, scale);
                target_region_simple(array1, N, scale);
                printf("  Checksum: %.2f\n", compute_checksum(array1, N));
                break;
                
            case 2:
                printf("Iteration %d: Conditional transformation (threshold=%.2f)\n", iter, threshold);
                target_region_conditional(array2, N, threshold);
                printf("  Checksum: %.2f\n", compute_checksum(array2, N));
                break;
                
            case 3:
                printf("Iteration %d: Nested control flow\n", iter);
                target_region_nested_control(array1, array2, array3, N);
                printf("  Checksum: %.2f\n", compute_checksum(array3, N));
                break;
        }
        
        /* Modify data slightly between iterations to prevent optimization */
        for (int i = 0; i < N; i += 10) {
            array1[i] += 0.01f;
            array2[i] -= 0.01f;
        }
    }
    
    /* Clean up */
    free(array1);
    free(array2);
    free(array3);
    
    printf("Test completed successfully.\n");
    return 0;
}
