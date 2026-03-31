/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define SIZE (N * M)

/* Prevent optimization of helper function */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold, volatile int iter) {
    volatile int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i + iter;
        b[i] = i * 2 + iter;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel shared(a, b, c) private(i, j)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a[0:SIZE], b[0:SIZE], c[0:SIZE]) \
                num_teams(4) thread_limit(64)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Simple arithmetic operation */
                c[idx] = a[idx] + b[idx] + n;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 10000 && iter % 3 == 0) {
                    /* Early exit creates additional control flow */
                    c[idx] = 0;
                    /* This creates the label structure needed */
                    if (iter % 5 == 0) {
                        /* Nested condition for more complex CFG */
                        c[idx] = -1;
                    }
                }
                
                /* Another condition to force label generation */
                if (c[idx] < 0 && iter > 5) {
                    c[idx] = c[idx] * 2;
                }
            }
        }
        
        /* Additional parallel region to increase complexity */
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different structure */
__attribute__((noinline, noipa))
int simt_test2(int n, volatile int flag) {
    volatile int x[256], y[256], z[256];
    int i, j, sum = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = i * 3;
    }
    
    /* Nested OpenMP with target inside parallel */
    #pragma omp parallel for private(i, j) if(flag > 0)
    for (i = 0; i < 4; i++) {
        #pragma omp target teams distribute parallel for simd \
                map(to: x[i*64:(i+1)*64], y[i*64:(i+1)*64]) \
                map(from: z[i*64:(i+1)*64]) \
                num_teams(2) num_threads(32)
        for (j = i * 64; j < (i + 1) * 64; j++) {
            z[j] = x[j] * y[j] + n;
            
            /* Multiple conditions to create labels */
            if (z[j] % 7 == 0) {
                z[j] = z[j] / 2;
                if (z[j] > 1000) {
                    z[j] = 1000;
                }
            }
        }
        
        #pragma omp atomic
        sum += z[i * 64];
    }
    
    return sum;
}

int main(void) {
    int results[10];
    volatile int threshold = 5;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test function with varying parameters */
    for (volatile int iter = 0; iter < 10; iter++) {
        int n = iter + 1;
        
        /* Mix two different test patterns */
        if (iter % 2 == 0) {
            results[iter] = simt_test(n, threshold, iter);
        } else {
            results[iter] = simt_test2(n, iter);
        }
        
        printf("Iteration %d: result = %d\n", iter, results[iter]);
    }
    
    /* Final computation to ensure everything is used */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += results[i];
    }
    
    printf("Total sum: %d\n", total);
    printf("Test completed.\n");
    
    return total > 0 ? 0 : 1;
}
