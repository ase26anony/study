/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    volatile int vol_n = n; /* Prevent constant propagation */
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel reduction(+:sum)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(vol_n > threshold) \
                map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
                map(to: vol_n) private(local_sum)
        for (int i = 0; i < g_volatile_bound; i++) {
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + vol_n;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 1000 && i > 10) {
                    /* Early exit creates extra labels/basic blocks */
                    c[idx] = 999;
                    /* Continue anyway to avoid actual break */
                }
                
                /* Another condition to split basic blocks */
                if (j % 8 == 0) {
                    c[idx] += 1;
                }
                
                local_sum += c[idx];
            }
        }
        
        sum += local_sum;
    }
    
    return sum;
}

/* Another test function with different structure */
__attribute__((noinline, cold))
int simt_test2(int n, int *restrict arr) {
    int result = 0;
    volatile int flag = n % 2;
    
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                if(flag) map(tofrom: arr[0:TOTAL]) \
                num_teams(2) thread_limit(64)
        for (int i = 0; i < TOTAL; i++) {
            /* Complex computation with multiple basic blocks */
            int val = arr[i];
            if (val > 50) {
                arr[i] = val * 2 - 1;
                if (arr[i] > 100) {
                    arr[i] = 100;
                }
            } else {
                arr[i] = val + i;
            }
            
            /* Force side effect */
            result += arr[i];
        }
    }
    
    return result;
}

int main(void) {
    int *a = (int*)malloc(TOTAL * sizeof(int));
    int *b = (int*)malloc(TOTAL * sizeof(int));
    int *c = (int*)malloc(TOTAL * sizeof(int));
    int *arr = (int*)malloc(TOTAL * sizeof(int));
    
    /* Initialize with non-uniform values */
    for (int i = 0; i < TOTAL; i++) {
        a[i] = i % 37;
        b[i] = i % 41;
        c[i] = 0;
        arr[i] = i % 53;
    }
    
    int total_sum = 0;
    
    /* Call test function multiple times with varying arguments */
    for (int t = 1; t <= 10; t++) {
        int threshold = t * 3;
        
        /* Mix of different calls to avoid pattern recognition */
        if (t % 2 == 0) {
            total_sum += simt_test(t, threshold, a, b, c);
        } else {
            total_sum += simt_test2(t, arr);
        }
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: partial sum = %d\n", t, total_sum % 1000);
        
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < TOTAL; i += 7) {
            a[i] += t;
            b[i] -= t;
            arr[i] ^= t;
        }
    }
    
    /* Final validation */
    int final_check = 0;
    #pragma omp parallel for simd reduction(+:final_check)
    for (int i = 0; i < TOTAL; i++) {
        final_check += c[i] + arr[i];
    }
    
    printf("Final check: %d\n", final_check);
    printf("Total sum: %d\n", total_sum);
    
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
