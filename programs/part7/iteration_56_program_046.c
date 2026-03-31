/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with register pressure and dependencies */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            /* Path A: Reset and do more computation */
            s = 0;
            for (j = 0; j < M; ++j) {
                c[j] += s * j;
                /* Inline assembly barrier to prevent optimization */
                __asm__ volatile("" : : : "memory");
            }
        } else if (s > 500) {
            /* Path B: Different computation pattern */
            s = s / 2;
            for (j = 0; j < M/2; ++j) {
                c[j] += s * (j + 1);
                __asm__ volatile("" : : : "memory");
            }
        } else {
            /* Path C: Yet another computation pattern */
            s = s * 2;
            for (j = M-1; j >= 0; --j) {
                c[j] += s * (M - j);
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += trigger * 2;
                break;
            case 1:
                s -= trigger;
                break;
            case 2:
                s ^= trigger;
                break;
            case 3:
                s = (s << 1) | (trigger & 1);
                break;
        }
        
        /* More arithmetic with variable indices */
        int idx = (s + i) % M;
        if (idx >= 0 && idx < M) {
            c[idx] += s;
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_matrix(int *mat, int rows, int cols) {
    volatile int v = 0;
    int sum = 0;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            mat[idx] = (mat[idx] * 3 + 7) % 256;
            
            /* Complex conditional with side effects */
            if ((r + c) % 3 == 0) {
                sum += mat[idx];
                v = sum;
            } else if ((r * c) % 5 == 0) {
                sum -= mat[idx];
                v = sum;
            } else {
                sum ^= mat[idx];
            }
            
            /* Nested loop with dependency */
            for (int k = 0; k < 3; ++k) {
                mat[idx] += (sum >> k) & 1;
                __asm__ volatile("" : : : "memory");
            }
        }
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int MAT_SIZE = 20;
    
    /* Initialize with deterministic pseudo-random values */
    srand(42);
    
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *matrix = (int*)malloc(MAT_SIZE * MAT_SIZE * sizeof(int));
    
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
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; ++i) {
        matrix[i] = rand() % 256;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int result1 = compute_checksum(a, b, c, N, M);
    process_matrix(matrix, MAT_SIZE, MAT_SIZE);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = result1;
    for (int i = 0; i < M; ++i) {
        final_sum += c[i];
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; ++i) {
        final_sum += matrix[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
