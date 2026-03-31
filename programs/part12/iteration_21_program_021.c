/* test_sel_sched.c - Test program to trigger sel_print_insn coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that should trigger selective scheduling */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency - creates scheduling region */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional control flow inside loop - creates multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex inline assembly to generate non-trivial RTL patterns */
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
                "sbbl $0, %0"
                : "+r" (sum)
                : "r" (temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase instruction count in loop */
        int extra = a[i] ^ b[i];
        asm volatile (
            "xorl %1, %0"
            : "+r" (extra)
            : "r" (i)
        );
    }
    
    return sum;
}

/* Second function with nested loops for more complex scheduling */
int matrix_multiply(int size) {
    int result = 0;
    
    /* Nested loops create larger scheduling regions */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int val = i * j;
            
            /* Multiple conditions to create control flow */
            if (val & 1) {
                asm volatile (
                    "rorl $1, %0"
                    : "+r" (val)
                    :: "cc"
                );
                result += val;
            } else if (val & 2) {
                asm volatile (
                    "roll $2, %0"
                    : "+r" (val)
                    :: "cc"
                );
                result -= val;
            } else {
                result ^= val;
            }
        }
    }
    
    return result;
}

/* Third function with loop-carried dependency */
int fibonacci_style(int n) {
    int a = 1, b = 1;
    
    /* Strong loop-carried dependency */
    for (int i = 2; i < n; i++) {
        int c = a + b;
        
        /* Inline assembly with multiple outputs */
        asm volatile (
            "movl %1, %0\n\t"
            "movl %2, %1"
            : "=r" (a), "=r" (b)
            : "0" (b), "1" (c)
        );
    }
    
    return b;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Use __builtin_assume to provide optimization hints */
    __builtin_assume(SIZE > 0);
    
    /* Call functions to create multiple scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(32);
    int sum3 = fibonacci_style(20);
    
    /* Compute final result to prevent dead code elimination */
    int final_result = sum1 + sum2 + sum3;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_result);
    
    /* Verify computation is correct */
    int expected = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected += temp;
        } else {
            expected -= temp;
        }
    }
    
    /* Simple validation */
    if (final_result != expected + sum2 + sum3) {
        printf("Computation error!\n");
        return 1;
    }
    
    return 0;
}
