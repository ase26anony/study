/* test_sel_sched_dump.c
 * Designed to trigger selective scheduling RTL dumps in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 10000

/* Volatile variables to prevent optimization */
volatile int trigger = 0;
volatile int volatile_index = 0;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int *d) {
    int s = 0;
    int t = 0;
    int u = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Artificial dependency through inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Conditional with multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            t += i * 2;
        } else if (s < -LIMIT) {
            s = 1;
            t -= i * 3;
        } else {
            t += s;
        }
        
        /* Use volatile variable in index */
        int idx = (i + trigger) % M;
        
        /* Inner loop with more operations */
        for (int j = 0; j < M; ++j) {
            /* Complex dependency chain */
            u = c[j] + s * j;
            d[j] += u;
            
            /* Another conditional inside inner loop */
            if (j % 4 == 0) {
                c[j] = u / 2;
            } else if (j % 4 == 1) {
                c[j] = u * 2;
            } else if (j % 4 == 2) {
                c[j] = u + 5;
            } else {
                c[j] = u - 3;
            }
            
            /* Switch statement for more basic blocks */
            switch (j % 3) {
                case 0:
                    d[j] += 1;
                    break;
                case 1:
                    d[j] += 2;
                    break;
                case 2:
                    d[j] += 3;
                    break;
            }
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Complex expression with volatile */
        volatile_index = (volatile_index + 1) % 16;
        s += d[volatile_index];
    }
    
    return s + t + u;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int process_matrix(int *mat1, int *mat2, int rows, int cols) {
    int sum = 0;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            
            /* Complex conditional with early exit */
            if (mat1[idx] > 0) {
                mat2[idx] = mat1[idx] * 3;
                sum += mat2[idx];
                
                /* Nested loop with variable bound */
                for (int k = 0; k < (c % 8); ++k) {
                    mat2[idx] -= k;
                    sum += mat1[idx] * k;
                }
            } else {
                mat2[idx] = -mat1[idx] * 2;
                sum -= mat2[idx];
            }
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize with deterministic pseudo-random values */
    srand(42);
    
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    int *d = malloc(M * sizeof(int));
    int *mat1 = malloc(N * M * sizeof(int));
    int *mat2 = malloc(N * M * sizeof(int));
    
    if (!a || !b || !c || !d || !mat1 || !mat2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000 - 500;
        b[i] = rand() % 1000 - 500;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
        d[i] = rand() % 1000;
    }
    
    for (int i = 0; i < N * M; ++i) {
        mat1[i] = rand() % 2000 - 1000;
        mat2[i] = 0;
    }
    
    /* Call functions to trigger selective scheduling */
    int result1 = compute_checksum(a, b, c, d);
    int result2 = process_matrix(mat1, mat2, 16, 16);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = result1 + result2;
    
    /* Use results to prevent optimization */
    for (int i = 0; i < M; ++i) {
        final_checksum += d[i];
    }
    
    for (int i = 0; i < N * M; ++i) {
        final_checksum += mat2[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(mat1);
    free(mat2);
    
    return 0;
}
