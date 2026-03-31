/* test_sel_sched.c - Test program to trigger sel_print_insn coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop for selective scheduling */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex computation with inline assembly */
            int x = temp;
            int y = i;
            
            /* Inline assembly creating non-trivial RTL patterns */
            asm volatile (
                "imul %1, %0\n\t"
                "addl $1, %0"
                : "+r" (x)
                : "r" (y)
                : "cc"
            );
            
            sum += x;
        } else {
            /* Alternative path with different computation */
            sum += temp >> 1;
        }
        
        /* Additional inline assembly to create scheduling complexity */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %%eax, %0"
            : "+r" (sum)
            : "r" (i)
            : "%eax", "cc"
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
            
            /* Conditional with multiple operations */
            if (val & 1) {
                /* Odd value processing */
                asm volatile (
                    "lea (%1, %1, 2), %0"
                    : "=r" (val)
                    : "r" (val)
                );
                row_sum += val;
            } else {
                /* Even value processing */
                row_sum += val * 3;
            }
            
            /* Prevent dead code elimination */
            matrix[i][j] = val;
        }
        
        /* Store result */
        matrix[i][0] = row_sum;
    }
}

/* Helper function with loop unrolling hints */
int vector_dot_product(const int* restrict v1, const int* restrict v2, int n) {
    int dot = 0;
    
    /* Loop with known bounds for better scheduling */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dot += v1[i] * v2[i];
        
        /* Use __builtin_assume to help compiler with optimization */
        if (i < n - 1) {
            __builtin_assume(v1[i+1] != 0);
        }
    }
    
    return dot;
}

int main() {
    /* Initialize test data */
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Initialize arrays with predictable but non-constant values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) % 50;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 8, 16);
    int sum2 = vector_dot_product(a, b, SIZE);
    
    /* Use results to prevent optimization */
    printf("Results: %d %d %d\n", 
           sum1, 
           matrix[0][0], 
           sum2);
    
    /* Verify computation is correct */
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected_sum += temp * i + 1 + i;
        } else {
            expected_sum += (temp >> 1) + i;
        }
    }
    
    printf("Expected sum1: %d\n", expected_sum);
    printf("Test %s\n", (sum1 == expected_sum) ? "PASSED" : "FAILED");
    
    return 0;
}
