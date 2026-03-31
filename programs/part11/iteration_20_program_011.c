/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
void simt_test(int n, int threshold, int *result) {
    volatile int i, j;
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int local_sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (int idx = 0; idx < TOTAL; idx++) {
        a[idx] = (idx * 17) % 100;
        b[idx] = (idx * 23) % 100;
        c[idx] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:local_sum)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a, b, c) \
                private(i, j) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + thread_id;
                
                /* Create internal basic block split with conditional */
                if (c[idx] > 150) {
                    /* Force multiple basic blocks - early exit simulation */
                    c[idx] = c[idx] % 100;
                    /* This creates additional control flow */
                } else if (c[idx] < 0) {
                    c[idx] = -c[idx];
                }
                
                /* Additional arithmetic to prevent loop simplification */
                c[idx] += (i ^ j) & 0x1F;
            }
        }
        
        /* Process results after target region */
        #pragma omp for simd reduction(+:local_sum)
        for (int idx = 0; idx < TOTAL; idx++) {
            local_sum += c[idx];
        }
    }
    
    *result = local_sum;
}

/* Another test function with different loop structure */
__attribute__((noinline, cold))
void simt_test_nested(int n, int *arr1, int *arr2, int *out) {
    volatile int bound = n;
    
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(bound > 8) \
                map(tofrom: arr1[0:n*n], arr2[0:n*n], out[0:n*n]) \
                num_teams(2)
        for (volatile int i = 0; i < bound; i++) {
            for (volatile int j = 0; j < bound; j++) {
                int idx = i * n + j;
                out[idx] = arr1[idx] * arr2[idx];
                
                /* Complex conditional to encourage label generation */
                if (out[idx] > 1000) {
                    out[idx] = out[idx] / (i + j + 1);
                    if (out[idx] < 10) {
                        out[idx] = 10;
                    }
                } else {
                    out[idx] = out[idx] + (i << 2) - j;
                }
            }
        }
    }
}

int main(void) {
    int results[10];
    int total_sum = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying parameters to prevent constant propagation */
    for (int iter = 0; iter < 10; iter++) {
        int threshold = iter * 3;
        int n = iter + 5;
        
        simt_test(n, threshold, &results[iter]);
        total_sum += results[iter];
        
        printf("Iteration %d: result = %d\n", iter, results[iter]);
        
        /* Additional test with 2D arrays */
        if (iter % 3 == 0) {
            int size = n + 2;
            int *arr1 = (int*)malloc(size * size * sizeof(int));
            int *arr2 = (int*)malloc(size * size * sizeof(int));
            int *out = (int*)malloc(size * size * sizeof(int));
            
            for (int i = 0; i < size * size; i++) {
                arr1[i] = (i * 7) % 50;
                arr2[i] = (i * 11) % 50;
            }
            
            simt_test_nested(size, arr1, arr2, out);
            
            /* Use results to prevent elimination */
            int check = 0;
            for (int i = 0; i < size * size; i++) {
                check += out[i];
            }
            printf("  Nested test check: %d\n", check);
            
            free(arr1);
            free(arr2);
            free(out);
        }
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Final test with explicit SIMD clause */
    {
        int final_arr[256];
        int final_result = 0;
        
        #pragma omp parallel for simd simdlen(8) if(omp_in_parallel())
        for (int i = 0; i < 256; i++) {
            final_arr[i] = i * i;
        }
        
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: final_arr) if(total_sum > 100)
        for (int i = 0; i < 256; i++) {
            final_arr[i] += omp_get_team_num();
        }
        
        for (int i = 0; i < 256; i++) {
            final_result += final_arr[i];
        }
        
        printf("Final SIMD test result: %d\n", final_result);
    }
    
    return 0;
}
