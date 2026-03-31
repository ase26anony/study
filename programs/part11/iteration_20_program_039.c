/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int global_seed = 42;
__attribute__((noinline, noipa))
int simt_test(int n, int threshold, int iter) {
    volatile int use_offload = (n > threshold);
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter + global_seed) % 100;
        b[i] = (i * 2 + iter) % 100;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel shared(a, b, c) private(i, j)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
                if(n > threshold) reduction(+:local_sum)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + (iter % 10);
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j > M/2) {
                    /* Early exit creates extra labels and jumps */
                    c[idx] = 150;  /* Force clipping */
                    if (i > N/2) {
                        /* Another nested condition for more blocks */
                        local_sum += 1;
                    }
                } else if (c[idx] < 0) {
                    c[idx] = 0;
                }
                
                /* Force SIMD vectorization with reduction */
                local_sum += (c[idx] % 10);
            }
        }
        
        #pragma omp atomic
        sum += local_sum;
        
        /* Additional nested parallel for to increase complexity */
        #pragma omp for simd nowait
        for (i = 0; i < TOTAL; i++) {
            if (c[i] % 3 == 0) {
                c[i] += iter;
            }
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) simd
    for (i = 0; i < TOTAL; i++) {
        final_sum += c[i];
        if (final_sum > 1000000) {
            final_sum = 1000000;  /* Another basic block split */
        }
    }
    
    return final_sum + sum;
}

/* Helper with volatile to prevent constant propagation */
__attribute__((noinline))
int get_threshold() {
    volatile int t = 5;
    return t;
}

int main() {
    int results[10];
    int i, total_result = 0;
    
    /* Varying parameters to prevent constant folding */
    int threshold = get_threshold();
    
    for (i = 0; i < 10; i++) {
        /* Mix of conditions to trigger both paths */
        int n = (i % 3 == 0) ? 10 : 2;  /* Sometimes n > threshold, sometimes not */
        results[i] = simt_test(n, threshold, i);
        total_result += results[i];
        
        /* Print to prevent optimization */
        if (i % 4 == 0) {
            printf("Iteration %d: result = %d\n", i, results[i]);
        }
    }
    
    printf("Total result: %d\n", total_result);
    
    /* Use result to prevent dead code elimination */
    if (total_result > 1000) {
        return 0;
    } else {
        return 1;
    }
}
