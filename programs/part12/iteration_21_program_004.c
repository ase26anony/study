/* test_sel_sched.c - Test program to trigger sel_print_insn coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency - creates scheduling region */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern with inline assembly */
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
        sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return sum;
}

/* Second function with nested loops for more complex scheduling */
int matrix_multiply(int *mat1, int *mat2, int *result, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int dot = 0;
            
            /* Innermost loop with heavy computation */
            for (int k = 0; k < n; k++) {
                /* Data dependency chain */
                dot += mat1[i * n + k] * mat2[k * n + j];
                
                /* Conditional with inline assembly */
                if (dot > 1000) {
                    asm volatile (
                        "imull %1, %0\n\t"
                        "shrl $8, %0"
                        : "+r" (dot)
                        : "r" (k)
                        : "cc"
                    );
                }
            }
            
            result[i * n + j] = dot;
            total += dot;
        }
    }
    
    return total;
}

/* Function with pointer chasing to create complex memory dependencies */
int linked_sum(int *data, int *next, int start, int steps) {
    int sum = 0;
    int idx = start;
    
    for (int i = 0; i < steps; i++) {
        /* Memory load with dependency */
        int val = data[idx];
        
        /* Conditional with arithmetic */
        if (val & 1) {
            asm volatile (
                "xorl %%eax, %%eax\n\t"
                "cpuid"
                : 
                : "a" (0)
                : "ebx", "ecx", "edx"
            );
            sum += val * 3;
        } else {
            sum -= val / 2;
        }
        
        /* Pointer chasing */
        idx = next[idx];
        
        /* Loop invariant that compiler must handle */
        __builtin_assume(idx >= 0 && idx < SIZE);
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int mat1[16], mat2[16], result[16];
    int next[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
        next[i] = (i + 1) % SIZE;
    }
    
    for (int i = 0; i < 16; i++) {
        mat1[i] = i;
        mat2[i] = 16 - i;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(mat1, mat2, result, 4);
    int sum3 = linked_sum(a, next, 0, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d\n", sum1, sum2, sum3);
    
    /* Verify computation is correct */
    int expected_sum1 = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected_sum1 += temp;
        } else {
            expected_sum1 -= temp;
        }
        expected_sum1 = (expected_sum1 * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Expected sum1: %d\n", expected_sum1);
    printf("Computed sum1: %d\n", sum1);
    
    return (sum1 == expected_sum1) ? 0 : 1;
}
