/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int iter) {
    volatile int bound = g_volatile_bound;
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, idx;
    int sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j, idx) shared(a, b, c, bound, n, threshold)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
                private(idx)
        for (i = 0; i < bound; i++) {
            for (j = 0; j < M; j++) {
                idx = i * M + j;
                c[idx] = a[idx] + b[idx] + (iter % 10);
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j > (M/2)) {
                    /* Early exit creates additional control flow */
                    c[idx] = 255; /* Dummy operation */
                    /* Cannot use break in collapsed loop, use conditional */
                    if (i > (bound/2)) {
                        /* Force multiple basic blocks */
                        c[idx] = c[idx] * 2;
                    }
                } else if (c[idx] < 50) {
                    /* Another basic block */
                    c[idx] = c[idx] / 2;
                }
                
                /* Additional control flow complexity */
                switch (iter % 4) {
                    case 0: c[idx] += 1; break;
                    case 1: c[idx] -= 1; break;
                    case 2: c[idx] *= 2; break;
                    case 3: c[idx] /= 2; break;
                }
            }
        }
        
        /* Additional computation outside target region */
        #pragma omp for reduction(+:sum)
        for (idx = 0; idx < TOTAL; idx++) {
            sum += c[idx];
        }
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int n, int threshold, int iter) {
    volatile int bound = g_volatile_bound;
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, idx;
    int sum = 0;
    
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter * 3) % 100;
        b[i] = (i * 3 + iter) % 100;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    collapse(2) if(iter > threshold) \
                    map(to: a[0:TOTAL], b[0:TOTAL]) map(from: c[0:TOTAL]) \
                    num_teams(2) thread_limit(128)
            for (i = 0; i < bound; i++) {
                for (j = 0; j < M; j++) {
                    idx = i * M + j;
                    c[idx] = a[idx] * b[idx] - iter;
                    
                    /* Complex control flow with multiple labels */
                    if (c[idx] % 7 == 0) {
                        c[idx] = c[idx] + idx;
                        if (c[idx] > 1000) {
                            c[idx] = 1000;
                        }
                    } else if (c[idx] % 5 == 0) {
                        c[idx] = c[idx] - idx;
                    } else {
                        c[idx] = c[idx] * 2;
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:sum) nowait
        for (idx = 0; idx < TOTAL; idx++) {
            sum += c[idx];
        }
    }
    
    return sum;
}

int main(void) {
    int results[10];
    int i, total_sum = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying parameters to prevent constant propagation */
    for (i = 0; i < 10; i++) {
        /* Alternate between two different thresholds */
        int threshold = (i % 2 == 0) ? 5 : 8;
        
        /* Call test functions with varying arguments */
        results[i] = simt_test(i + 1, threshold, i);
        results[i] += simt_test2(i + 2, threshold, i + 5);
        
        total_sum += results[i];
        printf("Iteration %d: result = %d\n", i, results[i]);
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Use result to prevent dead code elimination */
    if (total_sum > 0) {
        printf("Test completed successfully.\n");
    } else {
        printf("Warning: Possible dead code elimination.\n");
    }
    
    return 0;
}
