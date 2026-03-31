/* test_sel_sched.c - Program to trigger selective scheduler debug output */
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
        int prod = a[i] * b[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (prod > THRESHOLD) {
            sum += prod;
            
            /* Inline assembly with multiple operands - creates complex RTL */
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
            sum += a[i];
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "r"(a[i])
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        b[i] = (b[i] * 3 + 1) & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
int matrix_multiply(int size) {
    int A[SIZE][SIZE];
    int B[SIZE][SIZE];
    int C[SIZE][SIZE] = {0};
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i][j] = (i + j) % 256;
            B[i][j] = (i * j) % 256;
        }
    }
    
    /* Nested loops with complex access patterns */
    for (int i = 0; i < size; i++) {
        for (int k = 0; k < size; k++) {
            int aik = A[i][k];
            
            /* Inner loop with carried dependency */
            for (int j = 0; j < size; j++) {
                C[i][j] += aik * B[k][j];
                
                /* Conditional with inline assembly */
                if (C[i][j] > 1000000) {
                    asm volatile (
                        "sar $2, %0\n\t"
                        "and $0xFFFF, %0"
                        : "+r"(C[i][j])
                        :
                        : "cc"
                    );
                }
            }
        }
    }
    
    /* Compute final sum */
    int total = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            total += C[i][j];
        }
    }
    
    return total;
}

/* Main function with multiple optimization targets */
int main() {
    int a[SIZE], b[SIZE];
    
    /* Initialize arrays with predictable but non-trivial patterns */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3 + 7) & 0xFF;
        b[i] = (i * 5 + 11) & 0xFF;
    }
    
    /* Call functions that should trigger selective scheduling */
    int result1 = compute_sum(a, b, SIZE);
    
    /* Use __builtin_assume to provide optimization hints */
    if (result1 > 1000000) {
        __builtin_assume(result1 < 2000000);
    }
    
    /* Smaller matrix multiplication for reasonable runtime */
    int result2 = matrix_multiply(16);
    
    /* Final computation mixing results */
    int final_result = result1 + result2 / 100;
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    printf("Result: %d\n", final_result);
    
    /* Return non-zero to ensure execution */
    return final_result != 0 ? 0 : 1;
}
