/* test_sel_sched.c - Test program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - encourages software pipelining */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int product = a[i] * b[i];
        
        /* Conditional control flow to create multi-block scheduling region */
        if (product > THRESHOLD) {
            sum += product;
            
            /* Inline assembly with multiple operands to create complex RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:\n\t"
                : "+r"(sum)
                : "r"(product)
                : "cc"
            );
        } else {
            /* Alternative path with different computation */
            sum += product / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "shrl $1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r"(sum)
                : "r"(product)
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase instruction count in loop */
        a[i] = (a[i] + b[i]) & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
void process_matrix(int mat[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access pattern */
        for (int j = 0; j < cols; j++) {
            int val = mat[i][j];
            
            /* Complex conditional with multiple branches */
            if (val > 0) {
                row_sum += val * 2;
                
                /* Inline assembly with memory operand */
                asm volatile (
                    "imull $2, %1\n\t"
                    "addl %1, %0\n\t"
                    : "+r"(row_sum)
                    : "r"(val)
                    : "cc"
                );
            } else if (val < 0) {
                row_sum -= val;
                
                /* Assembly with immediate and register */
                asm volatile (
                    "negl %1\n\t"
                    "addl %1, %0\n\t"
                    : "+r"(row_sum)
                    : "r"(val)
                    : "cc"
                );
            }
            
            /* Modify matrix element */
            mat[i][j] = row_sum % 256;
        }
        
        /* Store result with barrier */
        asm volatile ("" ::: "memory");
        mat[i][0] = row_sum;
    }
}

/* Helper function with switch statement for control flow variety */
int switch_compute(int x, int mode) {
    int result = 0;
    
    switch (mode) {
        case 0:
            result = x * 2;
            asm volatile ("addl $1, %0" : "+r"(result) :: "cc");
            break;
        case 1:
            result = x + x;
            asm volatile ("subl $1, %0" : "+r"(result) :: "cc");
            break;
        case 2:
            result = x << 1;
            asm volatile ("orl $1, %0" : "+r"(result) :: "cc");
            break;
        default:
            result = x;
            asm volatile ("xorl $0xFF, %0" : "+r"(result) :: "cc");
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) % 128 - 64;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    
    /* Use __builtin_assume to provide optimization hints */
    if (sum1 > 0) {
        __builtin_assume(sum1 > 0);
    }
    
    process_matrix(matrix, 8, 16);
    
    int sum2 = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            sum2 += matrix[i][j];
        }
    }
    
    /* Mix in switch-based computation */
    int sum3 = 0;
    for (int i = 0; i < 100; i++) {
        sum3 += switch_compute(i, i % 4);
    }
    
    /* Final result computation */
    int final_result = sum1 + sum2 + sum3;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Verify computation is correct */
    int expected = 0;
    for (int i = 0; i < SIZE; i++) {
        int product = a[i] * b[i];
        if (product > THRESHOLD) {
            expected += product;
        } else {
            expected += product / 2;
        }
    }
    
    printf("Expected base: %d\n", expected);
    
    return 0;
}
