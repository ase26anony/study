/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f + threshold;
        }
    }
}

__attribute__((noinline))
void target_multi_clause_simt(int *a, int *b, int *c, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        collapse(2) num_teams(16)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            c[idx] = a[idx] + b[idx] * (j + 1);
        }
    }
}

__attribute__((noinline))
void target_nested_if_simt(float *arr, int size, int *mask) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:size]) map(to: mask[0:size])
    for (int i = 0; i < size; ++i) {
        /* Nested conditionals to create complex GIMPLE */
        if (mask[i] > 0) {
            if (arr[i] < 0.0f) {
                arr[i] = -arr[i] * 2.0f;
            } else {
                arr[i] = arr[i] * arr[i];
            }
        } else {
            arr[i] = arr[i] * 0.1f;
        }
    }
}

float verify_sum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    int *idata1 = (int *)malloc(N * 2 * sizeof(int));
    int *idata2 = (int *)malloc(N * 2 * sizeof(int));
    int *idata3 = (int *)malloc(N * 2 * sizeof(int));
    int *mask = (int *)malloc(N * sizeof(int));
    
    /* Initialize data with varying patterns */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i * 1.5f;
        data2[i] = (float)(N - i) * 0.7f;
        mask[i] = (i % 3 == 0) ? 1 : 0;
    }
    
    for (int i = 0; i < N * 2; ++i) {
        idata1[i] = i % 17;
        idata2[i] = i % 23;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < 3; ++iter) {
        printf("Iteration %d, test case %d\n", iter, test_case);
        
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with vector scaling */
                target_simt_vector_scale(data1, N, 2.0f + iter * 0.1f);
                printf("  Vector scale checksum: %.2f\n", verify_sum(data1, N));
                break;
                
            case 2:
                /* Conditional update without explicit simd clause */
                target_simt_conditional_update(data2, N, THRESHOLD + iter * 10.0f);
                printf("  Conditional update checksum: %.2f\n", verify_sum(data2, N));
                break;
                
            case 3:
                /* Multi-clause with collapse and explicit simd */
                target_multi_clause_simt(idata1, idata2, idata3, N);
                {
                    int sum = 0;
                    for (int i = 0; i < N * 2; ++i) sum += idata3[i];
                    printf("  Multi-clause sum: %d\n", sum);
                }
                break;
                
            default:
                /* Nested if statements for complex GIMPLE */
                target_nested_if_simt(data1, N, mask);
                printf("  Nested if checksum: %.2f\n", verify_sum(data1, N));
                
                /* Also run the other cases */
                target_simt_vector_scale(data2, N, 1.5f);
                printf("  Additional scale checksum: %.2f\n", verify_sum(data2, N));
                break;
        }
        
        /* Change test case for next iteration if multiple args provided */
        if (argc > iter + 2) {
            test_case = atoi(argv[iter + 2]);
        }
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(idata1);
    free(idata2);
    free(idata3);
    free(mask);
    
    return 0;
}
