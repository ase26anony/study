/* Test program to trigger SIMT transformation in GCC omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization */
volatile int global_threshold = 5;
volatile int use_offload = 1;

/* Helper function marked noinline to preserve structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold) {
    volatile int i, j;  /* volatile to prevent optimization */
    int sum = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel reduction(+:sum) if(use_offload)
    {
        /* Dynamic arrays to force heap allocation */
        int *a = (int*)malloc(SIZE * sizeof(int));
        int *b = (int*)malloc(SIZE * sizeof(int));
        int *c = (int*)malloc(SIZE * sizeof(int));
        
        if (!a || !b || !c) {
            #pragma omp single
            printf("Allocation failed\n");
            free(a); free(b); free(c);
            return -1;
        }
        
        /* Initialize arrays */
        #pragma omp for simd collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                a[idx] = i + j;
                b[idx] = i * j;
                c[idx] = 0;
            }
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:SIZE], b[0:SIZE]) \
                map(from: c[0:SIZE]) \
                num_teams(4) thread_limit(64)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx];
                
                /* Create internal basic block split */
                if (c[idx] > 100) {
                    /* Dummy operation to prevent dead code elimination */
                    c[idx] = c[idx] % 97;
                    /* Early break simulation */
                    if (c[idx] == 0 && i > N/2) {
                        /* This creates additional control flow */
                        c[idx] = 1;
                    }
                } else {
                    /* Alternative path */
                    c[idx] = c[idx] * 2;
                }
            }
        }
        
        /* Verify and reduce results */
        #pragma omp for simd collapse(2) reduction(+:sum)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                int expected = (i + j) + (i * j);
                if (c[idx] != expected * 2 && c[idx] != (expected % 97)) {
                    sum += c[idx];
                }
            }
        }
        
        free(a); free(b); free(c);
    }
    
    return sum;
}

/* Another test with different loop structure */
__attribute__((noinline, noipa))
int simt_test2(int n) {
    volatile int i, j, k;
    int arr[64][64];
    int sum = 0;
    
    /* Complex nesting */
    #pragma omp parallel private(i, j, k) reduction(+:sum)
    {
        #pragma omp target teams distribute parallel for simd \
                collapse(3) if(n > 2) \
                map(tofrom: arr) \
                num_teams(2) num_threads(32)
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                for (k = 0; k < 64; k++) {
                    arr[i][j] = i * j + k;
                    
                    /* Multiple conditional blocks */
                    if (arr[i][j] > 1000) {
                        arr[i][j] /= 2;
                        if (k % 7 == 0) {
                            arr[i][j] += k;
                        }
                    } else if (arr[i][j] < 100) {
                        arr[i][j] *= 3;
                    }
                    
                    /* Early exit-like pattern */
                    if (i > 50 && arr[i][j] > 2000) {
                        arr[i][j] = 2000;
                    }
                }
            }
        }
        
        /* Process results */
        #pragma omp for simd collapse(2)
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                sum += arr[i][j];
            }
        }
    }
    
    return sum;
}

int main() {
    int results[10];
    int i;
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 0; i < 10; i++) {
        results[i] = simt_test(i, global_threshold);
        printf("Test %d: result = %d\n", i, results[i]);
    }
    
    /* Second test with different parameters */
    int total = 0;
    #pragma omp parallel for reduction(+:total) if(use_offload)
    for (i = 0; i < 5; i++) {
        total += simt_test2(i * 3);
    }
    printf("Total from second test: %d\n", total);
    
    /* Mix of conditions */
    #pragma omp target teams distribute parallel for simd \
            if(use_offload) map(tofrom: results) \
            num_teams(1) num_threads(4)
    for (i = 0; i < 10; i++) {
        if (results[i] > 0) {
            results[i] = results[i] * 2;
        } else {
            results[i] = -results[i];
        }
    }
    
    /* Final verification */
    int final_sum = 0;
    for (i = 0; i < 10; i++) {
        final_sum += results[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
