/* sel-sched-trigger.c
 * Program designed to trigger selective scheduling RTL dumps in GCC
 * Specifically targets sel_print_insn_rtl function in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int trigger = 0;
volatile int sink = 0;

/* Deterministic pseudo-random generator */
static unsigned int simple_rand(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    int checksum = 0;
    unsigned int seed = 42;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Memory barrier to prevent reordering */
            __asm__ volatile("" : : : "memory");
        } else if (s < -1000) {
            s = 100;
            __asm__ volatile("" : : : "memory");
        } else {
            /* Another arithmetic operation */
            s = (s * 3) / 2;
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += trigger;  /* Use volatile */
                break;
            case 1:
                s -= simple_rand(&seed) % 10;
                break;
            case 2:
                s *= 2;
                break;
            default:
                s = s ^ (i & 0xFF);
                break;
        }
        
        /* Inner loop with more operations */
        for (int j = 0; j < M; ++j) {
            /* Variable index using volatile */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditional logic */
            if (c[idx] > 10000) {
                c[idx] = c[idx] % 1000;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Complex dependency chain */
            checksum += c[idx] * (i + 1) * (j + 1);
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return checksum;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int process_matrix(int *mat, int rows, int cols) {
    int total = 0;
    volatile int v = 0;
    
    for (int r = 0; r < rows; ++r) {
        int row_sum = 0;
        for (int c = 0; c < cols; ++c) {
            /* Complex index calculation */
            int idx = (r * cols + c + v) % (rows * cols);
            row_sum += mat[idx] * (r + c);
            
            /* Nested conditionals */
            if (row_sum > 5000) {
                row_sum = row_sum / 2;
                if (c % 3 == 0) {
                    mat[idx] = row_sum;
                }
            }
        }
        
        /* Use volatile in condition */
        if (row_sum > trigger) {
            total += row_sum;
        } else {
            total -= row_sum / 2;
        }
        
        __asm__ volatile("" : : : "memory");
    }
    
    return total;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int ROWS = 20;
    const int COLS = 15;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!a || !b || !c || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    unsigned int seed = 12345;
    for (int i = 0; i < N; i++) {
        a[i] = simple_rand(&seed) % 100;
        b[i] = simple_rand(&seed) % 100;
    }
    
    for (int i = 0; i < M; i++) {
        c[i] = simple_rand(&seed) % 100;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = simple_rand(&seed) % 200;
    }
    
    /* Modify trigger to affect scheduling */
    trigger = 5;
    
    /* Call functions with complex loops */
    int result1 = compute_checksum(a, b, c, N, M);
    int result2 = process_matrix(matrix, ROWS, COLS);
    
    /* Use results to prevent dead code elimination */
    sink = result1 + result2;
    
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Total: %d\n", sink);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
