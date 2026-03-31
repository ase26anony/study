/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization of helper functions */
__attribute__((noinline, used))
int simt_test(int n, int threshold, volatile int *result) {
    volatile int i, j;
    int a[SIZE], b[SIZE], c[SIZE];
    int sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
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
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create internal basic block split with early exit pattern */
                if (c[idx] > 150) {
                    /* Force multiple basic blocks - early exit simulation */
                    c[idx] = 150;  /* This creates a separate basic block */
                }
                
                /* Additional computation to prevent loop simplification */
                if (j % 8 == 0) {
                    c[idx] += (i % 4);
                }
            }
        }
        
        /* Additional parallel region to increase complexity */
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    *result = sum;
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, used))
void nested_simt_test(int n, volatile int *arr) {
    volatile int x, y;
    
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp target teams distribute parallel for simd \
                    map(tofrom: arr[0:SIZE]) \
                    if(n > 5) \
                    collapse(2)
            for (x = 0; x < N/2; x++) {
                for (y = 0; y < M/2; y++) {
                    int idx = x * M + y;
                    arr[idx] = arr[idx] * 2 + n;
                    
                    /* Complex conditional to force label generation */
                    if (arr[idx] > 1000) {
                        arr[idx] = 1000;
                        /* Potential early return simulation */
                        if (x > 10 && y > 10) {
                            arr[idx] = 999;  /* Another basic block */
                        }
                    }
                }
            }
        }
        
        /* Barrier to ensure synchronization */
        #pragma omp barrier
    }
}

int main() {
    volatile int results[10];
    volatile int test_arr[SIZE];
    int i, total = 0;
    
    /* Initialize test array */
    for (i = 0; i < SIZE; i++) {
        test_arr[i] = i;
    }
    
    /* Call test functions with varying parameters to prevent constant propagation */
    for (i = 0; i < 10; i++) {
        int threshold = (i % 3) + 2;  /* Varying threshold */
        int result;
        
        /* Test with different conditions */
        simt_test(i + 1, threshold, &result);
        results[i] = result;
        total += result;
        
        /* Call nested test every other iteration */
        if (i % 2 == 0) {
            nested_simt_test(i, test_arr);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total);
    for (i = 0; i < 10; i++) {
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    /* Final verification sum */
    int final_sum = 0;
    for (i = 0; i < SIZE; i++) {
        final_sum += test_arr[i];
    }
    printf("Final array sum: %d\n", final_sum);
    
    return 0;
}
