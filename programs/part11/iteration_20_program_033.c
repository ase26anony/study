/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization of helper functions */
__attribute__((noinline, used))
int simt_test(int n, int threshold, int iter) {
    volatile int size = SIZE;  /* volatile to prevent optimization */
    int a[SIZE], b[SIZE], c[SIZE];
    int i, j, idx;
    int sum = 0;
    
    /* Initialize arrays with pattern based on iteration */
    for (i = 0; i < SIZE; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j, idx)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:SIZE], b[0:SIZE]) \
                map(from: c[0:SIZE]) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < n; i++) {
            for (j = 0; j < M; j++) {
                idx = i * M + j;
                if (idx < SIZE) {
                    c[idx] = a[idx] + b[idx] + (i * j);
                    
                    /* Create internal basic block split */
                    if (c[idx] > 150 && j % 7 == 0) {
                        /* Dummy operation that can't be optimized away */
                        c[idx] = c[idx] % 100;
                        if (c[idx] < 0) {
                            /* Another basic block split */
                            c[idx] = -c[idx];
                        }
                    }
                    
                    /* Early exit condition to force multiple labels */
                    if (c[idx] > 200 && i > n/2) {
                        /* This creates additional control flow */
                        c[idx] = 100;
                    }
                }
            }
        }
        
        /* Additional parallel for to maintain context */
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int result = sum;
    return result;
}

/* Another test function with different structure */
__attribute__((noinline, used))
int simt_test2(int n, int flag) {
    int x[N][M], y[N][M], z[N][M];
    volatile int bound = n;
    int i, j;
    int total = 0;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            x[i][j] = i + j;
            y[i][j] = i * j;
            z[i][j] = 0;
        }
    }
    
    #pragma omp parallel
    {
        /* Nested target with teams and distribute */
        #pragma omp target teams distribute parallel for simd \
                if(flag) map(to: x, y) map(from: z) \
                collapse(2) num_teams(4)
        for (i = 0; i < bound && i < N; i++) {
            for (j = 0; j < M; j++) {
                z[i][j] = x[i][j] * 2 + y[i][j];
                
                /* Complex conditional to encourage SIMT transformation */
                if (z[i][j] > 50) {
                    z[i][j] = z[i][j] / 2;
                    if (i % 3 == 0) {
                        z[i][j] += j;
                    }
                } else if (z[i][j] < 10) {
                    z[i][j] = z[i][j] * 3;
                }
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for reduction(+:total) collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                total += z[i][j];
            }
        }
    }
    
    return total;
}

int main() {
    int i;
    int results[10];
    int total_sum = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying parameters to prevent constant propagation */
    for (i = 0; i < 10; i++) {
        int threshold = 5;
        int n = 8 + (i % 3);  /* Vary between 8, 9, 10 */
        int flag = i % 2;     /* Alternate between 0 and 1 */
        
        /* Call first test function */
        results[i] = simt_test(n, threshold, i);
        total_sum += results[i];
        
        /* Call second test function with different pattern */
        if (i % 2 == 0) {
            results[i] += simt_test2(n, flag);
        }
        
        printf("Iteration %d: result = %d\n", i, results[i]);
    }
    
    /* Use results to prevent optimization */
    volatile int final_result = total_sum;
    printf("Total sum: %d\n", final_result);
    
    /* Additional test with dynamic allocation */
    #pragma omp parallel
    {
        int *dyn_a, *dyn_b, *dyn_c;
        #pragma omp single
        {
            dyn_a = (int*)malloc(SIZE * sizeof(int));
            dyn_b = (int*)malloc(SIZE * sizeof(int));
            dyn_c = (int*)malloc(SIZE * sizeof(int));
        }
        
        #pragma omp for simd
        for (i = 0; i < SIZE; i++) {
            dyn_a[i] = i;
            dyn_b[i] = i * 2;
        }
        
        /* Target with mapped pointers */
        #pragma omp target teams distribute parallel for simd \
                map(to: dyn_a[0:SIZE], dyn_b[0:SIZE]) \
                map(from: dyn_c[0:SIZE]) \
                if(1)  /* Always true condition */
        for (i = 0; i < SIZE; i++) {
            dyn_c[i] = dyn_a[i] + dyn_b[i];
            if (dyn_c[i] > 100) {
                dyn_c[i] = 100;
            }
        }
        
        #pragma omp single
        {
            free(dyn_a);
            free(dyn_b);
            free(dyn_c);
        }
    }
    
    return 0;
}
