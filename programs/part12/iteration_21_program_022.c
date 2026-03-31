/* test_sel_sched_coverage.c
 * Designed to trigger sel_print_insn() in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -dS test_sel_sched_coverage.c -o test_sel_sched_coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop that should trigger selective scheduling */
int compute_sum_with_conditions(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with data dependency and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex computation with inline assembly to generate non-trivial RTL */
            int adjusted;
            asm volatile (
                "imull %1, %0\n\t"           /* Multiply */
                "addl $100, %0\n\t"          /* Add constant */
                : "=r"(adjusted)
                : "r"(temp), "0"(temp)
                : "cc"
            );
            sum += adjusted;
        } else {
            /* Different computation path */
            int reduced;
            asm volatile (
                "subl $50, %0\n\t"
                "andl $0xFF, %0\n\t"
                : "=r"(reduced)
                : "0"(temp)
                : "cc"
            );
            sum += reduced;
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 2 == 0) {
            asm volatile (
                "xorl %%eax, %%eax\n\t"
                "cpuid\n\t"
                : 
                : "a"(0)
                : "ebx", "ecx", "edx", "memory"
            );
        }
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
void process_matrix(int mat[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Data-dependent computation with inline assembly */
            int val = mat[i][j];
            int processed;
            
            asm volatile (
                "movl %1, %%eax\n\t"
                "leal (%%eax,%%eax,2), %%eax\n\t"  /* val * 3 */
                "addl $42, %%eax\n\t"
                : "=a"(processed)
                : "r"(val)
                : "cc"
            );
            
            /* Conditional with complex RTL patterns */
            if (processed > 1000) {
                asm volatile (
                    "shrl $2, %0\n\t"
                    : "+r"(processed)
                    :: "cc"
                );
            }
            
            row_sum += processed;
            mat[i][j] = processed;
        }
        
        /* Store result with memory barrier */
        asm volatile (
            "mfence\n\t"
            ::: "memory"
        );
        
        /* Use __builtin_assume to help compiler optimization */
        if (row_sum > 0) {
            __builtin_assume(row_sum > 0);
        }
    }
}

/* Main function with constant loop counts for compile-time scheduling */
int main() {
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int matrix[8][16];
    
    /* Initialize arrays with predictable but non-constant patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 100 + j * 7) % 2000;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum_with_conditions(a, b, ARRAY_SIZE);
    
    /* Use __builtin_assume to inform compiler about loop bounds */
    __builtin_assume(ARRAY_SIZE > 0);
    
    process_matrix(matrix, 8, 16);
    
    /* Compute verification sum */
    int verify_sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            verify_sum += matrix[i][j];
        }
    }
    
    printf("Result 1: %d\n", sum1);
    printf("Result 2: %d\n", verify_sum);
    
    /* Return deterministic result for verification */
    return (sum1 + verify_sum) > 0 ? 0 : 1;
}
