/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int global_seed = 42;
__attribute__((noinline, cold))
int simt_test(int n, int threshold) {
    volatile int i, j;  /* Prevent loop optimization */
    int sum = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:sum)
    {
        /* Dynamic arrays to force heap allocation */
        int *a = (int*)malloc(TOTAL * sizeof(int));
        int *b = (int*)malloc(TOTAL * sizeof(int));
        int *c = (int*)malloc(TOTAL * sizeof(int));
        
        if (!a || !b || !c) {
            #pragma omp critical
            { printf("Allocation failed\n"); }
            free(a); free(b); free(c);
            return -1;
        }
        
        /* Initialize with volatile to prevent constant propagation */
        for (i = 0; i < TOTAL; i++) {
            a[i] = (i + global_seed) % 100;
            b[i] = (i * 2 + global_seed) % 100;
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) if(n > threshold) \
            map(to: a[0:TOTAL], b[0:TOTAL]) map(from: c[0:TOTAL]) \
            private(i, j)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create conditional basic block inside SIMD loop */
                if (c[idx] > 150) {
                    /* Early exit-like construct to force label generation */
                    c[idx] = 150;  /* Clamp value */
                    /* This creates additional control flow */
                }
                
                /* Another condition to split basic blocks further */
                if (j % 8 == 0 && i > N/2) {
                    c[idx] += 1;  /* Additional operation */
                }
            }
        }
        
        /* Reduction outside target region */
        for (i = 0; i < TOTAL; i++) {
            sum += c[i];
            /* Use result to prevent dead code elimination */
            if (c[i] % 7 == 0) {
                sum += 1;
            }
        }
        
        free(a); free(b); free(c);
    }
    
    return sum;
}

/* Another test function with different structure */
__attribute__((noinline, cold))
int simt_test2(int size, int threshold) {
    volatile int x, y;
    int result = 0;
    
    #pragma omp parallel
    {
        int arr1[256], arr2[256], arr3[256];
        
        /* Initialize */
        for (x = 0; x < 256; x++) {
            arr1[x] = x * size;
            arr2[x] = x + size;
        }
        
        /* Nested target with teams and simd */
        #pragma omp target teams distribute parallel for simd \
            if(size > threshold) \
            map(to: arr1, arr2) map(from: arr3) \
            collapse(2) schedule(static, 4)
        for (x = 0; x < 16; x++) {
            for (y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                arr3[idx] = arr1[idx] * arr2[idx];
                
                /* Multiple conditions to encourage label generation */
                if (arr3[idx] > 1000) {
                    arr3[idx] = arr3[idx] % 1000;
                    if (x == y) {
                        arr3[idx] += 100;
                    }
                } else if (arr3[idx] < 0) {
                    arr3[idx] = 0;
                }
            }
        }
        
        /* Process results */
        for (x = 0; x < 256; x++) {
            result += arr3[x];
        }
    }
    
    return result;
}

int main(void) {
    int total_sum = 0;
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Varying parameters to prevent constant propagation */
    for (int iter = 1; iter <= 10; iter++) {
        int threshold = iter * 5;
        
        /* Test with different conditions to trigger both paths */
        int res1 = simt_test(iter * 8, threshold);
        int res2 = simt_test2(iter * 4, threshold / 2);
        
        total_sum += res1 + res2;
        
        printf("Iteration %d: res1=%d, res2=%d\n", iter, res1, res2);
        
        /* Modify global volatile to affect future iterations */
        global_seed += iter;
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with explicit SIMD directive */
    {
        int arr[1000];
        #pragma omp parallel for simd
        for (int i = 0; i < 1000; i++) {
            arr[i] = i * i;
        }
        
        int check = 0;
        for (int i = 0; i < 1000; i++) {
            check += arr[i];
        }
        printf("SIMD test check: %d\n", check);
    }
    
    return total_sum > 0 ? 0 : 1;
}
