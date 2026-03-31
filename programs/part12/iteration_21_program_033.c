/* test_sel_sched_coverage.c
 * Designed to trigger selective scheduler debugging output
 * that calls sel_print_insn with the uncovered dump switching logic.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int* a, int* b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional creates multiple basic blocks for scheduler */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern with inline assembly */
            int adjusted;
            asm volatile (
                "imul %2, %1\n\t"
                "add %1, %0"
                : "+r"(sum), "=&r"(adjusted)
                : "r"(temp)
                : "cc"
            );
            
            /* Additional arithmetic to create more scheduling opportunities */
            adjusted = adjusted >> 2;
            asm volatile (
                "sar $2, %0\n\t"
                "add $1, %0"
                : "+r"(adjusted)
                :
                : "cc"
            );
            
            sum += adjusted;
        } else {
            /* Alternative path with different operations */
            int reduced = temp / 3;
            
            /* Another inline asm with clobbers */
            asm volatile (
                "mov %1, %%eax\n\t"
                "cdq\n\t"
                "mov $3, %%ecx\n\t"
                "idiv %%ecx\n\t"
                "add %%eax, %0"
                : "+r"(sum)
                : "r"(temp)
                : "eax", "edx", "ecx", "cc"
            );
            
            /* Prevent dead code elimination */
            sum += (reduced & 1);
        }
        
        /* Cross-iteration dependency */
        a[i] = sum & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
int matrix_multiply_accumulate(int n) {
    int total = 0;
    
    /* Nested loops create complex scheduling graphs */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = i * j;
            
            /* Multiple conditions create control flow */
            if (val & 1) {
                /* SIMD-like operation simulated with inline asm */
                int doubled;
                asm volatile (
                    "lea (%1, %1), %0"
                    : "=r"(doubled)
                    : "r"(val)
                );
                
                total += doubled;
                
                /* Memory barrier asm to create scheduling barriers */
                asm volatile ("" ::: "memory");
            } else {
                total -= val;
            }
            
            /* Modulo operation creates complex RTL */
            if (j % 8 == 0) {
                asm volatile (
                    "and $7, %0"
                    : "+r"(val)
                );
                total ^= val;
            }
        }
    }
    
    return total;
}

/* Helper to initialize arrays */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 37) & 0xFF;
        b[i] = (i * 73) & 0xFF;
    }
}

int main() {
    int a[SIZE];
    int b[SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(a, b, SIZE);
    
    /* Compute with selective scheduling opportunities */
    int result1 = compute_sum_with_conditions(a, b, SIZE);
    
    /* Second computation with different pattern */
    int result2 = matrix_multiply_accumulate(32);
    
    /* Use results to prevent optimization */
    int final_result = result1 + result2;
    
    /* Print to verify correctness and prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Additional volatile access to ensure all computations are kept */
    volatile int check = final_result;
    
    return (check > 0) ? 0 : 1;
}
