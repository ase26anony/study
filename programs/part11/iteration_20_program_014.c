/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 8

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline,noipa))
int simt_test(int n, int threshold, int *result) {
    volatile int local_bound = M;
    int i, j;
    int sum = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel reduction(+:sum)
    {
        int a[N*M], b[N*M], c[N*M];
        
        /* Initialize arrays */
        #pragma omp for simd collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                a[i*M + j] = i + j;
                b[i*M + j] = i - j;
                c[i*M + j] = 0;
            }
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a, b, c) num_teams(4) thread_limit(128)
        for (i = 0; i < local_bound; i++) {  /* volatile bound */
            for (j = 0; j < g_volatile_bound; j++) {  /* global volatile bound */
                int idx = i*N + j;
                c[idx] = a[idx] + b[idx];
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 1000) {
                    /* Early exit path - forces label generation */
                    c[idx] = 1000;
                    /* Continue instead of break to maintain loop structure */
                }
                
                /* Additional condition to create more control flow */
                if (j % 2 == 0) {
                    c[idx] += 1;
                } else {
                    c[idx] -= 1;
                }
            }
        }
        
        /* Reduction with another parallel loop */
        #pragma omp for simd reduction(+:sum)
        for (i = 0; i < N*M; i++) {
            sum += c[i];
        }
    }
    
    *result = sum;
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline,noipa))
int simt_test2(int n, int *arr) {
    int i, j;
    int sum = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Combined construct that may trigger SIMT */
            #pragma omp target teams distribute parallel for simd \
                    if(n > 5) map(tofrom: arr[0:N*N]) \
                    num_teams(2) num_threads(64)
            for (i = 0; i < N; i++) {
                for (j = 0; j < N; j++) {
                    int idx = i*N + j;
                    arr[idx] = arr[idx] * 2 + i - j;
                    
                    /* Complex control flow with multiple exits */
                    if (arr[idx] < 0) {
                        arr[idx] = 0;
                        if (i > j) {
                            arr[idx] += 100;
                        }
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:sum)
        for (i = 0; i < N*N; i++) {
            sum += arr[i];
        }
    }
    
    return sum;
}

int main() {
    int i, results[10];
    int arr[N*N];
    
    /* Initialize array */
    for (i = 0; i < N*N; i++) {
        arr[i] = i % 100;
    }
    
    /* Call test functions with varying parameters */
    printf("Running SIMT tests...\n");
    for (i = 0; i < 10; i++) {
        int result;
        int threshold = (i % 3) + 2;  /* Varying threshold */
        
        /* First test - may trigger SIMT with conditional offloading */
        simt_test(i, threshold, &result);
        results[i] = result;
        
        /* Second test - different pattern */
        if (i % 2 == 0) {
            results[i] += simt_test2(i, arr);
        }
        
        printf("Test %d: result = %d\n", i, results[i]);
    }
    
    /* Final verification sum */
    int total = 0;
    #pragma omp parallel for reduction(+:total)
    for (i = 0; i < 10; i++) {
        total += results[i];
    }
    
    printf("Total: %d\n", total);
    
    /* Additional test with dynamic adjustment */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:N*N]) if(total > 1000)
        for (i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int idx = i*N + j;
                /* Operation with multiple basic blocks */
                if (arr[idx] % 3 == 0) {
                    arr[idx] = arr[idx] / 3;
                } else if (arr[idx] % 3 == 1) {
                    arr[idx] = arr[idx] * 2;
                } else {
                    arr[idx] = arr[idx] + 1;
                }
            }
        }
    }
    
    return 0;
}
