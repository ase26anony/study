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
        /* Complex enough body for GIMPLE sequence */
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
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple operations to create interesting GIMPLE */
        float temp = a[i] + b[i];
        if (temp > 0.0f) {
            c[i] = logf(fabsf(temp) + 1.0f);
        } else {
            c[i] = -logf(fabsf(temp) + 1.0f);
        }
        /* Additional operation to prevent over-simplification */
        c[i] = c[i] * (i % 10);
    }
}

__attribute__((noinline))
void target_simt_multiple_clauses(float *arr, int size, int offset) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:size]) collapse(1) \
        num_teams(16) thread_limit(128) \
        private(offset) firstprivate(size)
    for (int i = 0; i < size; ++i) {
        /* Use offset in computation to ensure it's used */
        arr[i] = arr[i] + (i + offset) * 0.01f;
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
    /* Use command line to select different test cases */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Allocate and initialize test arrays */
    float *arr1 = (float *)malloc(N * sizeof(float));
    float *arr2 = (float *)malloc(N * sizeof(float));
    float *arr3 = (float *)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; ++i) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
        arr3[i] = (float)(i * 3);
    }
    
    float checksum = 0.0f;
    
    /* Execute different target regions based on test case */
    switch (test_case) {
        case 1:
            /* Basic SIMD clause with simple scaling */
            target_simt_vector_scale(arr1, N, 2.5f);
            checksum = compute_checksum(arr1, N);
            printf("Test 1 - Vector scale checksum: %f\n", checksum);
            break;
            
        case 2:
            /* Conditional update with branching */
            target_simt_conditional_update(arr2, N, THRESHOLD);
            checksum = compute_checksum(arr2, N);
            printf("Test 2 - Conditional update checksum: %f\n", checksum);
            break;
            
        case 3:
            /* Multiple arrays with nested control flow */
            target_simt_nested_control(arr1, arr2, arr3, N);
            checksum = compute_checksum(arr3, N);
            printf("Test 3 - Nested control checksum: %f\n", checksum);
            break;
            
        case 4:
            /* Multiple clauses including private/firstprivate */
            target_simt_multiple_clauses(arr1, N, 100);
            checksum = compute_checksum(arr1, N);
            printf("Test 4 - Multiple clauses checksum: %f\n", checksum);
            break;
            
        default:
            /* Run all tests sequentially */
            printf("Running all tests sequentially:\n");
            
            /* Reset arrays */
            for (int i = 0; i < N; ++i) {
                arr1[i] = (float)i;
                arr2[i] = (float)(i * 2);
                arr3[i] = (float)(i * 3);
            }
            
            target_simt_vector_scale(arr1, N, 1.5f);
            printf("  Test 1 checksum: %f\n", compute_checksum(arr1, N));
            
            target_simt_conditional_update(arr2, N, THRESHOLD);
            printf("  Test 2 checksum: %f\n", compute_checksum(arr2, N));
            
            target_simt_nested_control(arr1, arr2, arr3, N);
            printf("  Test 3 checksum: %f\n", compute_checksum(arr3, N));
            
            target_simt_multiple_clauses(arr1, N, 50);
            printf("  Test 4 checksum: %f\n", compute_checksum(arr1, N));
            break;
    }
    
    /* Additional loop to potentially trigger multiple transformations */
    for (int iter = 0; iter < 3; ++iter) {
        #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:N/2]) if(iter > 0)
        for (int i = 0; i < N/2; ++i) {
            arr1[i] = arr1[i] * (iter + 1);
        }
    }
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
