/* test_sel_sched.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex computation with inline assembly */
            int adjusted = temp;
            
            /* Inline assembly with multiple operands to generate complex RTL */
            asm volatile (
                "imul %1, %0\n\t"           /* Multiply */
                "addl $1, %0\n\t"           /* Add constant */
                : "+r" (adjusted)           /* Output operand */
                : "r" (i)                   /* Input operand */
                : "cc"                      /* Clobber flags */
            );
            
            sum += adjusted;
        } else {
            /* Another path with different computation */
            sum += temp / 2;
        }
        
        /* Additional inline assembly to create more scheduling opportunities */
        asm volatile (
            "add %1, %0\n\t"
            "cmpl $0, %0\n\t"
            : "+r" (sum)
            : "r" (temp & 0xFF)
            : "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
void process_matrix(int matrix[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with pointer arithmetic */
        int* row_ptr = matrix[i];
        for (int j = 0; j < cols; j++) {
            /* Data dependency chain */
            row_sum = row_sum * 3 + row_ptr[j];
            
            /* Conditional with multiple basic blocks */
            if (row_sum > 1000) {
                /* Inline assembly with memory operand */
                asm volatile (
                    "andl $0xFF, %0\n\t"
                    : "+r" (row_sum)
                    :
                    : "cc"
                );
            }
        }
        
        /* Store result with barrier */
        asm volatile ("" ::: "memory");
        matrix[i][0] = row_sum;
    }
}

/* Helper to prevent dead code elimination */
volatile int global_result = 0;

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Fill arrays with values that create varied execution paths */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 5;
        }
    }
    
    /* Force compiler to assume loop bounds */
    int n = SIZE;
    __builtin_assume(n > 0 && n <= SIZE);
    
    /* Compute results using both functions */
    int result1 = compute_sum(a, b, n);
    process_matrix(matrix, 8, 16);
    
    /* Combine results to prevent optimization */
    int result2 = 0;
    for (int i = 0; i < 8; i++) {
        result2 += matrix[i][0];
    }
    
    int final_result = result1 + result2;
    
    /* Use result to prevent dead code elimination */
    global_result = final_result;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
