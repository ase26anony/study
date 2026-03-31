/* test_sel_sched_coverage.c
 * 
 * This program is designed to trigger the selective scheduler's debug output
 * to cover the uncovered lines in sel-sched-dump.cc:
 *   switch_dump (stderr);
 *   dump_insn_rtx_1 (insn, debug_insn_rtx_flags);
 *   sel_print ("\n");
 *   restore_dump ();
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -dS test_sel_sched_coverage.c -o test_sel_sched_coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop that benefits from software pipelining */
int compute_sum_with_conditions(int* a, int* b, int size) {
    int sum = 0;
    
    /* Loop with data dependency and conditional control flow */
    for (int i = 0; i < size; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern: arithmetic with conditional update */
            sum += temp;
            
            /* Inline assembly to create non-trivial RTL instructions */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n\t"
                "1:\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Alternative path with different computation */
            sum += temp / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "shrl $1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 8 == 0) {
            /* Nested condition with inline assembly */
            int mod_result = a[i] % 16;
            asm volatile (
                "andl $15, %1\n\t"
                "addl %1, %0\n\t"
                : "+r"(sum)
                : "r"(mod_result)
                : "cc"
            );
        }
    }
    
    return sum;
}

/* Second function with different loop structure for additional scheduling regions */
int compute_weighted_sum(int* a, int* b, int* weights, int size) {
    int result = 0;
    
    /* Unrolled loop with software pipelining opportunities */
    for (int i = 0; i < size; i += 4) {
        int sum1 = a[i] * weights[i];
        int sum2 = b[i] * weights[i];
        
        /* Complex conditional with multiple basic blocks */
        if (sum1 > sum2) {
            result += sum1 - sum2;
            
            /* Inline assembly with multiple outputs */
            int diff;
            asm volatile (
                "subl %2, %1\n\t"
                "movl %1, %0\n\t"
                : "=r"(diff), "+r"(sum1)
                : "r"(sum2)
                : "cc"
            );
            
            /* Use the result to prevent dead code elimination */
            result += diff;
        } else {
            result += sum2 - sum1;
        }
        
        /* Prevent loop invariant code motion */
        if (i < size - 4) {
            /* Additional computation with inline assembly */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(result)
                : "r"(weights[i])
                : "cc"
            );
        }
    }
    
    return result;
}

/* Helper function to initialize arrays */
void initialize_arrays(int* a, int* b, int* weights, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
        weights[i] = (i % 10) + 1;
    }
}

/* Main function with multiple computation paths */
int main() {
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int weights[ARRAY_SIZE];
    
    /* Initialize data */
    initialize_arrays(a, b, weights, ARRAY_SIZE);
    
    /* Perform computations that will trigger selective scheduling */
    int sum1 = compute_sum_with_conditions(a, b, ARRAY_SIZE);
    int sum2 = compute_weighted_sum(a, b, weights, ARRAY_SIZE);
    
    /* Final result computation with inline assembly */
    int final_result;
    asm volatile (
        "addl %1, %0\n\t"
        "imull $2, %0\n\t"
        : "=r"(final_result)
        : "r"(sum1), "0"(sum2)
        : "cc"
    );
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    /* Additional loop to ensure scheduler sees more opportunities */
    int verification_sum = 0;
    for (int i = 0; i < 100; i++) {
        int x = i * i;
        int y = i * 2;
        
        /* Complex inline assembly pattern */
        asm volatile (
            "leal (%1, %2, 2), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r"(verification_sum)
            : "r"(x), "r"(y)
            : "%eax", "cc"
        );
    }
    
    printf("Verification sum: %d\n", verification_sum);
    
    return (final_result > 0 && verification_sum > 0) ? 0 : 1;
}
