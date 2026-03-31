/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = TOTAL;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int iter) {
    /* Use volatile to prevent optimization */
    volatile int v_n = n;
    volatile int v_thresh = threshold;
    
    /* Arrays with mapping to force offloading */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + iter;
        b[i] = i * 2 + iter;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(v_n > v_thresh) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + thread_id;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 1000 && j % 16 == 0) {
                    /* Early exit creates additional labels/blocks */
                    if (i > N/2) {
                        /* This creates more control flow complexity */
                        c[idx] = c[idx] % 100;
                    }
                }
                
                /* Additional computation to prevent loop simplification */
                c[idx] += (idx % 7) * (iter % 5);
            }
        }
        
        /* Additional parallel region inside to increase complexity */
        #pragma omp barrier
        
        #pragma omp for simd nowait
        for (i = 0; i < TOTAL; i++) {
            c[i] = c[i] * 2;
        }
    }
    
    /* Compute reduction result to prevent dead code elimination */
    int sum = 0;
    volatile int* volatile_c = (volatile int*)c;
    for (i = 0; i < TOTAL; i++) {
        sum += volatile_c[i];
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int v_size = size;
    int x[256], y[256], z[256];
    int i, j;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = 256 - i;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel if(flag > 0)
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(v_size > 50) \
                    map(to: x[0:256], y[0:256]) \
                    map(from: z[0:256]) \
                    collapse(2)
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    z[idx] = x[idx] * y[idx];
                    
                    /* Complex control flow */
                    if (z[idx] % 3 == 0) {
                        z[idx] += i;
                        if (z[idx] > 1000) {
                            z[idx] = 999;
                        }
                    } else {
                        z[idx] -= j;
                    }
                }
            }
        }
    }
    
    int result = 0;
    for (i = 0; i < 256; i++) {
        result ^= z[i];
    }
    return result;
}

int main(void) {
    int total_sum = 0;
    int i;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test functions with varying parameters to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold, i);
        int result2 = simt_test2(i * 20, i % 2);
        
        total_sum += result1 + result2;
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total sum: %d\n", total_sum);
    printf("Test completed.\n");
    
    return total_sum != 0 ? 0 : 1;
}
