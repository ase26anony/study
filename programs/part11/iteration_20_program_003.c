/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization of helper functions */
__attribute__((noinline)) 
int simt_test(int n, int threshold, int iter) {
    volatile int size = SIZE; /* volatile to prevent optimization */
    int a[SIZE], b[SIZE], c[SIZE];
    int i, j;
    int sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < SIZE; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel shared(a, b, c) private(i, j)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(tofrom: a[0:size], b[0:size], c[0:size]) \
                num_teams(4) thread_limit(64)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create internal basic block split - early exit condition */
                if (c[idx] > 150 && iter % 2 == 0) {
                    /* This creates additional basic blocks and control flow */
                    c[idx] = 150; /* Cap the value */
                    /* Could add a break here for more complex flow, but 
                       breaks aren't allowed in OpenMP loops */
                }
                
                /* Additional arithmetic to prevent dead code elimination */
                c[idx] += (i * j) % 7;
            }
        }
        
        /* Additional parallel work in host thread */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline))
int simt_test2(int n, int flag) {
    volatile int dim = 16;
    int x[256], y[256], z[256];
    int i, j;
    int result = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i * 3;
        y[i] = i * 5;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(flag) \
                    map(to: x[0:256], y[0:256]) map(from: z[0:256]) \
                    num_teams(2)
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    z[idx] = x[idx] * y[idx] + n;
                    
                    /* Multiple basic blocks within loop */
                    if (z[idx] % 3 == 0) {
                        z[idx] += 1;
                    } else if (z[idx] % 5 == 0) {
                        z[idx] += 2;
                    } else {
                        z[idx] += 3;
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
    int total = 0;
    int i;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test functions with varying parameters to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold, i);
        int result2 = simt_test2(i, i % 2);
        
        total += result1 + result2;
        printf("Iteration %d: result1 = %d, result2 = %d\n", i, result1, result2);
    }
    
    printf("Total sum: %d\n", total);
    
    /* Additional test with larger data */
    volatile int big_n = 1000;
    int *big_a = (int*)malloc(big_n * sizeof(int));
    int *big_b = (int*)malloc(big_n * sizeof(int));
    int *big_c = (int*)malloc(big_n * sizeof(int));
    
    if (big_a && big_b && big_c) {
        #pragma omp parallel for simd
        for (i = 0; i < big_n; i++) {
            big_a[i] = i;
            big_b[i] = big_n - i;
        }
        
        #pragma omp target teams distribute parallel for simd \
                map(to: big_a[0:big_n], big_b[0:big_n]) \
                map(from: big_c[0:big_n]) \
                if(big_n > 100)
        for (i = 0; i < big_n; i++) {
            big_c[i] = big_a[i] + big_b[i];
            /* Complex conditional to encourage SIMT transformation */
            big_c[i] = (big_c[i] % 2 == 0) ? big_c[i] / 2 : big_c[i] * 3 + 1;
        }
        
        int check = 0;
        #pragma omp parallel for reduction(+:check)
        for (i = 0; i < big_n; i++) {
            check += big_c[i];
        }
        printf("Large array check sum: %d\n", check);
    }
    
    free(big_a);
    free(big_b);
    free(big_c);
    
    return 0;
}
