/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_limit = 1000;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold)) 
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    volatile int trigger = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Artificial dependency barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Conditional with multiple basic blocks */
        if (s > g_volatile_limit) {
            s = 0;
            trigger = i;  /* Use volatile */
        } else if (s < -g_volatile_limit) {
            s = g_volatile_limit / 2;
            trigger = i * 2;
        } else {
            /* Switch statement for more basic blocks */
            switch (i % 4) {
                case 0:
                    s += i;
                    break;
                case 1:
                    s -= i * 2;
                    break;
                case 2:
                    s *= (i % 10) + 1;
                    break;
                case 3:
                    s = s / ((i % 5) + 1);
                    break;
            }
        }
        
        /* Inner loop with more operations */
        for (int j = 0; j < M; ++j) {
            c[j] += s * j;
            
            /* More conditionals inside inner loop */
            if ((i + j) % 3 == 0) {
                c[j] -= a[i];
            } else if ((i + j) % 3 == 1) {
                c[j] += b[i % N];
            }
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Update volatile counter */
        g_volatile_counter++;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_matrix(int *mat, int rows, int cols) {
    volatile int row_sum = 0;
    
    for (int r = 0; r < rows; ++r) {
        int col_sum = 0;
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            col_sum += mat[idx] * (r + c);
            
            /* Complex conditional */
            if (col_sum > 100) {
                mat[idx] = col_sum % 256;
                col_sum = 0;
            } else if (col_sum < -50) {
                mat[idx] = (-col_sum) % 256;
                col_sum = col_sum / 2;
            }
            
            /* Use volatile in computation */
            mat[idx] += g_volatile_counter % 10;
        }
        
        row_sum += col_sum;
        
        /* Switch with multiple cases */
        switch (r % 5) {
            case 0: mat[r] = row_sum; break;
            case 1: mat[r] = row_sum * 2; break;
            case 2: mat[r] = row_sum - r; break;
            case 3: mat[r] = row_sum / ((r % 3) + 1); break;
            case 4: mat[r] = row_sum ^ r; break;
        }
    }
}

int main(void) {
    const int N = 128;
    const int M = 64;
    const int MAT_SIZE = 32;
    
    /* Initialize with deterministic pseudo-random values */
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    int *matrix = malloc(MAT_SIZE * MAT_SIZE * sizeof(int));
    
    srand(42);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        c[i] = rand() % 50;
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        matrix[i] = rand() % 200 - 100;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_matrix(matrix, MAT_SIZE, MAT_SIZE);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_sum = checksum1;
    for (int i = 0; i < M; i++) {
        final_sum += c[i];
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i += 16) {
        final_sum += matrix[i];
    }
    
    printf("Result: %d (Volatile counter: %d)\n", final_sum, g_volatile_counter);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
