/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Volatile to prevent elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Multiple arithmetic operations with dependencies */
        int idx = (i + trigger) % N;  /* Volatile influences index */
        s += a[idx] * b[idx];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional operations in this path */
            a[idx] = (a[idx] * 3) / 2;
        } else if (s < -500) {
            s = 100;
            b[idx] = b[idx] + i;
        } else {
            /* Third path with different operations */
            s = s * 2 - 1;
            __asm__ volatile("" : : : "memory");  /* Memory barrier */
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with multiple dependencies */
            int jdx = (j + i) % M;
            c[jdx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[jdx] > 10000) {
                c[jdx] = c[jdx] % 1000;
                __asm__ volatile("" : : : "memory");  /* Another barrier */
            }
            
            /* Additional arithmetic to increase pressure */
            c[jdx] = c[jdx] ^ (s & 0xFF);
        }
        
        /* Switch statement for more basic blocks */
        switch (i % 4) {
            case 0:
                s += a[idx] >> 1;
                break;
            case 1:
                s -= b[idx] << 1;
                break;
            case 2:
                s ^= (a[idx] + b[idx]);
                break;
            case 3:
                s = (s * 3) / 2;
                __asm__ volatile("" : : : "memory");
                break;
        }
        
        /* Use volatile in condition */
        if (trigger++ > 50) {
            trigger = 0;
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_matrix(int *mat, int rows, int cols) {
    volatile int v = 1;
    int i, j, k;
    
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            int temp = 0;
            
            /* Nested loops with complex addressing */
            for (k = 0; k < 8; ++k) {
                temp += mat[(idx + k) % (rows * cols)] * k;
                
                /* Conditional with side effects */
                if (temp & 1) {
                    mat[idx] ^= temp;
                    __asm__ volatile("" : : : "memory");
                } else {
                    mat[idx] += temp;
                }
            }
            
            /* More arithmetic operations */
            mat[idx] = (mat[idx] * v) % 10007;
            v = (v * 13) % 17;  /* Volatile-like behavior */
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
    srand(42);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_matrix(matrix, ROWS, COLS);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = checksum1;
    for (int i = 0; i < N; i++) {
        final_checksum += a[i] + b[i];
    }
    for (int i = 0; i < M; i++) {
        final_checksum += c[i];
    }
    for (int i = 0; i < ROWS * COLS; i++) {
        final_checksum += matrix[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
