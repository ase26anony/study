/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower simt_test.c -o simt_test */

#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent inlining to preserve call structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold, int iter) {
    volatile int size = TOTAL; /* volatile to prevent optimization */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) shared(a, b, c, size)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a[0:size], b[0:size], c[0:size]) \
                reduction(+:local_sum)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + (iter % 10);
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 150 && j > 30) {
                    /* Early exit creates additional control flow */
                    c[idx] = 150; /* Cap value */
                }
                
                /* Complex operation to prevent simplification */
                if ((i * j) % 7 == 0) {
                    c[idx] += (n % 5);
                }
                
                local_sum += c[idx];
            }
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    /* Additional computation to use results */
    int final_sum = 0;
    for (i = 0; i < TOTAL; i++) {
        final_sum += c[i] % 100;
    }
    
    return final_sum + sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, noipa))
int simt_test2(int n, int flag) {
    volatile int dim = 100;
    int x[100][100], y[100][100], z[100][100];
    int i, j, result = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            x[i][j] = i + j + n;
            y[i][j] = i * j - n;
        }
    }
    
    /* Different nesting: parallel region inside target */
    #pragma omp target if(flag) map(tofrom: x, y, z) device(0)
    {
        #pragma omp teams distribute parallel for simd \
                collapse(2) num_teams(4) thread_limit(128)
        for (i = 0; i < dim; i++) {
            for (j = 0; j < dim; j++) {
                z[i][j] = x[i][j] * y[i][j];
                
                /* Multiple conditions for control flow */
                if (z[i][j] < 0) {
                    z[i][j] = -z[i][j];
                    if (i > j) {
                        z[i][j] += 1000;
                    }
                } else if (z[i][j] > 10000) {
                    z[i][j] = 10000;
                }
                
                /* Early return simulation */
                if (z[i][j] == 777 && i == 50) {
                    z[i][j] = 0; /* Simulate special case */
                }
            }
        }
    }
    
    /* Use results */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            result += z[i][j] % 1000;
        }
    }
    
    return result;
}

int main() {
    int total = 0;
    
    /* Varying parameters to prevent constant propagation */
    for (int iter = 0; iter < 5; iter++) {
        int threshold = 3 + (iter % 3);
        int n = 5 + (iter % 7);
        
        /* Call with different conditions */
        int res1 = simt_test(n, threshold, iter);
        int res2 = simt_test2(n, iter % 2);
        
        total += res1 + res2;
        
        printf("Iteration %d: res1=%d, res2=%d\n", iter, res1, res2);
    }
    
    printf("Total: %d\n", total);
    
    /* Additional test with large collapse */
    #pragma omp target teams distribute parallel for simd \
            collapse(3) map(tofrom: total)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                total += i * j * k;
            }
        }
    }
    
    printf("Final total: %d\n", total);
    
    return 0;
}
