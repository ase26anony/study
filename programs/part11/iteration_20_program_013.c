/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization of test function */
__attribute__((noinline)) 
int simt_test(int n, int threshold, volatile int iter) {
    volatile int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[TOTAL];
    volatile int b[TOTAL];
    volatile int c[TOTAL];
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel num_threads(2) private(i, j)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a, b, c) num_teams(2) thread_limit(64)
        for (i = 0; i < n; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + (i * j);
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j > 30) {
                    /* Early exit creates additional labels */
                    c[idx] = c[idx] % 100;
                    if (i > n/2) {
                        /* Another nested condition for more blocks */
                        c[idx] += 1;
                    }
                }
                
                /* Force side effect to prevent dead code elimination */
                if (idx % 13 == 0) {
                    c[idx] = c[idx] ^ (i + j);
                }
            }
        }
        
        /* Additional computation outside target to maintain context */
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < TOTAL; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline))
int simt_test2(int size, int flag) {
    volatile int x[256], y[256], z[256];
    int i, j, result = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = 256 - i;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                #pragma omp target teams distribute parallel for simd \
                        if(flag) map(to: x, y) map(from: z) \
                        collapse(2) num_teams(4)
                for (i = 0; i < 16; i++) {
                    for (j = 0; j < 16; j++) {
                        int idx = i * 16 + j;
                        z[idx] = x[idx] * y[idx];
                        
                        /* Complex control flow */
                        switch (idx % 4) {
                            case 0: z[idx] += 1; break;
                            case 1: z[idx] -= 1; break;
                            case 2: z[idx] *= 2; break;
                            case 3: z[idx] /= 2; break;
                        }
                        
                        if (z[idx] < 0) {
                            z[idx] = 0;
                        }
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:result)
        for (i = 0; i < 256; i++) {
            result += z[i];
        }
    }
    
    return result;
}

int main() {
    int total_sum = 0;
    volatile int threshold = 5;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test function multiple times with varying parameters */
    for (int iter = 0; iter < 10; iter++) {
        int n = (iter % 3 == 0) ? 32 : 64;
        
        /* Test both paths of the conditional offloading */
        int result1 = simt_test(n, threshold, iter);
        int result2 = simt_test2(16, iter % 2);
        
        total_sum += result1 + result2;
        
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               iter, result1, result2);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with dynamic teams */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: total_sum) if(total_sum > 1000)
        for (int i = 0; i < 100; i++) {
            #pragma omp atomic
            total_sum += i;
        }
    }
    
    printf("Final total: %d\n", total_sum);
    
    return 0;
}
