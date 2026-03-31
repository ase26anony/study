#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_counter = 0;
static volatile int s_volatile_threshold = 1000;

/* Helper function marked noinline to preserve structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold) {
    /* Use volatile variables to prevent optimization */
    volatile int i, j;
    int result = 0;
    
    /* Arrays with volatile elements to prevent dead code elimination */
    int a[SIZE], b[SIZE], c[SIZE];
    
    /* Initialize arrays with non-constant values */
    for (int idx = 0; idx < SIZE; idx++) {
        a[idx] = (idx * 3) % 7;
        b[idx] = (idx * 5) % 11;
        c[idx] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:result)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) map(tofrom: a, b, c) \
                if(n > threshold) num_teams(2) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Simple arithmetic operation */
                c[idx] = a[idx] + b[idx] + thread_id;
                
                /* Conditional to create multiple basic blocks */
                if (c[idx] > (threshold * 2)) {
                    /* Early exit-like construct to force label generation */
                    if (c[idx] > 1000 && n < 5) {
                        /* This creates additional control flow */
                        c[idx] = c[idx] % 100;
                    } else {
                        /* Alternative path */
                        c[idx] = c[idx] + 1;
                    }
                } else {
                    /* Another path for the else case */
                    c[idx] = c[idx] - 1;
                }
                
                /* Force side effect to prevent dead code elimination */
                if (idx % 13 == 0) {
                    /* This creates more complex control flow */
                    #pragma omp atomic
                    g_volatile_counter++;
                }
            }
        }
        
        /* Process results after target region */
        #pragma omp for collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                result += c[idx];
                
                /* Additional conditional to maintain complexity */
                if (result > 1000000) {
                    result = result % 1000000;
                }
            }
        }
    }
    
    return result;
}

/* Another test function with different structure */
__attribute__((noinline, noipa))
int simt_test2(int n, int *output) {
    volatile int x, y;
    int arr1[N][M], arr2[N][M], arr3[N][M];
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i * j + n;
            arr2[i][j] = i + j * n;
            arr3[i][j] = 0;
        }
    }
    
    /* Nested OpenMP regions */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Target with teams and distribute */
            #pragma omp target teams distribute parallel for simd \
                    collapse(2) map(tofrom: arr1, arr2, arr3) \
                    if(n > 2) num_teams(4)
            for (x = 0; x < N; x++) {
                for (y = 0; y < M; y++) {
                    /* Complex expression with conditional */
                    int val = arr1[x][y] * arr2[x][y];
                    
                    if (val % 3 == 0) {
                        arr3[x][y] = val / 3;
                        /* Early return simulation */
                        if (val > 1000 && x > N/2) {
                            arr3[x][y] = 1;
                        }
                    } else if (val % 5 == 0) {
                        arr3[x][y] = val / 5;
                    } else {
                        arr3[x][y] = val;
                    }
                    
                    /* Force multiple basic blocks */
                    switch (val % 4) {
                        case 0: arr3[x][y] += 1; break;
                        case 1: arr3[x][y] += 2; break;
                        case 2: arr3[x][y] += 3; break;
                        default: arr3[x][y] += 4; break;
                    }
                }
            }
        }
        
        /* Reduction after target */
        #pragma omp for collapse(2) reduction(+:sum)
        for (x = 0; x < N; x++) {
            for (y = 0; y < M; y++) {
                sum += arr3[x][y];
            }
        }
    }
    
    *output = sum;
    return sum;
}

int main() {
    int total = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (int iter = 1; iter <= 10; iter++) {
        int threshold = iter * 100;
        
        /* Call test function with different parameters */
        int result1 = simt_test(iter, threshold);
        printf("Iteration %d, test1 result: %d\n", iter, result1);
        total += result1;
        
        int result2;
        int result3 = simt_test2(iter + 3, &result2);
        printf("Iteration %d, test2 results: %d, %d\n", iter, result2, result3);
        total += result2 + result3;
        
        /* Use volatile to prevent dead code elimination */
        s_volatile_threshold = threshold;
    }
    
    printf("Total sum: %d\n", total);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return total > 0 ? 0 : 1;
}
