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
    volatile int bound = g_volatile_bound;
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, idx;
    int sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j, idx) shared(a, b, c, bound, sum)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                private(i, j, idx) reduction(+:local_sum)
        for (i = 0; i < bound; i++) {
            for (j = 0; j < M; j++) {
                idx = i * M + j;
                c[idx] = a[idx] + b[idx] + (iter % 10);
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j > M/2) {
                    /* Early exit creates additional control flow */
                    c[idx] = 150;  /* Force value limit */
                    if (i > bound/2) {
                        /* Another nested condition for more blocks */
                        local_sum += 1;
                    }
                } else if (c[idx] < 0) {
                    c[idx] = 0;
                }
                
                /* Volatile access to prevent optimization */
                int volatile tmp = c[idx];
                local_sum += tmp & 1;  /* Simple reduction */
            }
        }
        
        #pragma omp atomic
        sum += local_sum;
        
        /* Additional parallel region inside to increase complexity */
        #pragma omp single
        {
            int dummy = 0;
            #pragma omp simd reduction(+:dummy)
            for (idx = 0; idx < TOTAL; idx += 16) {
                dummy += c[idx];
            }
            sum += dummy & 1;
        }
    }
    
    /* Final reduction with volatile to prevent dead code elimination */
    volatile int final_sum = sum;
    return final_sum;
}

/* Helper to ensure multiple calls with varying conditions */
__attribute__((noinline))
int test_driver(int base, int range) {
    int total = 0;
    int i;
    
    #pragma omp parallel for reduction(+:total) if(range > 5)
    for (i = 0; i < range; i++) {
        /* Varying thresholds to trigger different paths */
        int threshold = (i % 3) + 1;
        total += simt_test(base + i, threshold, i);
    }
    
    return total;
}

int main(void) {
    int results[10];
    int i, final_result = 0;
    
    /* Multiple invocations with different parameters */
    for (i = 0; i < 10; i++) {
        results[i] = test_driver(i, 8 + (i % 3));
        printf("Iteration %d: result = %d\n", i, results[i]);
        final_result += results[i];
    }
    
    /* Use result to prevent optimization */
    volatile int output = final_result;
    printf("Final result: %d\n", output);
    
    return output != 0 ? 0 : 1;
}
