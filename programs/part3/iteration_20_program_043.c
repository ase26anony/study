/* Test program to trigger SIMT transformation in OpenMP target offloading */
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
        /* Complex enough control flow to create interesting GIMPLE */
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
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create complex GIMPLE sequence */
        float val = a[i] + b[i];
        if (val < 0) {
            c[i] = -val;
        } else if (val > 1000.0f) {
            c[i] = val / 2.0f;
        } else {
            c[i] = val * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_multi_clause_simt(float *arr, int size) {
    /* Multiple clauses to test clause processing */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) \
        num_teams(16) num_threads(128) \
        reduction(+:arr[0:size]) /* Note: array reduction */
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] + (float)i;
    }
}

float compute_checksum(float *data, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *array4 = (float *)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)(i % 100);
        array2[i] = (float)(i * 2);
        array3[i] = (float)(i * 3);
        array4[i] = (float)(i * 4);
    }
    
    int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10) iterations = 10;
    }
    
    printf("Running %d iterations of SIMT target tests...\n", iterations);
    
    for (int iter = 0; iter < iterations; ++iter) {
        /* Vary parameters to potentially trigger different code paths */
        float scale = 1.5f + (float)iter * 0.1f;
        float threshold = THRESHOLD + (float)iter * 50.0f;
        int size = N - iter * 10;
        if (size < 100) size = 100;
        
        /* Test 1: Basic SIMD clause */
        target_simt_vector_scale(array1, size, scale);
        float sum1 = compute_checksum(array1, size);
        printf("Iter %d, Test1 checksum: %f\n", iter, sum1);
        
        /* Test 2: Conditional execution within loop */
        target_simt_conditional_update(array2, size, threshold);
        float sum2 = compute_checksum(array2, size);
        printf("Iter %d, Test2 checksum: %f\n", iter, sum2);
        
        /* Test 3: Multiple arrays with nested control flow */
        target_simt_nested_control(array3, array4, array1, size);
        float sum3 = compute_checksum(array1, size);
        printf("Iter %d, Test3 checksum: %f\n", iter, sum3);
        
        /* Test 4: Multiple clauses including reduction */
        target_multi_clause_simt(array4, size);
        float sum4 = compute_checksum(array4, size);
        printf("Iter %d, Test4 checksum: %f\n", iter, sum4);
        
        /* Re-initialize arrays for next iteration */
        for (int i = 0; i < size; ++i) {
            array1[i] = (float)((i + iter) % 100);
            array2[i] = (float)((i + iter) * 2);
            array3[i] = (float)((i + iter) * 3);
            array4[i] = (float)((i + iter) * 4);
        }
    }
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    printf("All SIMT target tests completed.\n");
    return 0;
}
