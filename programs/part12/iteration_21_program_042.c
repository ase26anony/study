/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multi-block region */
        if (temp > THRESHOLD) {
            /* Complex inline assembly to generate non-trivial RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "adcl $0, %0"
                : "+r" (sum)
                : "r" (temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "subl %1, %0\n\t"
                "cmpl $0, %0"
                : "+r" (sum)
                : "r" (temp)
                : "cc", "eax"
            );
        }
        
        /* Additional arithmetic to create more scheduling opportunities */
        sum = (sum * 3) / 2;
    }
    
    return sum;
}

/* Second function with nested loops for more complex scheduling */
void process_matrix(int mat[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with pointer arithmetic */
        for (int j = 0; j < cols; j++) {
            /* Use __builtin_assume to help compiler know loop bounds */
            if (j < cols) __builtin_assume(j >= 0);
            
            /* Mixed operations to create varied RTL patterns */
            int val = mat[i][j];
            val = (val << 3) | (val >> 5);  /* Rotate */
            
            /* Conditional with multiple basic blocks */
            if (val & 1) {
                row_sum += val * 7;
            } else {
                row_sum -= val / 3;
            }
            
            /* Memory barrier asm to create scheduling boundaries */
            asm volatile ("" ::: "memory");
        }
        
        /* Store result with volatile to prevent elimination */
        *(volatile int*)&mat[i][0] = row_sum;
    }
}

/* Main function with multiple hot loops */
int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 11;
        }
    }
    
    /* Call functions to create scheduling regions */
    int result1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 8, 16);
    
    /* Compute verification value */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify ^= a[i] ^ b[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (verification: %d)\n", result1, verify);
    
    /* Additional loop with switch to create more control flow */
    int switch_sum = 0;
    for (int i = 0; i < 1000; i++) {
        switch (i % 4) {
            case 0: switch_sum += i * 2; break;
            case 1: switch_sum -= i * 3; break;
            case 2: switch_sum |= i; break;
            case 3: switch_sum &= ~i; break;
        }
    }
    printf("Switch sum: %d\n", switch_sum);
    
    return 0;
}
