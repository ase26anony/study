/* test_sel_sched_coverage.c
 * 
 * This program is designed to trigger GCC's selective scheduler
 * with pipelining to exercise the sel_print_insn function's
 * debug output switching logic in sel-sched-dump.cc.
 * 
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 
 *               -fdump-rtl-sched2 -dS -march=native test_sel_sched_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define THRESHOLD 500

/* Function with tight, data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Loop with carried dependency and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with dependency chain */
        int temp = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly to create complex RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl %2, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "r"(temp), "r"(THRESHOLD * 2)
                : "rax", "cc"
            );
        } else {
            sum += temp / 2;
            
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull %1, %0\n\t"
                "addl $1, %0"
                : "+r"(sum)
                : "r"(i)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        a[i] = (a[i] + b[i]) * i;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
int matrix_multiply_accumulate(int size) {
    int total = 0;
    
    /* Nested loops create complex control flow */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int val = i * j;
            
            /* Multiple conditions create basic blocks */
            if (val % 3 == 0) {
                total += val;
                
                /* Complex inline assembly with multiple clobbers */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %%eax, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(total)
                    : "r"(val)
                    : "rax", "cc"
                );
            } else if (val % 5 == 0) {
                total -= val;
            } else {
                total ^= val;
            }
            
            /* Prevent dead code elimination */
            __asm__ __volatile__("" : : "r"(total) : "memory");
        }
    }
    
    return total;
}

/* Main function with multiple hot loops */
int main() {
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    
    /* Initialize arrays with predictable but non-trivial patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
    }
    
    /* Call functions that should trigger selective scheduling */
    int result1 = compute_sum_with_conditions(a, b, ARRAY_SIZE);
    int result2 = matrix_multiply_accumulate(64);
    
    /* Use __builtin_assume to provide optimization hints */
    if (result1 > 0) {
        __builtin_assume(result1 > 0);
    }
    
    /* Final computation with loop carried dependency */
    int final_sum = 0;
    for (int i = 0; i < 1000; i++) {
        final_sum = final_sum * 13 + a[i % ARRAY_SIZE];
        
        /* Mixed integer operations for varied RTL patterns */
        if (final_sum & 1) {
            final_sum = (final_sum << 3) | (final_sum >> 29);
        } else {
            final_sum = (final_sum + i) ^ 0x5A5A5A5A;
        }
    }
    
    /* Print results to prevent optimization */
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Final sum: %d\n", final_sum);
    
    /* Return predictable value for verification */
    return (result1 + result2 + final_sum) > 0 ? 0 : 1;
}
