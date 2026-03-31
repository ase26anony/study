/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Create some control flow within the loop body */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        /* More complex control flow to create interesting GIMPLE */
        float val = a[i] + b[i];
        if (val < 0) {
            c[i] = -val;
        } else if (val > 100.0f) {
            c[i] = val / 2.0f;
        } else {
            c[i] = val * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_multi_clause_simt(float *x, float *y, float *z, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: x[0:n], y[0:n]) map(from: z[0:n]) \
        reduction(+:z[0:n]) num_teams(4) collapse(2)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            z[idx] = x[idx] + y[idx] * 3.14f;
        }
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
    int test_size = N;
    int num_iterations = 1;
    
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = N;
    }
    if (argc > 2) {
        num_iterations = atoi(argv[2]);
        if (num_iterations <= 0) num_iterations = 1;
    }
    
    printf("Test size: %d, Iterations: %d\n", test_size, num_iterations);
    
    /* Allocate and initialize test arrays */
    float *arr1 = (float *)malloc(test_size * sizeof(float));
    float *arr2 = (float *)malloc(test_size * sizeof(float));
    float *arr3 = (float *)malloc(test_size * sizeof(float));
    float *arr4 = (float *)malloc(test_size * 2 * sizeof(float));
    float *arr5 = (float *)malloc(test_size * 2 * sizeof(float));
    float *arr6 = (float *)malloc(test_size * 2 * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5 || !arr6) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with test patterns */
    for (int i = 0; i < test_size; ++i) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
        arr3[i] = (float)(i % 100);
    }
    for (int i = 0; i < test_size * 2; ++i) {
        arr4[i] = (float)i;
        arr5[i] = (float)(i * 3);
        arr6[i] = 0.0f;
    }
    
    float total_checksum = 0.0f;
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < num_iterations; ++iter) {
        printf("Iteration %d:\n", iter + 1);
        
        /* Test 1: Simple SIMD scaling */
        target_simt_vector_scale(arr1, test_size, 2.5f);
        float sum1 = compute_checksum(arr1, test_size);
        printf("  Test1 checksum: %f\n", sum1);
        total_checksum += sum1;
        
        /* Test 2: Conditional update with teams distribute */
        target_simt_conditional_update(arr2, test_size, THRESHOLD);
        float sum2 = compute_checksum(arr2, test_size);
        printf("  Test2 checksum: %f\n", sum2);
        total_checksum += sum2;
        
        /* Test 3: Nested control flow */
        target_simt_nested_control(arr1, arr3, arr2, test_size);
        float sum3 = compute_checksum(arr2, test_size);
        printf("  Test3 checksum: %f\n", sum3);
        total_checksum += sum3;
        
        /* Test 4: Multi-clause with collapse and reduction */
        if (iter % 2 == 0) {  /* Alternate to vary execution path */
            target_multi_clause_simt(arr4, arr5, arr6, test_size);
            float sum4 = compute_checksum(arr6, test_size * 2);
            printf("  Test4 checksum: %f\n", sum4);
            total_checksum += sum4;
        }
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < test_size; ++i) {
            arr1[i] += 0.1f;
            arr3[i] = sinf((float)i + iter);
        }
    }
    
    printf("Total checksum: %f\n", total_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    free(arr6);
    
    return 0;
}
