/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency and conditional control flow */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional creates multiple basic blocks for scheduler */
        if (temp > THRESHOLD) {
            /* Inline assembly creates complex RTL patterns */
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
                "andl $0x7FFFFFFF, %0"
                : "+r" (sum)
                : "r" (temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int idx = i & 0xFF;
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "addl %%eax, %0"
            : "+r" (sum)
            : "r" (idx)
            : "%eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
void process_matrix(int matrix[][16], int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < cols; j++) {
            int val = matrix[i][j];
            
            /* Complex conditional with multiple basic blocks */
            if (val > 0) {
                asm volatile (
                    "movl %1, %%ecx\n\t"
                    "shll $2, %%ecx\n\t"
                    "addl %%ecx, %0"
                    : "+r" (row_sum)
                    : "r" (val)
                    : "%ecx", "cc"
                );
            } else if (val < 0) {
                asm volatile (
                    "negl %1\n\t"
                    "addl %1, %0"
                    : "+r" (row_sum)
                    : "r" (val)
                    : "cc"
                );
            }
            
            /* Prevent dead code elimination */
            __builtin_assume(val != 0);
        }
        
        total += row_sum;
        
        /* Loop-carried dependency */
        asm volatile (
            "cmpl $1000, %0\n\t"
            "cmovg %1, %0"
            : "+r" (total)
            : "r" (row_sum)
            : "cc"
        );
    }
    
    /* Use result to prevent optimization */
    volatile int dummy = total;
    (void)dummy;
}

/* Main function with multiple hot loops */
int main() {
    int a[SIZE], b[SIZE];
    int matrix[64][16];
    
    /* Initialize arrays with predictable but non-trivial values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 17 + j * 13) % 200 - 100;
        }
    }
    
    /* Call functions to create scheduling regions */
    int result1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 64, 16);
    
    /* Additional loop with switch statement for control flow variety */
    int switch_sum = 0;
    for (int i = 0; i < 1000; i++) {
        switch (i % 4) {
            case 0:
                asm volatile ("addl $1, %0" : "+r" (switch_sum));
                break;
            case 1:
                asm volatile ("subl $1, %0" : "+r" (switch_sum));
                break;
            case 2:
                asm volatile ("addl $2, %0" : "+r" (switch_sum));
                break;
            case 3:
                asm volatile ("subl $2, %0" : "+r" (switch_sum));
                break;
        }
    }
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d\n", result1 + switch_sum);
    
    return 0;
}
