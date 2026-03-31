/* simt_test.c - Program to trigger SIMT transformation in GCC omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int global_seed = 42;
__attribute__((noinline, cold))
int simt_test(int n, int threshold) {
    volatile int i, j;  /* volatile to prevent optimization */
    int result = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:result)
    {
        /* Dynamic arrays to force heap allocation */
        int *a = (int*)malloc(SIZE * sizeof(int));
        int *b = (int*)malloc(SIZE * sizeof(int));
        int *c = (int*)malloc(SIZE * sizeof(int));
        
        if (!a || !b || !c) {
            #pragma omp single
            fprintf(stderr, "Allocation failed\n");
            free(a); free(b); free(c);
            return -1;
        }
        
        /* Initialize arrays with non-constant values */
        #pragma omp for simd collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                a[idx] = (i + j + global_seed) % 100;
                b[idx] = (i * j + global_seed) % 100;
                c[idx] = 0;
            }
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) if(n > threshold) \
            map(to: a[0:SIZE], b[0:SIZE]) map(from: c[0:SIZE]) \
            num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx];
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150) {  /* This condition is unlikely but creates BB split */
                    /* Early exit path - forces label generation */
                    c[idx] = 255;  /* Saturate value */
                    /* Cannot use break in SIMD loop, but creates control flow */
                }
                
                /* Additional computation to prevent dead code elimination */
                if ((i * j) % 7 == 0) {
                    c[idx] += (n % 5);
                }
            }
        }
        
        /* Verify and reduce results */
        #pragma omp for simd collapse(2) reduction(+:result)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                int expected = ((i + j + global_seed) % 100) + 
                               ((i * j + global_seed) % 100);
                if ((i * j) % 7 == 0) {
                    expected += (n % 5);
                }
                
                /* Force use of result to prevent optimization */
                result += (c[idx] - expected);
                
                /* Additional branching for control flow complexity */
                if (c[idx] != expected && n > 1) {
                    result += 1;
                }
            }
        }
        
        free(a); free(b); free(c);
    }
    
    return result;
}

/* Another test function with different pattern */
__attribute__((noinline, cold))
int simt_test2(int n, int threshold) {
    volatile int i, j, k;
    int sum = 0;
    int arr[64][64];
    
    /* Initialize with volatile to prevent constant propagation */
    volatile int init_val = n + global_seed;
    
    #pragma omp parallel private(i, j, k) reduction(+:sum)
    {
        /* Triple nested loop with collapse */
        #pragma omp target teams distribute parallel for simd \
            collapse(3) if(threshold > 0) \
            map(tofrom: arr) \
            num_teams(2) num_threads(64)
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                for (k = 0; k < 2; k++) {  /* Small innermost for collapse */
                    int val = i * 1000 + j * 100 + k * 10 + init_val;
                    arr[i][j] = val % 1000;
                    
                    /* Complex conditional with multiple basic blocks */
                    if (arr[i][j] > 500) {
                        arr[i][j] = arr[i][j] * 2;
                        if (arr[i][j] > 900) {
                            arr[i][j] = 900;
                        }
                    } else if (arr[i][j] < 100) {
                        arr[i][j] = arr[i][j] + 50;
                    }
                }
            }
        }
        
        /* Reduction with early exit simulation */
        #pragma omp for simd collapse(2)
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                sum += arr[i][j];
                /* Simulate early exit with conditional */
                if (sum > 1000000 && n > 5) {
                    sum = 1000000;  /* Cap the sum */
                }
            }
        }
    }
    
    return sum;
}

int main() {
    int total = 0;
    volatile int threshold = 5;  /* volatile to prevent constant propagation */
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Call test functions with varying parameters to prevent optimization */
    for (int iter = 0; iter < 10; iter++) {
        int n = iter + 1;
        
        /* Mix two different test patterns */
        if (iter % 2 == 0) {
            int res = simt_test(n, threshold);
            printf("Iteration %d, simt_test returned: %d\n", iter, res);
            total += res;
        } else {
            int res = simt_test2(n, threshold);
            printf("Iteration %d, simt_test2 returned: %d\n", iter, res);
            total += res;
        }
        
        /* Modify threshold to affect conditional offloading */
        threshold += (iter % 3);
    }
    
    printf("Total result: %d\n", total);
    printf("Global seed was: %d\n", global_seed);
    
    return total != 0 ? 0 : 1;
}
