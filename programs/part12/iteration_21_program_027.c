/* test_sel_sched.c - Test program to trigger sel_print_insn debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern with inline assembly */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "subl %1, %0\n\t"
                "testl %0, %0\n\t"
                "jns 1f\n\t"
                "negl %0\n"
                "1:"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int extra = (i & 0xF) + 1;
        asm volatile (
            "imull %1, %0"
            : "+r"(extra)
            : "r"(i)
            : "cc"
        );
        
        if (extra > 50) {
            sum += extra / 2;
        }
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
int matrix_multiply(int *mat1, int *mat2, int *result, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                /* Complex RTL pattern with memory operands */
                int val1, val2, prod;
                val1 = mat1[i * n + k];
                val2 = mat2[k * n + j];
                
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %2, %%eax\n\t"
                    "addl %%eax, %0\n\t"
                    "addl %%eax, %3"
                    : "+r"(result[i * n + j]), "=r"(prod)
                    : "r"(val1), "r"(val2)
                    : "%eax", "cc"
                );
                
                total += prod;
            }
        }
    }
    
    return total;
}

/* Function with switch statement for control flow variety */
int process_with_switch(int *data, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        switch (data[i] % 4) {
            case 0:
                asm volatile ("addl $1, %0" : "+r"(result) :: "cc");
                break;
            case 1:
                asm volatile ("subl $1, %0" : "+r"(result) :: "cc");
                break;
            case 2:
                asm volatile ("imull $2, %0" : "+r"(result) :: "cc");
                break;
            case 3:
                asm volatile ("andl $0xFF, %0" : "+r"(result) :: "cc");
                break;
        }
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int result[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    
    /* Create smaller matrix for multiplication */
    int mat1[16], mat2[16], mat_result[16];
    for (int i = 0; i < 16; i++) {
        mat1[i] = i;
        mat2[i] = 16 - i;
    }
    
    int sum2 = matrix_multiply(mat1, mat2, mat_result, 4);
    
    /* Process with switch */
    int sum3 = process_with_switch(a, SIZE);
    
    /* Final computation to prevent dead code elimination */
    int final_result = sum1 + sum2 + sum3;
    
    /* Use __builtin_assume to provide optimization hints */
    if (final_result > 0) {
        __builtin_assume(final_result > 0);
    }
    
    printf("Result: %d\n", final_result);
    
    /* Verify computation is correct */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += a[i] * b[i];
    }
    printf("Verification sum: %d\n", verify);
    
    return (final_result > 0) ? 0 : 1;
}
