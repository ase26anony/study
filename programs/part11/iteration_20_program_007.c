/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent inlining to maintain call structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold, volatile int iter) {
    /* Volatile to prevent optimization */
    volatile int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + iter;
        b[i] = i * 2 + iter;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel num_threads(2)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) if(n > threshold) \
            map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
            num_teams(2) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Main computation - simple arithmetic */
                c[idx] = a[idx] + b[idx] + n;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 1000 && idx % 32 == 0) {
                    /* Dummy operation that can't be optimized away */
                    c[idx] = c[idx] % 256;
                }
                
                /* Another condition to create more control flow */
                if (idx == iter && iter > 50) {
                    /* Early exit-like pattern */
                    c[idx] = -1;
                }
            }
        }
        
        /* Additional computation in parallel region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < TOTAL; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Helper function with different signature */
__attribute__((noinline, noipa))
int simt_test_variant(int n, volatile int flag) {
    volatile int x, y;
    int arr[256];
    int result = 0;
    
    for (x = 0; x < 256; x++) {
        arr[x] = x * n;
    }
    
    /* Different OpenMP construct combination */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: arr) if(flag) \
            num_teams(1) num_threads(64)
        for (x = 0; x < 256; x++) {
            arr[x] = arr[x] * 2 + flag;
            
            /* Complex condition with multiple branches */
            if (arr[x] > 500) {
                arr[x] = arr[x] / 2;
                if (x % 3 == 0) {
                    arr[x] = arr[x] + 100;
                }
            } else if (arr[x] < 100) {
                arr[x] = arr[x] * 3;
            }
        }
        
        #pragma omp for reduction(+:result)
        for (x = 0; x < 256; x++) {
            result += arr[x];
        }
    }
    
    return result;
}

int main(void) {
    int total_sum = 0;
    volatile int iter;
    
    /* Call test function with varying parameters */
    for (iter = 1; iter <= 10; iter++) {
        int threshold = 5;
        int n = iter * 2;
        
        /* Call with different conditions */
        int result1 = simt_test(n, threshold, iter);
        total_sum += result1;
        
        /* Call variant function */
        int result2 = simt_test_variant(n, iter % 2);
        total_sum += result2;
        
        printf("Iteration %d: result1 = %d, result2 = %d, total = %d\n",
               iter, result1, result2, total_sum);
    }
    
    /* Additional test with collapse clause only */
    {
        int arr1[1000], arr2[1000], arr3[1000];
        int i, j;
        
        for (i = 0; i < 1000; i++) {
            arr1[i] = i;
            arr2[i] = i * 2;
        }
        
        #pragma omp target teams distribute parallel for simd collapse(2) \
            map(tofrom: arr1, arr2, arr3) if(1)
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 20; j++) {
                int idx = i * 20 + j;
                arr3[idx] = arr1[idx] + arr2[idx];
                
                /* Conditional that might trigger SIMT wrapper */
                if (arr3[idx] > 500) {
                    arr3[idx] = arr3[idx] - 250;
                }
            }
        }
        
        int check = 0;
        for (i = 0; i < 1000; i++) {
            check += arr3[i];
        }
        printf("Collapse test result: %d\n", check);
    }
    
    return total_sum > 0 ? 0 : 1;
}
