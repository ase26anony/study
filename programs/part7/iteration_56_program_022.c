/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_trigger = 1;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M, int LIMIT) {
    int s = 0;
    int i, j;
    
    /* Complex loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Array accesses with variable indices */
        int idx = (i + g_volatile_counter) % N;
        s += a[idx] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: reset and do some extra computation */
            for (int k = 0; k < 3; ++k) {
                s += k * a[i % 10];
            }
        } else if (s < -LIMIT) {
            s = 1;
            /* Path B: different computation */
            for (int k = 0; k < 2; ++k) {
                s -= b[i % 5] * k;
            }
        } else {
            /* Path C: default path with switch statement */
            switch (i % 4) {
                case 0:
                    s += i * 2;
                    break;
                case 1:
                    s -= i / 2;
                    break;
                case 2:
                    s *= (i % 10) + 1;
                    break;
                default:
                    s = s / ((i % 5) + 1);
                    break;
            }
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with dependency on outer loop */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile */
            int j_idx = (j + g_volatile_trigger) % M;
            c[j_idx] += s * j;
            
            /* More conditional logic */
            if (c[j_idx] > 1000) {
                c[j_idx] %= 1000;
                /* Another memory barrier */
                __asm__ volatile("" : : : "memory");
            }
            
            /* Additional arithmetic to increase register pressure */
            c[j_idx] += (i * j) % 7;
        }
        
        /* Update volatile to prevent dead code elimination */
        g_volatile_counter += (i % 3);
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_matrix(int *mat, int rows, int cols) {
    volatile int local_volatile = 0;
    
    for (int r = 0; r < rows; ++r) {
        int row_sum = 0;
        for (int c = 0; c < cols; ++c) {
            /* Access with stride */
            int idx = r * cols + c;
            row_sum += mat[idx] * (r + c);
            
            /* Conditional store */
            if ((r + c) % 3 == 0) {
                mat[idx] = row_sum % 256;
            } else if ((r + c) % 3 == 1) {
                mat[idx] = (row_sum * 2) % 256;
            } else {
                mat[idx] = (row_sum / 2) % 256;
            }
            
            /* Use volatile in index calculation */
            mat[idx] += local_volatile % 10;
        }
        
        /* Cross-iteration dependency */
        local_volatile = row_sum % 100;
        __asm__ volatile("" : : : "memory");
    }
}

int main() {
    const int N = 100;
    const int M = 50;
    const int ROWS = 20;
    const int COLS = 15;
    const int LIMIT = 1000;
    
    /* Initialize with deterministic pseudo-random values */
    srand(42);
    
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!a || !b || !c || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = 0;
    }
    
    for (int i = 0; i < ROWS * COLS; ++i) {
        matrix[i] = rand() % 100;
    }
    
    /* Call the complex function */
    int checksum1 = compute_checksum(a, b, c, N, M, LIMIT);
    
    /* Process matrix with different pattern */
    process_matrix(matrix, ROWS, COLS);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = checksum1;
    for (int i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    
    for (int i = 0; i < ROWS * COLS; ++i) {
        final_checksum += matrix[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
