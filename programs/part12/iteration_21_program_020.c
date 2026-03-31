/* test_sel_sched_coverage.c
 * Designed to trigger selective scheduler debugging output
 * that exercises sel_print_insn with RTL dump switching
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex computation path */
            sum += temp;
            
            /* Inline assembly to create non-trivial RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "cc", "memory"
            );
        } else {
            /* Alternative path with different operations */
            sum -= temp / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "imull $3, %0\n\t"
                "andl $0xFF, %0"
                : "+r"(sum)
                :
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 16 == 0) {
            /* Nested condition creates more control flow */
            asm volatile (
                "xorl %%eax, %%eax\n\t"
                "cpuid"
                : 
                : "a"(0)
                : "ebx", "ecx", "edx", "memory"
            );
        }
    }
    
    return sum;
}

/* Second function with different loop structure */
int compute_product_with_branches(int *arr, int n) {
    int prod = 1;
    int i = 0;
    
    /* While loop with early exit condition */
    while (i < n) {
        /* Multiple conditions in loop */
        if (arr[i] > 0) {
            prod *= arr[i];
            
            /* Prevent overflow */
            if (prod > 1000000) {
                prod /= 2;
                
                /* Memory barrier asm */
                asm volatile ("" ::: "memory");
            }
        } else if (arr[i] < 0) {
            prod /= -arr[i];
        }
        
        /* Loop with stride */
        i += (arr[i] % 3) + 1;
        
        /* Another asm with multiple outputs */
        int old_prod = prod;
        asm volatile (
            "movl %1, %%eax\n\t"
            "shrl $2, %%eax\n\t"
            "addl %%eax, %0"
            : "+r"(prod)
            : "r"(old_prod)
            : "eax", "cc"
        );
    }
    
    return prod;
}

/* Helper function to initialize arrays */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 37) % 256;
        b[i] = (i * 73) % 256;
    }
}

int main(void) {
    int array_a[SIZE];
    int array_b[SIZE];
    int result1, result2;
    
    /* Initialize with pseudo-random but deterministic values */
    init_arrays(array_a, array_b, SIZE);
    
    /* Call functions that should trigger selective scheduling */
    result1 = compute_sum_with_conditions(array_a, array_b, SIZE);
    
    /* Use __builtin_assume to provide optimization hints */
    if (result1 > 0) {
        __builtin_assume(result1 > 0);
    }
    
    result2 = compute_product_with_branches(array_a, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Final computation using both results */
    int final_result = result1 + result2;
    
    /* Use inline asm to ensure the computation isn't optimized away */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r"(final_result)
        : "r"(SIZE)
        : "cc"
    );
    
    printf("Final result: %d\n", final_result);
    
    /* Return deterministic value for verification */
    return (final_result > 0) ? 0 : 1;
}
