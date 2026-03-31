/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization of test function */
__attribute__((noinline, cold))
int simt_test(int n, int threshold, volatile int *result) {
    int i, j;
    int sum = 0;
    
    /* Use volatile to prevent optimization */
    volatile int bound = n > 0 ? n : 1;
    
    /* Arrays with volatile elements to prevent dead code elimination */
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
    #pragma omp parallel private(i, j) shared(a, b, c, bound, threshold)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                    collapse(2) \
                    if(bound > threshold) \
                    map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
                    reduction(+:local_sum)
        for (i = 0; i < bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < TOTAL) {
                    /* Force multiple basic blocks with conditional */
                    if (a[idx] > 50) {
                        /* Early exit simulation - creates extra labels */
                        if (a[idx] > 90 && j % 8 == 0) {
                            /* Dummy operation that can't be optimized away */
                            c[idx] = a[idx] * 2 + b[idx];
                        } else {
                            c[idx] = a[idx] + b[idx];
                        }
                    } else {
                        c[idx] = a[idx] - b[idx];
                    }
                    
                    /* Additional condition to create more control flow */
                    if (c[idx] > 100) {
                        c[idx] = 100;
                    } else if (c[idx] < -100) {
                        c[idx] = -100;
                    }
                    
                    local_sum += c[idx];
                }
            }
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    /* Additional computation to prevent optimization */
    int final_sum = 0;
    for (i = 0; i < TOTAL; i++) {
        final_sum += c[i];
    }
    
    *result = sum + final_sum;
    return *result;
}

/* Another test function with different structure */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int x[256];
    volatile int y[256];
    int i, total = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = 0;
    }
    
    /* Nested OpenMP with target and simd */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                        if(flag) \
                        map(to: x[0:256]) map(from: y[0:256])
            for (i = 0; i < size; i++) {
                /* Complex loop body to encourage SIMT transformation */
                if (i % 3 == 0) {
                    y[i] = x[i] * 2;
                } else if (i % 3 == 1) {
                    y[i] = x[i] / 2;
                } else {
                    y[i] = x[i] + x[255 - i];
                }
                
                /* Early break simulation */
                if (y[i] > 200 && i > 100) {
                    y[i] = 200;
                }
            }
        }
    }
    
    for (i = 0; i < size; i++) {
        total += y[i];
    }
    
    return total;
}

int main(void) {
    int i;
    int results[10];
    volatile int total_result = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test functions with varying parameters */
    for (i = 1; i <= 10; i++) {
        int result;
        
        /* Vary parameters to prevent constant propagation */
        int threshold = i % 3;
        int size = 50 + i * 10;
        
        /* First test - should trigger SIMT with conditional offloading */
        result = simt_test(size, threshold, &result);
        results[i-1] = result;
        total_result += result;
        
        /* Second test - different structure */
        if (i % 2 == 0) {
            int r2 = simt_test2(size, i > 5);
            total_result += r2;
        }
        
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Total result: %d\n", total_result);
    
    /* Additional test with explicit SIMD directive */
    {
        int z[1000];
        #pragma omp parallel for simd
        for (i = 0; i < 1000; i++) {
            z[i] = i * i;
        }
        
        int check = 0;
        for (i = 0; i < 1000; i++) {
            check += z[i];
        }
        printf("SIMD check: %d\n", check);
    }
    
    return total_result > 0 ? 0 : 1;
}
