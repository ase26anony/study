/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop and control flow */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency and conditional */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional creates multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Inline assembly with multiple operands and clobbers */
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
        
        /* Additional computation to create more scheduling opportunities */
        int idx = i & 0xFF;
        asm volatile (
            "imull %1, %0\n\t"
            "addl %%eax, %0"
            : "+r" (sum)
            : "r" (idx)
            : "%eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
int matrix_multiply(int n) {
    int A[SIZE], B[SIZE], C[SIZE];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
        C[i] = 0;
    }
    
    /* Nested loops create complex scheduling regions */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int idx = (i * n + j) % SIZE;
            int prod = A[idx] * B[idx];
            
            /* Complex conditional with multiple paths */
            if (prod & 1) {
                C[idx] += prod;
                asm volatile (
                    "rorl $1, %0\n\t"
                    "addl %1, %0"
                    : "+r" (C[idx])
                    : "r" (prod)
                    : "cc"
                );
            } else {
                C[idx] -= prod;
                asm volatile (
                    "roll $1, %0\n\t"
                    "subl %1, %0"
                    : "+r" (C[idx])
                    : "r" (prod)
                    : "cc"
                );
            }
            
            /* Accumulate result with carry simulation */
            asm volatile (
                "addl %1, %0\n\t"
                "setc %%al\n\t"
                "movzbl %%al, %%eax\n\t"
                "addl %%eax, %0"
                : "+r" (result)
                : "r" (C[idx])
                : "%eax", "cc"
            );
        }
    }
    
    return result;
}

/* Third function with switch statement for varied control flow */
int process_with_switch(int* arr, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Switch creates multiple basic blocks */
        switch (val & 0x3) {
            case 0:
                asm volatile (
                    "shll $2, %0\n\t"
                    "addl $1, %0"
                    : "+r" (val)
                    :: "cc"
                );
                total += val;
                break;
            case 1:
                asm volatile (
                    "shrl $1, %0\n\t"
                    "xorl $0x55, %0"
                    : "+r" (val)
                    :: "cc"
                );
                total -= val;
                break;
            case 2:
                asm volatile (
                    "imull $3, %0\n\t"
                    "andl $0xFF, %0"
                    : "+r" (val)
                    :: "%eax", "cc"
                );
                total ^= val;
                break;
            case 3:
                asm volatile (
                    "neg %0\n\t"
                    "orl $0x80, %0"
                    : "+r" (val)
                    :: "cc"
                );
                total |= val;
                break;
        }
    }
    
    return total;
}

int main() {
    int data_a[SIZE], data_b[SIZE];
    int result1, result2, result3;
    
    /* Initialize with predictable but non-trivial patterns */
    for (int i = 0; i < SIZE; i++) {
        data_a[i] = i * 3 + 1;
        data_b[i] = i * 5 - 2;
    }
    
    /* Use __builtin_assume to provide loop bound hints */
    if (SIZE > 0) {
        __builtin_assume(SIZE <= 1024);
    }
    
    /* Call all three functions to create multiple scheduling regions */
    result1 = compute_sum(data_a, data_b, SIZE);
    result2 = matrix_multiply(32);  /* 32x32 matrix */
    
    int switch_data[SIZE];
    for (int i = 0; i < SIZE; i++) {
        switch_data[i] = i * 7 % 256;
    }
    result3 = process_with_switch(switch_data, SIZE);
    
    /* Compute final result to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_result);
    
    /* Additional volatile asm to prevent over-optimization */
    asm volatile ("" : : "r"(final_result) : "memory");
    
    return (final_result != 0) ? 0 : 1;
}
