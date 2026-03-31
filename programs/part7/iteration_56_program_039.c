/* sel_sched_dump_test.c
 * 
 * This program is designed to trigger GCC's selective scheduling
 * RTL dump functionality, specifically targeting the uncovered
 * lines in sel-sched-dump.cc that switch dump streams and print
 * instruction RTL.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and hint as cold path */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Prevent optimization */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            /* Path A: Reset with complex operation */
            s = (s % 256) * 2;
            __asm__ volatile("" : : : "memory");  /* Memory barrier */
        } else if (s > 500) {
            /* Path B: Different transformation */
            s = (s << 1) | 1;
            __asm__ volatile("" : : : "memory");
        } else {
            /* Path C: Yet another operation */
            s = s ^ (i * 3);
        }
        
        /* Inner loop with variable bounds */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile influence */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* Another conditional inside inner loop */
            if (c[idx] > 10000) {
                c[idx] = c[idx] % 1000;
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += 1;
                break;
            case 1:
                s -= 2;
                break;
            case 2:
                s *= 3;
                break;
            default:
                s /= 2;
                break;
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline))
static void process_matrix(int *mat, int rows, int cols) {
    volatile int seed = 7;
    int i, j, k;
    
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            
            /* Nested conditionals */
            if (mat[idx] % 2 == 0) {
                for (k = 0; k < 3; ++k) {
                    mat[idx] += (seed * k) % 17;
                    __asm__ volatile("" : : : "memory");
                }
            } else {
                mat[idx] -= (seed * j) % 13;
            }
            
            /* More arithmetic with volatile dependency */
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            mat[idx] ^= seed;
        }
    }
}

int main(void) {
    const int N = 100;  /* Runtime constants > 10 */
    const int M = 50;
    const int ROWS = 20;
    const int COLS = 15;
    
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    int *matrix = malloc(ROWS * COLS * sizeof(int));
    
    if (!a || !b || !c || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < ROWS * COLS; ++i) {
        matrix[i] = rand() % 1000;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_matrix(matrix, ROWS, COLS);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = checksum1;
    for (int i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    for (int i = 0; i < ROWS * COLS; ++i) {
        final_checksum += matrix[i] % 256;
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
