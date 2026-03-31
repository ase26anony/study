/* Test program to trigger SIMT transformation in GCC omp-low.cc */
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
    #pragma omp parallel reduction(+:result)
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
        
        /* Initialize arrays */
        #pragma omp for simd collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                a[idx] = (i + j) % 100;
                b[idx] = (i * j) % 100;
                c[idx] = 0;
            }
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:SIZE], b[0:SIZE]) map(from: c[0:SIZE]) \
                private(i, j)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150) {
                    /* Early exit simulation - creates extra labels */
                    c[idx] = 150;  /* Cap value */
                    /* This creates additional control flow */
                    if (idx % 7 == 0) {
                        /* Dummy operation to prevent dead code elimination */
                        c[idx] += (global_seed % 5);
                    }
                } else if (c[idx] < 0) {
                    /* Another basic block */
                    c[idx] = 0;
                }
                
                /* Force SIMD transformation with reduction-like pattern */
                result += (c[idx] % 10);
            }
        }
        
        /* Verify computation on host */
        #pragma omp for simd collapse(2) reduction(+:result)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                int expected = ((i + j) % 100) + ((i * j) % 100) + n;
                if (expected > 150) expected = 150 + (global_seed % 5);
                else if (expected < 0) expected = 0;
                
                if (c[idx] != expected) {
                    result += 1;  /* Count mismatches */
                }
            }
        }
        
        free(a); free(b); free(c);
    }
    
    return result;
}

/* Another test function with different pattern */
__attribute__((noinline, cold))
int simt_test2(int n, int flag) {
    volatile int x, y;
    int arr1[256], arr2[256], arr3[256];
    int sum = 0;
    
    /* Initialize with volatile to prevent constant propagation */
    volatile int init_val = n;
    
    #pragma omp parallel for simd
    for (x = 0; x < 256; x++) {
        arr1[x] = x + init_val;
        arr2[x] = x * init_val;
    }
    
    /* Nested OpenMP with target and simd */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                if(flag) map(to: arr1, arr2) map(from: arr3) \
                collapse(2) num_teams(2) thread_limit(128)
        for (x = 0; x < 16; x++) {
            for (y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                arr3[idx] = arr1[idx] * arr2[idx];
                
                /* Complex control flow to generate multiple labels */
                switch (arr3[idx] % 4) {
                    case 0:
                        arr3[idx] += 1;
                        break;
                    case 1:
                        arr3[idx] -= 1;
                        /* Fall through to create more edges */
                    case 2:
                        arr3[idx] *= 2;
                        break;
                    default:
                        arr3[idx] /= 2;
                        if (arr3[idx] > 1000) {
                            arr3[idx] = 1000;
                        }
                }
            }
        }
        
        /* Reduction with atomic to force synchronization */
        #pragma omp for reduction(+:sum)
        for (x = 0; x < 256; x++) {
            sum += arr3[x];
        }
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Varying arguments to prevent constant propagation */
    for (int iter = 0; iter < 10; iter++) {
        int threshold = iter * 3;
        int n = iter * 5 + 1;
        
        /* Call test functions with varying conditions */
        int res1 = simt_test(n, threshold);
        int res2 = simt_test2(n, iter % 2);
        
        total += res1 + res2;
        
        /* Print to prevent dead code elimination */
        if (iter % 3 == 0) {
            printf("Iteration %d: res1=%d, res2=%d\n", iter, res1, res2);
        }
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with large collapse factor */
    {
        #pragma omp target teams distribute parallel for simd \
                collapse(3) map(tofrom: total)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    total += i * j * k;
                }
            }
        }
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
