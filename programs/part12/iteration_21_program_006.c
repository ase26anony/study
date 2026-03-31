/* test_sel_sched_coverage.c
 * 
 * This test is designed to trigger the selective scheduler's debugging output
 * to cover the sel_print_insn function in sel-sched-dump.cc, specifically
 * the block that switches dump output to stderr, prints RTL, and restores.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -march=x86-64 test_sel_sched_coverage.c -o test_sel_sched_executable
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 500

/* Function with tight, data-dependent loop that benefits from software pipelining */
int compute_sum_with_conditions(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (a[i] > THRESHOLD) {
            /* Complex computation path */
            temp += (a[i] >> 3) | (b[i] << 2);
            
            /* Inline assembly to create non-trivial RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "andl $0xFFF, %0"
                : "+r"(temp)
                : "r"(i)
                : "cc"
            );
        } else {
            /* Alternative path with different operations */
            temp -= (b[i] % 16);
            
            /* Another inline assembly with multiple clobbers */
            asm volatile (
                "imull %1, %0\n\t"
                "subl $1, %0"
                : "+r"(temp)
                : "r"(a[i])
                : "cc", "eax", "edx"
            );
        }
        
        /* Additional conditional to create more scheduling opportunities */
        if (temp & 1) {
            /* Use inline assembly with memory operand */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "m"(temp)
                : "eax", "cc"
            );
        } else {
            sum += temp;
        }
        
        /* Prevent loop unrolling from eliminating scheduling complexity */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Second function with nested loops for additional scheduling regions */
int matrix_multiply_accumulate(int* restrict mat, int rows, int cols) {
    int total = 0;
    
    /* Nested loops create complex scheduling regions */
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access pattern */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = mat[idx];
            
            /* Conditional with arithmetic */
            if (val > 0) {
                row_sum += val * (j + 1);
                
                /* Inline assembly with multiple constraints */
                asm volatile (
                    "leal (%1,%2,2), %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(row_sum)
                    : "r"(val), "r"(j)
                    : "eax", "cc"
                );
            } else {
                row_sum -= (-val) >> 2;
            }
            
            /* Memory barrier to prevent over-optimization */
            asm volatile ("" : : "r"(row_sum), "m"(mat[idx]) : "memory");
        }
        
        /* Conditional accumulation */
        if (row_sum > 1000) {
            total += row_sum >> 1;
        } else {
            total += row_sum;
        }
    }
    
    return total;
}

/* Helper to initialize arrays with pattern */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 37) % 1000;  /* Pattern ensures some values > THRESHOLD */
        b[i] = (i * 53) % 1000;
    }
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int matrix[SIZE];
    
    /* Initialize data */
    init_arrays(a, b, SIZE);
    for (int i = 0; i < SIZE; i++) {
        matrix[i] = (i * 73) % 2000 - 1000;  /* Mix of positive and negative */
    }
    
    /* Compute results using both functions */
    int result1 = compute_sum_with_conditions(a, b, SIZE);
    int result2 = matrix_multiply_accumulate(matrix, 32, 32);  /* 32x32 matrix */
    
    /* Use results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    /* Print result for verification */
    printf("Computed result: %d\n", final_result);
    
    /* Additional computation to ensure loops aren't optimized away */
    volatile int check = final_result;
    if (check != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
