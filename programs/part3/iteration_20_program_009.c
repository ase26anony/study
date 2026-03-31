/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * 
 * For coverage analysis: add -ftest-coverage -fprofile-arcs and run the executable
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_region_simple(float *arr, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * 3.14f + 1.0f;
    }
}

__attribute__((noinline))
void target_region_conditional(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: threshold)
    for (int i = 0; i < size; ++i) {
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
        map(to: a[0:size], b[0:size]) map(tofrom: c[0:size])
    for (int i = 0; i < size; ++i) {
        float sum = a[i] + b[i];
        if (i % 2 == 0) {
            c[i] = sum * 2.0f;
        } else {
            c[i] = sum * 0.5f;
        }
        /* Additional operation to increase GIMPLE complexity */
        c[i] = c[i] + (float)(i % 16);
    }
}

__attribute__((noinline))
void target_region_multiple_clauses(float *x, float *y, float *z, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: x[0:size], y[0:size]) map(from: z[0:size]) \
        num_teams(32) num_threads(64)
    for (int i = 0; i < size; ++i) {
        z[i] = x[i] * y[i] + sinf((float)i * 0.01f);
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
    /* Use command-line arguments to vary execution paths */
    int test_case = 1;
    int iterations = 2;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (test_case < 1 || test_case > 4) test_case = 1;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 2;
    }
    
    printf("Running test case %d with %d iterations\n", test_case, iterations);
    
    /* Allocate and initialize test data */
    float *arr1 = (float *)malloc(N * sizeof(float));
    float *arr2 = (float *)malloc(N * sizeof(float));
    float *arr3 = (float *)malloc(N * sizeof(float));
    float *arr4 = (float *)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; ++i) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
        arr3[i] = (float)(i % 100);
        arr4[i] = 1.0f;
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < iterations; ++iter) {
        printf("Iteration %d:\n", iter + 1);
        
        switch (test_case) {
            case 1:
                target_region_simple(arr1, N);
                printf("  Simple SIMD checksum: %f\n", compute_checksum(arr1, N));
                break;
                
            case 2:
                target_region_conditional(arr2, N, THRESHOLD);
                printf("  Conditional checksum: %f\n", compute_checksum(arr2, N));
                break;
                
            case 3:
                target_region_nested_control(arr1, arr2, arr3, N);
                printf("  Nested control checksum: %f\n", compute_checksum(arr3, N));
                break;
                
            case 4:
                target_region_multiple_clauses(arr1, arr2, arr4, N);
                printf("  Multiple clauses checksum: %f\n", compute_checksum(arr4, N));
                break;
        }
        
        /* Vary loop bounds slightly between iterations */
        int current_size = N - (iter % 10) * 10;
        if (current_size < 100) current_size = N;
        
        /* Mix different target regions in later iterations */
        if (iter > 0 && test_case < 4) {
            target_region_simple(arr3, current_size);
        }
    }
    
    /* Final verification */
    float final_sum = compute_checksum(arr1, N) + 
                     compute_checksum(arr2, N) + 
                     compute_checksum(arr3, N) + 
                     compute_checksum(arr4, N);
    printf("Final combined checksum: %f\n", final_sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
