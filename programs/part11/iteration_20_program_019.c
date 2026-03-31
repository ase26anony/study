/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int iter) {
    volatile int local_bound = M;
    int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[TOTAL];
    volatile int b[TOTAL];
    volatile int c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + iter;
        b[i] = i * 2 + iter;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(n, threshold, local_bound)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a, b, c) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < local_bound; j++) {
                int idx = i * M + j;
                
                /* Complex computation to prevent simplification */
                c[idx] = a[idx] + b[idx] + thread_id + iter;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 1000 && iter % 2 == 0) {
                    /* Early exit creates additional control flow */
                    c[idx] = 999;
                    /* This creates the label structure needed */
                    if (idx % 7 == 0) goto early_exit;
                }
                
                /* Another conditional to split basic blocks */
                if (c[idx] < 0 && n > 5) {
                    c[idx] = c[idx] * 2 - 1;
                }
                
                early_exit:;
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

/* Helper to ensure multiple calls with varying conditions */
__attribute__((noinline))
int driver_function(int base) {
    int results[10];
    int total = 0;
    
    #pragma omp parallel for simd
    for (int i = 0; i < 10; i++) {
        /* Varying conditions to prevent constant propagation */
        int threshold = (base + i) % 5;
        results[i] = simt_test(base + i, threshold, i);
    }
    
    for (int i = 0; i < 10; i++) {
        total += results[i];
    }
    
    return total;
}

int main() {
    int final_result = 0;
    
    /* Multiple iterations with different parameters */
    for (int outer = 0; outer < 3; outer++) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Nested OpenMP for additional complexity */
                #pragma omp taskloop simd
                for (int inner = 0; inner < 4; inner++) {
                    int val = driver_function(outer * 10 + inner);
                    #pragma omp atomic
                    final_result += val;
                }
            }
        }
    }
    
    printf("Final result: %d\n", final_result);
    
    /* Additional test with explicit SIMD directive */
    {
        int test_arr[100];
        #pragma omp simd
        for (int i = 0; i < 100; i++) {
            test_arr[i] = i * 2;
        }
        
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: test_arr) if(1) \
                num_teams(1) num_threads(32)
        for (int i = 0; i < 100; i++) {
            test_arr[i] += final_result % 100;
        }
    }
    
    return final_result > 0 ? 0 : 1;
}
