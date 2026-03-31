/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int iter) {
    volatile int use_gpu = (n > threshold);
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
    #pragma omp parallel private(i, j, idx) firstprivate(use_gpu)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                idx = i * M + j;
                c[idx] = a[idx] + b[idx] + iter;
                
                /* Create internal basic block split */
                if (c[idx] > 150 && use_gpu) {
                    /* Early exit-like construct to force label generation */
                    c[idx] = c[idx] % 100;
                    /* This creates additional control flow */
                }
                
                /* Additional computation to prevent loop simplification */
                c[idx] += (idx % 7);
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < TOTAL; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int dyn_size = size;
    int x[TOTAL], y[TOTAL];
    int result = 0;
    
    for (int i = 0; i < TOTAL; i++) {
        x[i] = i % 50;
        y[i] = 0;
    }
    
    /* Complex nesting: parallel -> single -> target */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(flag) map(to: x) map(from: y) \
                    num_teams(2) num_threads(64)
            for (int i = 0; i < dyn_size; i++) {
                y[i] = x[i] * 2;
                
                /* Multiple basic blocks within SIMD loop */
                if (y[i] > 40) {
                    y[i] = y[i] - 40;
                    if (y[i] < 10) {
                        y[i] = y[i] + 30;
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:result)
        for (int i = 0; i < dyn_size; i++) {
            result += y[i];
        }
    }
    
    return result;
}

int main() {
    int total_sum = 0;
    int threshold = 5;
    
    /* Varying arguments to prevent constant propagation */
    for (int iter = 0; iter < 10; iter++) {
        int n = (iter % 3) + 3;  /* Vary between 3-5 */
        
        /* Call with varying conditions to trigger both paths */
        int res1 = simt_test(n, threshold, iter);
        int res2 = simt_test2(TOTAL, iter > 5);
        
        total_sum += res1 + res2;
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: res1=%d, res2=%d\n", iter, res1, res2);
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with explicit SIMD clause */
    {
        int z[1000];
        #pragma omp parallel for simd simdlen(8)
        for (int i = 0; i < 1000; i++) {
            z[i] = i * 2;
        }
        
        /* Combined construct that might trigger SIMT */
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: z[0:1000]) if(1)
        for (int i = 0; i < 1000; i++) {
            z[i] += i % 3;
        }
    }
    
    return 0;
}
