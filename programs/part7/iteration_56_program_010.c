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
            s = 100;
            trigger = 2;
        } else {
            /* Nested switch statement for more basic blocks */
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
        
        /* Inner loop with variable bounds */
        for (j = 0; j < M; ++j) {
            /* Complex array access with volatile index influence */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More arithmetic to increase register pressure */
            c[idx] ^= (i * j) & 0xFF;
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Additional conditional inside outer loop */
        if (i % 7 == 0) {
            s = (s * 3) / 2;
        } else if (i % 7 == 3) {
            s = s ^ (s >> 16);
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_matrix(int *mat, int rows, int cols) {
    volatile int seed = 42;
    int i, j, k;
    
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            
            /* Complex computation with multiple dependencies */
            mat[idx] = (mat[idx] * seed) + (i * j);
            
            /* Triple nested loop for more scheduling complexity */
            for (k = 0; k < 5; ++k) {
                mat[idx] += (k * seed) ^ (i + j);
                __asm__ volatile("" : : : "memory");
            }
            
            /* Conditional with side effect */
            if (mat[idx] > 1000) {
                mat[idx] %= 1000;
                seed = (seed * 13 + 17) & 0xFF;
            }
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
    
    /* Initialize with pseudo-random values */
    srand(12345);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = 0;
    }
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = rand() % 500;
    }
    
    /* Call functions that should trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_matrix(matrix, ROWS, COLS);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = checksum1;
    for (int i = 0; i < M; i++) {
        final_checksum += c[i];
    }
    for (int i = 0; i < ROWS * COLS; i++) {
        final_checksum ^= matrix[i];
    }
    
    printf("Result: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
