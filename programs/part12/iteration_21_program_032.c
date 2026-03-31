/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop and control flow */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int prod = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (prod > THRESHOLD) {
            /* Complex inline asm to generate non-trivial RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:"
                : "+r"(sum)
                : "r"(prod)
                : "cc"
            );
        } else {
            /* Another asm with different pattern */
            asm volatile (
                "imull $2, %0\n\t"
                "addl %1, %0"
                : "+r"(sum)
                : "r"(prod)
                : "cc"
            );
        }
        
        /* Additional asm to create more scheduling opportunities */
        asm volatile (
            "movl %0, %%eax\n\t"
            "shrl $1, %%eax\n\t"
            "addl %%eax, %0"
            : "+r"(sum)
            :
            : "eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
void process_matrix(int mat[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (int j = 0; j < cols; j += 2) {
            int val = mat[i][j];
            
            /* Conditional with multiple basic blocks */
            if (val & 1) {
                /* Asm with memory operand */
                asm volatile (
                    "leal (%1,%1,2), %0"
                    : "=r"(val)
                    : "r"(val)
                );
            }
            
            row_sum += val;
            
            /* Prevent dead code elimination */
            asm volatile ("" : : "r"(row_sum));
        }
        
        /* Store result back */
        mat[i][0] = row_sum;
    }
}

/* Main function with multiple hot loops */
int main(void) {
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 5;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 8, 16);
    
    /* Compute verification sum to prevent optimization */
    int verify_sum = 0;
    for (int i = 0; i < 8; i++) {
        verify_sum += matrix[i][0];
    }
    
    /* Final result that depends on all computations */
    int final_result = sum1 + verify_sum;
    
    /* Use __builtin_assume to provide optimization hints */
    if (final_result > 0) {
        __builtin_assume(final_result > 0);
    }
    
    printf("Result: %d\n", final_result);
    
    /* Return non-zero for verification */
    return (final_result > 0) ? 0 : 1;
}
