/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
volatile int g_volatile_threshold = 5;
static int __attribute__((noinline)) dummy_side_effect = 0;

/* Helper function marked noinline to preserve structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold) {
    volatile int local_bound = M;  /* Prevent constant propagation */
    int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:sum)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(tofrom: a, b, c) \
                num_teams(2) \
                thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < local_bound; j++) {
                int idx = i * M + j;
                
                /* Force multiple basic blocks with conditional */
                if (thread_id % 2 == 0) {
                    c[idx] = a[idx] + b[idx] + n;
                } else {
                    c[idx] = a[idx] - b[idx] + n;
                }
                
                /* Early exit condition to create label jumps */
                if (c[idx] > 200) {
                    /* Dummy operation that can't be optimized away */
                    dummy_side_effect += 1;
                    /* Continue anyway - don't actually break */
                }
                
                /* Another condition to create more control flow */
                if (c[idx] < 0 && n > 10) {
                    c[idx] = 0;
                }
            }
        }
        
        /* Reduction on host side */
        #pragma omp for collapse(2)
        for (i = 0; i < n % N; i++) {
            for (j = 0; j < n % M; j++) {
                int idx = i * M + j;
                if (idx < SIZE) {
                    sum += c[idx];
                }
            }
        }
    }
    
    return sum + dummy_side_effect;
}

/* Another test with different construct nesting */
__attribute__((noinline, noipa))
int simt_test2(int n, int flag) {
    volatile int arr[256];
    int i, j, sum = 0;
    
    for (i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp target teams distribute parallel for simd \
                        if(flag) \
                        map(tofrom: arr) \
                        num_teams(1) \
                        num_threads(64)
                for (i = 0; i < 256; i++) {
                    /* Complex operation with multiple basic blocks */
                    int val = arr[i];
                    if (val % 3 == 0) {
                        arr[i] = val * 2 + n;
                    } else if (val % 3 == 1) {
                        arr[i] = val / 2 + n;
                        if (arr[i] > 100) {
                            arr[i] = 100;
                        }
                    } else {
                        arr[i] = val + n;
                        /* Force goto-like structure */
                        for (j = 0; j < 2; j++) {
                            arr[i] += j;
                        }
                    }
                }
            }
        }
        
        #pragma omp for simd reduction(+:sum)
        for (i = 0; i < 256; i++) {
            sum += arr[i];
        }
    }
    
    return sum;
}

int main() {
    int i, total = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int result1 = simt_test(i, g_volatile_threshold);
        int result2 = simt_test2(i, i % 2);
        
        total += result1 + result2;
        
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total sum: %d\n", total);
    printf("Dummy side effect counter: %d\n", dummy_side_effect);
    
    /* Use results to prevent dead code elimination */
    if (total > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
