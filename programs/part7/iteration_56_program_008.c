/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000000) {
            s = 0;
            trigger = 1;  /* Use volatile variable */
        } else if (s < -1000000) {
            s = 0;
            trigger = -1; /* Use volatile variable */
        } else {
            /* Switch statement for additional basic blocks */
            switch (i % 4) {
                case 0:
                    s += i * 2;
                    break;
                case 1:
                    s -= i * 3;
                    break;
                case 2:
                    s ^= i;
                    break;
                default:
                    s = (s << 1) | (s >> 31);
                    break;
            }
        }
        
        /* Inner loop with array accesses */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile variable */
            int idx = (j + trigger) % M;
            if (idx < 0) idx = -idx;
            
            /* Multiple operations to increase register pressure */
            c[idx] += s * j + a[i] - b[i];
            c[(idx + 1) % M] ^= s;
            c[(idx + 2) % M] *= (s & 0xFF) + 1;
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Additional conditional inside outer loop */
        if (i % 7 == 0) {
            for (j = 0; j < 3; ++j) {
                s += c[j] * (i + j);
            }
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_matrix(int *mat, int rows, int cols) {
    volatile int guard = 0;
    int i, j, k;
    
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            
            /* Complex conditional chain */
            if (mat[idx] % 2 == 0) {
                mat[idx] = (mat[idx] >> 1) ^ 0x5A5A5A5A;
                guard = mat[idx];
            } else if (mat[idx] % 3 == 0) {
                mat[idx] = (mat[idx] * 3 + 1) & 0x7FFFFFFF;
                guard = mat[idx] + 1;
            } else {
                mat[idx] = mat[idx] * 5 - 7;
                guard = mat[idx] - 1;
            }
            
            /* Nested loop with variable bounds */
            for (k = 0; k < (j % 5) + 1; ++k) {
                mat[idx] += (k * guard) % 256;
            }
        }
        
        /* Memory barrier every few iterations */
        if (i % 4 == 0) {
            __asm__ volatile("" : : : "memory");
        }
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int ROWS = 20;
    const int COLS = 15;
    
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    int *matrix = malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
    }
    for (int i = 0; i < ROWS * COLS; ++i) {
        matrix[i] = rand() % 10000;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int result1 = compute_checksum(a, b, c, N, M);
    process_matrix(matrix, ROWS, COLS);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = result1;
    for (int i = 0; i < M; ++i) {
        final_sum += c[i];
    }
    for (int i = 0; i < ROWS * COLS; ++i) {
        final_sum ^= matrix[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
