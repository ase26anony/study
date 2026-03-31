/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization of helper functions */
__attribute__((noinline)) 
int simt_test(int n, int threshold) {
    volatile int size = SIZE;  /* volatile to prevent optimization */
    int a[SIZE], b[SIZE], c[SIZE];
    int i, j;
    int sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) shared(a, b, c, size, n, threshold)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(tofrom: a[0:size], b[0:size], c[0:size]) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150) {
                    /* Early exit simulation - creates extra basic blocks */
                    c[idx] = 150;  /* Cap the value */
                    /* This creates additional control flow */
                }
                
                /* Another condition to force label generation */
                if (j == M-1 && i % 2 == 0) {
                    /* Dummy operation to prevent dead code elimination */
                    c[idx] += 1;
                }
            }
        }
        
        /* Reduction after target region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different loop structure */
__attribute__((noinline))
int simt_test2(int n, int flag) {
    volatile int dim = 16;
    int x[256], y[256], z[256];
    int i, j, k;
    int result = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = 256 - i;
    }
    
    /* Complex nesting to trigger GIMPLE sequence duplication */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp target teams distribute parallel for simd \
                        if(flag) \
                        map(to: x[0:256]) \
                        map(from: z[0:256]) \
                        collapse(2)
                for (i = 0; i < dim; i++) {
                    for (j = 0; j < dim; j++) {
                        int idx = i * dim + j;
                        z[idx] = x[idx] * y[idx] + n;
                        
                        /* Force multiple control flow paths */
                        switch (idx % 3) {
                            case 0: z[idx] += 1; break;
                            case 1: z[idx] -= 1; break;
                            case 2: z[idx] *= 2; break;
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
    int i;
    int total = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test functions with varying parameters to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int flag = i % 2;
        
        int result1 = simt_test(i, threshold);
        int result2 = simt_test2(i, flag);
        
        total += result1 + result2;
        
        printf("Iteration %d: test1=%d, test2=%d\n", i, result1, result2);
    }
    
    printf("Total sum: %d\n", total);
    printf("Test completed.\n");
    
    /* Verify results aren't optimized away */
    if (total == 0) {
        printf("Warning: Results may have been optimized away\n");
    }
    
    return 0;
}
