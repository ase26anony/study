/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;
    int s = 0;
    int result = 0;
    
    /* Complex loop nest with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional operations in this path */
            result += i * 2;
        } else if (s > 500) {
            s = s / 2;
            result += i;
        } else {
            /* Third path with different operations */
            s = s * 3;
            result += s % 17;
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with variable bounds */
        int limit = M - (i % 5);
        for (int j = 0; j < limit; ++j) {
            /* Complex addressing with multiple operations */
            c[j] += s * j + a[i] - b[i];
            
            /* Nested conditional */
            if (j % 3 == 0) {
                c[j] = c[j] * 2;
            } else if (j % 3 == 1) {
                c[j] = c[j] / 2;
            } else {
                c[j] = c[j] + trigger;
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
            case 3:
                s = s ^ 0x0F;
                break;
        }
    }
    
    return result + s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static int process_matrix(int *matrix, int size) {
    volatile int seed = 42;
    int sum = 0;
    
    /* Matrix processing with complex dependencies */
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int idx = i * size + j;
            
            /* Multiple conditional paths */
            if ((i + j) % 2 == 0) {
                matrix[idx] = matrix[idx] * seed + i;
            } else {
                matrix[idx] = matrix[idx] / (seed % 7 + 1) - j;
            }
            
            /* Cross-iteration dependency */
            if (i > 0 && j > 0) {
                matrix[idx] += matrix[(i-1) * size + (j-1)];
            }
            
            sum += matrix[idx];
            
            /* Memory barrier */
            __asm__ volatile("" : : : "memory");
        }
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int MATRIX_SIZE = 20;
    
    /* Initialize with deterministic pseudo-random values */
    srand(12345);
    
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    int *matrix = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
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
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i) {
        matrix[i] = rand() % 100;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, N, M);
    int checksum2 = process_matrix(matrix, MATRIX_SIZE);
    
    /* Final computation to prevent dead code elimination */
    int final_result = checksum1 + checksum2;
    
    /* Use results to ensure they're not optimized away */
    for (int i = 0; i < M; ++i) {
        final_result += c[i] % 256;
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(matrix);
    
    return 0;
}
