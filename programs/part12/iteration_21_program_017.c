/* Test program to trigger sel_print_insn with RTL dump switching */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex inline assembly to generate non-trivial RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n\t"
                "1:"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull $3, %1, %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "r"(temp)
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int idx = (i * 17) % n;
        asm volatile (
            "movl %1, %%ecx\n\t"
            "addl %%ecx, %0"
            : "+r"(sum)
            : "r"(a[idx])
            : "%ecx", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
void process_matrix(int matrix[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        /* Inner loop with software pipelining opportunities */
        for (int j = 0; j < cols; j++) {
            int val = matrix[i][j];
            
            /* Conditional with multiple basic blocks */
            if (val & 1) {
                /* Odd value processing */
                asm volatile (
                    "leal (%1,%1,2), %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(row_sum)
                    : "r"(val)
                    : "%eax"
                );
            } else {
                /* Even value processing */
                asm volatile (
                    "shrl $1, %1\n\t"
                    "addl %1, %0"
                    : "+r"(row_sum)
                    : "r"(val)
                    : "cc"
                );
            }
            
            /* Prevent dead code elimination */
            __asm__ __volatile__("" : : "r"(row_sum) : "memory");
        }
        
        /* Store result back */
        matrix[i][0] = row_sum;
    }
}

/* Helper to initialize arrays */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 128;
    }
}

int main() {
    /* Use __builtin_assume to provide loop bound hints to scheduler */
    int n = SIZE;
    __builtin_assume(n > 0 && n <= SIZE);
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, SIZE);
    
    /* Call computation function - this should trigger selective scheduling */
    int result = compute_sum(a, b, n);
    
    /* Process matrix for additional scheduling regions */
    int matrix[8][16];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 3;
        }
    }
    
    process_matrix(matrix, 8, 16);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    /* Verify computation */
    int verify = 0;
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            verify += temp;
            if (verify > 1000) verify -= 500;
        } else {
            verify += temp * 3;
        }
        verify += a[(i * 17) % n];
    }
    
    if (result != verify) {
        fprintf(stderr, "Verification failed: %d != %d\n", result, verify);
        free(a);
        free(b);
        return 1;
    }
    
    free(a);
    free(b);
    return 0;
}
