/* Test program to trigger SIMT transformation in GCC omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
volatile int g_volatile_threshold = 5;
static int __attribute__((noinline)) dummy_work = 0;

/* Helper function marked noinline to preserve structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold, int use_simd) {
    volatile int local_volatile = n;
    int i, j;
    int result = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[TOTAL];
    volatile int b[TOTAL];
    volatile int c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(local_volatile)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(local_volatile > threshold) \
                map(tofrom: a, b, c) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Force basic block split with conditional */
                if (use_simd && (thread_id % 2 == 0)) {
                    /* SIMD-friendly computation */
                    c[idx] = a[idx] + b[idx] + (i * j);
                } else {
                    /* Different computation path */
                    c[idx] = a[idx] - b[idx] + (i + j);
                }
                
                /* Additional conditional to create more basic blocks */
                if (c[idx] > 1000) {
                    /* Early exit simulation - creates jump labels */
                    c[idx] = 1000;
                    if (idx == TOTAL - 1) {
                        /* This creates additional control flow */
                        dummy_work = 1;
                    }
                }
                
                /* Another conditional to encourage label generation */
                if (i > n / 2 && j < M / 2) {
                    c[idx] *= 2;
                }
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for reduction(+:result)
        for (i = 0; i < TOTAL; i++) {
            result += c[i];
        }
    }
    
    return result;
}

/* Another test with different loop structure */
__attribute__((noinline, noipa))
int simt_test_nested(int size, int depth) {
    volatile int bound1 = size;
    volatile int bound2 = size / 2;
    int i, j, k;
    int sum = 0;
    
    volatile int arr1[size * size];
    volatile int arr2[size * size];
    
    /* Initialize */
    for (i = 0; i < size * size; i++) {
        arr1[i] = i;
        arr2[i] = size * size - i;
    }
    
    /* Complex nesting structure */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Teams with distribute */
            #pragma omp target teams distribute \
                    if(depth > 2) \
                    map(to: arr1) map(from: arr2) \
                    num_teams(4)
            for (i = 0; i < bound1; i++) {
                /* Parallel for simd inside teams */
                #pragma omp parallel for simd \
                        collapse(2) \
                        if(i % 2 == 0)
                for (j = 0; j < bound2; j++) {
                    for (k = 0; k < depth; k++) {
                        int idx = i * size + j;
                        
                        /* Multiple conditionals for control flow */
                        if (k == 0) {
                            arr2[idx] = arr1[idx] * 2;
                        } else if (k == depth - 1) {
                            arr2[idx] = arr1[idx] / 2;
                        } else {
                            arr2[idx] = arr1[idx] + k;
                        }
                        
                        /* Break-like condition */
                        if (arr2[idx] > 10000) {
                            arr2[idx] = 10000;
                            goto early_exit;
                        }
                        
                        early_exit:
                        /* Empty label to force label creation */
                        if (arr2[idx] < 0) {
                            arr2[idx] = 0;
                        }
                    }
                }
            }
        }
        
        /* Final reduction */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < size * size; i++) {
            sum += arr2[i];
        }
    }
    
    return sum;
}

int main() {
    int i;
    int total_result = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying parameters to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = g_volatile_threshold;
        int use_simd = i % 2;
        
        /* Call with different conditions */
        int result1 = simt_test(i, threshold, use_simd);
        int result2 = simt_test_nested(i + 5, i);
        
        total_result += result1 + result2;
        
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total result: %d\n", total_result);
    printf("Dummy work flag: %d\n", dummy_work);
    
    return total_result > 0 ? 0 : 1;
}
