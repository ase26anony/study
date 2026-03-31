/* test_sel_sched_coverage.c
 * 
 * This test program is designed to trigger the selective scheduler's
 * debugging output functions, specifically sel_print_insn() in
 * sel-sched-dump.cc, to cover the lines that switch dump output to
 * stderr, print RTL, and restore the original dump stream.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 
 *                -fdump-rtl-sched2 -dS -march=x86-64 -o test_sel_sched 
 *                test_sel_sched_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow creating multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly to create complex RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:\n\t"
                : "+r"(sum)
                : "r"(i)
                : "cc"
            );
        } else {
            sum += temp / 2;
            
            /* Another inline assembly with multiple clobbers */
            asm volatile (
                "imull %1, %0\n\t"
                "addl $1, %0"
                : "+r"(sum)
                : "r"(i)
                : "cc", "eax", "edx"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        a[i] = (a[i] + b[i]) * 2;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
void process_matrix(int mat[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with software pipelining opportunities */
        for (int j = 0; j < cols; j++) {
            int val = mat[i][j];
            
            /* Complex conditional with multiple branches */
            if (val & 1) {
                val = (val * 3 + 1) >> 1;
                
                /* Inline asm with memory operand */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "shrl $2, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(row_sum)
                    : "m"(val)
                    : "eax", "cc"
                );
            } else {
                val = val / 2;
                
                asm volatile (
                    "xorl %%edx, %%edx\n\t"
                    "movl %1, %%eax\n\t"
                    "movl $7, %%ecx\n\t"
                    "divl %%ecx\n\t"
                    "addl %%eax, %0"
                    : "+r"(row_sum)
                    : "r"(val)
                    : "eax", "edx", "ecx", "cc"
                );
            }
            
            mat[i][j] = val;
        }
        
        /* Store result with memory barrier */
        asm volatile (
            "mfence\n\t"
            "movl %0, %1"
            : 
            : "r"(row_sum), "m"(mat[i][0])
            : "memory"
        );
    }
}

/* Main function that creates hot loops for the scheduler */
int main() {
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int matrix[8][16];
    
    /* Initialize arrays with predictable but non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 5;
        }
    }
    
    /* Tell compiler loop bounds are known */
    __builtin_assume(ARRAY_SIZE == 1024);
    
    /* Compute results using functions designed for scheduling */
    int sum1 = compute_sum_with_conditions(a, b, ARRAY_SIZE);
    process_matrix(matrix, 8, 16);
    
    /* Additional computation to prevent dead code elimination */
    int sum2 = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            sum2 += matrix[i][j];
        }
    }
    
    /* Final result for verification */
    int final_result = sum1 + sum2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify computation is correct */
    if (final_result != 1305600) {  /* Pre-computed expected value */
        fprintf(stderr, "Error: Unexpected result %d\n", final_result);
        return 1;
    }
    
    return 0;
}
