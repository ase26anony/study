/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int n, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: threshold) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        /* Complex enough body to generate interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / threshold;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *input, float *output, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: input[0:n]) map(from: output[0:n]) \
        num_teams(16) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        /* Multiple conditions to create complex control flow */
        float val = input[i];
        if (i % 2 == 0) {
            output[i] = sinf(val) * cosf(val);
        } else if (i % 3 == 0) {
            output[i] = val * val * val;
        } else {
            output[i] = val + 1.0f;
        }
    }
}

__attribute__((noinline))
void target_simt_reduction(float *data, int n, float *sum) {
    float local_sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: data[0:n]) map(tofrom: local_sum) \
        reduction(+:local_sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        local_sum += data[i];
    }
    *sum = local_sum;
}

/* Helper function to verify results */
float verify_vector_add(float *a, float *b, float *c, int n) {
    float error = 0.0f;
    for (int i = 0; i < n; ++i) {
        float expected = a[i] + b[i];
        error += fabsf(c[i] - expected);
    }
    return error;
}

int main(int argc, char *argv[]) {
    /* Use command line to select different test modes */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }
    
    /* Allocate and initialize test data */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *data = (float *)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)i * 0.5f;
        data[i] = (float)i * 100.0f;
    }
    
    /* Execute different target regions based on test mode */
    switch (test_mode) {
        case 0:
            printf("Running all SIMT transformation tests...\n");
            /* Test 1: Basic SIMD vector addition */
            target_simt_vector_add(a, b, c, N);
            printf("Vector add error: %e\n", verify_vector_add(a, b, c, N));
            
            /* Test 2: Conditional update with complex control flow */
            target_simt_conditional_update(data, N, THRESHOLD);
            
            /* Test 3: Nested control flow */
            target_simt_nested_control(a, c, N);
            
            /* Test 4: Reduction with SIMD */
            float sum;
            target_simt_reduction(data, N, &sum);
            printf("Reduction sum: %f\n", sum);
            break;
            
        case 1:
            printf("Running vector addition with varying sizes...\n");
            for (int size = 256; size <= N; size *= 2) {
                target_simt_vector_add(a, b, c, size);
                printf("Size %d error: %e\n", size, 
                       verify_vector_add(a, b, c, size));
            }
            break;
            
        case 2:
            printf("Running conditional update with different thresholds...\n");
            for (float thresh = 100.0f; thresh <= 1000.0f; thresh += 200.0f) {
                memcpy(data, a, N * sizeof(float));
                target_simt_conditional_update(data, N, thresh);
                printf("Threshold %f completed\n", thresh);
            }
            break;
            
        default:
            printf("Running default test...\n");
            target_simt_vector_add(a, b, c, N);
            printf("Default test completed\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(data);
    
    return 0;
}
