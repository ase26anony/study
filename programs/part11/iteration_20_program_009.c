/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Use volatile to prevent optimization */
volatile int global_threshold = 5;

/* Mark as noinline to preserve structure */
__attribute__((noinline))
int simt_test(int n, int threshold) {
    volatile int i, j;  /* Prevent loop optimization */
    int sum = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel reduction(+:sum)
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
                a[idx] = i + j + n;
                b[idx] = i * j - n;
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
                c[idx] = a[idx] + b[idx];
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 1000 && n > 20) {
                    /* Early exit creates additional labels */
                    c[idx] = 1000;
                    /* Could use break here for more complex flow */
                }
                
                /* Another condition to split basic blocks */
                if (c[idx] < 0 && threshold < 0) {
                    c[idx] = 0;
                }
            }
        }
        
        /* Reduction with another parallel region */
        #pragma omp for simd reduction(+:sum) collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                sum += c[idx];
                
                /* Additional condition to prevent vectorization elimination */
                if (sum > 1000000) {
                    sum = sum % 1000;
                }
            }
        }
        
        free(a); free(b); free(c);
    }
    
    return sum;
}

/* Another test function with different structure */
__attribute__((noinline))
int simt_test2(int n, int *output) {
    volatile int i, j, k;
    int total = 0;
    
    /* Triple nested loop with collapse */
    #pragma omp parallel reduction(+:total)
    {
        int arr1[N][M];
        int arr2[N][M];
        
        /* Initialize */
        #pragma omp for simd collapse(2)
        for (i = 0; i < N; i++)
            for (j = 0; j < M; j++)
                arr1[i][j] = i * j + n;
        
        /* Complex target region with multiple clauses */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) if(n > global_threshold) \
            map(tofrom: arr1, arr2) \
            num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                arr2[i][j] = arr1[i][j] * 2;
                
                /* Force multiple basic blocks with goto-like logic */
                if (arr2[i][j] > 500) {
                    arr2[i][j] = arr2[i][j] / 2;
                    if (arr2[i][j] < 100) {
                        arr2[i][j] = 100;
                    }
                } else {
                    arr2[i][j] = arr2[i][j] + 1;
                }
            }
        }
        
        /* Final reduction with early exit possibility */
        #pragma omp for simd collapse(2) reduction(+:total)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                total += arr2[i][j];
                
                /* Conditional that might create extra labels */
                if (total > 10000 && n > 10) {
                    total = total - 5000;
                }
            }
        }
    }
    
    *output = total;
    return total;
}

int main() {
    int results[10];
    int i;
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 0; i < 10; i++) {
        int threshold = i % 3;
        int result = simt_test(i + 1, threshold);
        results[i] = result;
        printf("Test %d: result = %d\n", i, result);
        
        /* Call second test function */
        int output;
        result = simt_test2(i + 1, &output);
        printf("Test2 %d: result = %d, output = %d\n", i, result, output);
    }
    
    /* Final check to use results */
    int final_sum = 0;
    #pragma omp simd reduction(+:final_sum)
    for (i = 0; i < 10; i++) {
        final_sum += results[i];
    }
    
    printf("Final sum: %d\n", final_sum);
    
    /* Additional test with dynamic parallelism */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                int r = simt_test(15, 10);
                printf("Task result: %d\n", r);
            }
        }
    }
    
    return 0;
}
