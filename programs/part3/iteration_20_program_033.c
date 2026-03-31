/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simple_vector_scale(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor + 1.0f;
    }
}

__attribute__((noinline))
void target_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_nested_control_flow(int *a, int *b, int *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        collapse(2) num_teams(16)
    for (int i = 0; i < size/2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            if (idx % 3 == 0) {
                c[idx] = a[idx] + b[idx];
            } else if (idx % 3 == 1) {
                c[idx] = a[idx] - b[idx];
            } else {
                c[idx] = a[idx] * b[idx];
            }
        }
    }
}

__attribute__((noinline))
void target_reduction_like(float *arr, int size, float *result) {
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: arr[0:size]) map(from: sum) reduction(+:sum) \
        num_teams(1)
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    int *data3_a = (int *)malloc(N * sizeof(int));
    int *data3_b = (int *)malloc(N * sizeof(int));
    int *data3_c = (int *)malloc(N * sizeof(int));
    
    if (!data1 || !data2 || !data3_a || !data3_b || !data3_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with test patterns */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i * 1.5f;
        data2[i] = (float)i * 2.0f;
        data3_a[i] = i;
        data3_b[i] = N - i;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 4;
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < 3; ++iter) {
        int size = N / (iter + 1);
        
        switch (test_case) {
            case 0:
                /* Simple vector scaling with SIMD clause - most likely to trigger SIMT */
                target_simple_vector_scale(data1, size, 3.14159f);
                break;
                
            case 1:
                /* Conditional update with branching */
                target_conditional_update(data2, size, THRESHOLD);
                break;
                
            case 2:
                /* Nested loops with collapse clause */
                target_nested_control_flow(data3_a, data3_b, data3_c, size);
                break;
                
            case 3:
                /* Reduction-like pattern */
                float sum_result;
                target_reduction_like(data1, size, &sum_result);
                printf("Iteration %d, sum = %f\n", iter, sum_result);
                break;
        }
        
        /* Verify computation wasn't optimized away */
        float checksum = 0.0f;
        for (int i = 0; i < size; ++i) {
            if (test_case == 0) checksum += data1[i];
            else if (test_case == 1) checksum += data2[i];
            else if (test_case == 2) checksum += (float)data3_c[i];
        }
        
        if (test_case != 3) {
            printf("Test case %d, iteration %d: checksum = %f\n", 
                   test_case, iter, checksum);
        }
    }
    
    /* Additional test with varying parameters */
    if (argc > 2) {
        int dynamic_size = atoi(argv[2]);
        if (dynamic_size > 0 && dynamic_size <= N) {
            #pragma omp target teams distribute parallel for \
                map(tofrom: data1[0:dynamic_size]) simd
            for (int i = 0; i < dynamic_size; ++i) {
                data1[i] = cosf(data1[i]) * sinf(data1[i]);
            }
            
            float final_sum = 0.0f;
            for (int i = 0; i < dynamic_size; ++i) {
                final_sum += data1[i];
            }
            printf("Dynamic size %d: final sum = %f\n", dynamic_size, final_sum);
        }
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3_a);
    free(data3_b);
    free(data3_c);
    
    return 0;
}
