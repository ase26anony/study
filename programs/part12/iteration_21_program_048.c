/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for software pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Create scheduling region with conditional */
        if (a[i] > THRESHOLD) {
            /* Complex arithmetic with dependency chain */
            int temp = a[i] * b[i];
            
            /* Inline assembly to generate non-trivial RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "adcl $0, %0"
                : "+r" (sum)
                : "r" (temp)
                : "cc"
            );
        } else {
            /* Alternative path with different operations */
            int diff = b[i] - a[i];
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "subl %1, %0\n\t"
                "cmpl $0, %0"
                : "+r" (sum)
                : "r" (diff)
                : "cc", "memory"
            );
        }
        
        /* Additional computation to extend basic block */
        a[i] = (a[i] * 3 + 7) & 0xFF;
        b[i] = (b[i] * 5 + 11) & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
void process_matrix(int mat[][16], int rows) {
    int total = 0;
    
    /* Nested loops create complex control flow */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 16; j++) {
            /* Multiple conditions create multi-block regions */
            if (mat[i][j] > 50) {
                /* Complex RTL pattern with multiple operations */
                int val = mat[i][j];
                asm volatile (
                    "imull %1, %0\n\t"
                    "addl %%eax, %2"
                    : "+r" (val), "+r" (total)
                    : "r" (j)
                    : "eax", "cc"
                );
                mat[i][j] = val;
            } else if (mat[i][j] < -50) {
                /* Different arithmetic pattern */
                mat[i][j] = -mat[i][j];
                asm volatile (
                    "negl %0"
                    : "+r" (mat[i][j])
                    :
                    : "cc"
                );
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    asm volatile ("" : : "r" (total) : "memory");
}

/* Main function with multiple hot loops */
int main() {
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Initialize arrays with predictable but non-constant values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 17 + 23) % 256;
        b[i] = (i * 13 + 29) % 256;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 31 + j * 7) % 200 - 100;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 8);
    
    /* Additional loop with pointer arithmetic */
    int *pa = a;
    int *pb = b;
    int sum2 = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Pointer-based access creates different addressing modes */
        int prod = (*pa) * (*pb);
        
        /* Complex inline assembly with multiple constraints */
        asm volatile (
            "leal (%1, %2), %%eax\n\t"
            "addl %%eax, %0"
            : "+r" (sum2)
            : "r" (prod), "r" (i)
            : "eax", "cc"
        );
        
        pa++;
        pb++;
    }
    
    /* Combine results to produce final output */
    int final_result = sum1 + sum2;
    
    /* Use __builtin_assume to provide optimization hints */
    if (final_result > 0) {
        __builtin_assume(final_result > 0);
    }
    
    printf("Result: %d\n", final_result);
    
    /* Return non-zero to ensure all code paths matter */
    return (final_result != 0) ? 0 : 1;
}
