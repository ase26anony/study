/* sel-sched-test.c
 * Test program to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test sel-sched-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Volatile variables to prevent optimization */
volatile int volatile_trigger = 0;
volatile int volatile_mod = 7;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    int i, j;
    
    /* Outer loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        int idx = (i * 3 + volatile_trigger) % N;
        s += a[idx] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = s / 2;
            /* Path A: more arithmetic */
            for (int k = 0; k < 3; ++k) {
                s += k * volatile_mod;
            }
        } else if (s > 500) {
            s = s - 250;
            /* Path B: different operations */
            for (int k = 0; k < 2; ++k) {
                s ^= (k << 3);
            }
        } else {
            s = s * 2 + 1;
            /* Path C: yet another path */
            s = s % 997;
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with dependencies */
        for (j = 0; j < M; ++j) {
            /* Variable index with volatile */
            int j_idx = (j + volatile_trigger) % M;
            c[j_idx] += s * j;
            
            /* More conditional logic */
            if (c[j_idx] > 10000) {
                c[j_idx] = c[j_idx] % 1000;
            } else if (c[j_idx] < -10000) {
                c[j_idx] = -c[j_idx] % 1000;
            }
            
            /* Switch statement for additional basic blocks */
            switch (j % 4) {
                case 0:
                    c[j_idx] += i;
                    break;
                case 1:
                    c[j_idx] -= i * 2;
                    break;
                case 2:
                    c[j_idx] ^= i;
                    break;
                case 3:
                    c[j_idx] = c[j_idx] * 3 / 2;
                    break;
            }
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Update volatile to prevent dead code elimination */
        volatile_trigger = (volatile_trigger + 1) % 13;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int process_matrix(int *matrix, int rows, int cols) {
    int sum = 0;
    volatile int vol_idx = 0;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = (r * cols + c + vol_idx) % (rows * cols);
            
            /* Complex addressing mode */
            sum += matrix[idx] * (r + 1) * (c + 1);
            
            /* Nested conditionals */
            if (sum & 1) {
                sum = sum ^ matrix[(idx + 1) % (rows * cols)];
                if (sum > 1000000) {
                    sum = sum >> 2;
                }
            } else {
                sum = sum + matrix[(idx + cols) % (rows * cols)];
                if (sum < -1000000) {
                    sum = -sum;
                }
            }
            
            /* Artificial dependency chain */
            for (int k = 0; k < 2; ++k) {
                sum = (sum << 3) | (sum >> 29); /* rotate */
                sum ^= (k * 0x5A827999);
            }
        }
        
        /* Update volatile every row */
        vol_idx = (vol_idx + r) % 11;
        __asm__ volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int MATRIX_SIZE = 20;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    int *matrix = (int *)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    if (!a || !b || !c || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; ++i) {
        a[i] = simple_rand() % 1000 - 500;
        b[i] = simple_rand() % 1000 - 500;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = simple_rand() % 1000 - 500;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i) {
        matrix[i] = simple_rand() % 2000 - 1000;
    }
    
    /* Call functions with complex loop nests */
    int checksum1 = compute_checksum(a, b, c, N, M);
    int checksum2 = process_matrix(matrix, MATRIX_SIZE, MATRIX_SIZE);
    
    /* Compute final result to prevent optimization */
    int final_result = checksum1 + checksum2;
    
    /* Also use array results */
    for (int i = 0; i < M; ++i) {
        final_result += c[i];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
