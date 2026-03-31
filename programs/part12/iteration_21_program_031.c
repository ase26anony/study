/* test_sel_sched_coverage.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Specifically targets lines 159-163 in sel_print_insn function
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from software pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        /* Complex expression with multiple operations to create scheduling opportunities */
        int temp = a[i] * b[i];
        
        /* Conditional inside loop creates multiple basic blocks */
        if (temp > THRESHOLD) {
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
                : "cc"
            );
        } else {
            sum += temp / 2;
            
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull $3, %0, %0\n\t"
                "andl $0xFF, %0"
                : "+r"(sum)
                :
                : "cc"
            );
        }
        
        /* Additional arithmetic to increase instruction count */
        sum = (sum * 3) / 2;
    }
    
    return sum;
}

/* Second function with different loop pattern */
int compute_weighted_sum(int* restrict arr, int n) {
    int result = 0;
    int weight = 7;
    
    /* Loop with carried dependency through 'weight' variable */
    for (int i = 0; i < n; i++) {
        weight = (weight * 13 + 1) & 0xFF;
        
        /* Nested conditional for more complex control flow */
        if (arr[i] > 0) {
            if (arr[i] < 100) {
                result += arr[i] * weight;
                
                /* Complex inline assembly with multiple clobbers */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "mull %2\n\t"
                    "addl %%eax, %0\n\t"
                    "adcl $0, %0"
                    : "+r"(result)
                    : "r"(arr[i]), "r"(weight)
                    : "eax", "edx", "cc"
                );
            } else {
                result += arr[i] / weight;
            }
        }
        
        /* Prevent loop unrolling from being too aggressive */
        __asm__ __volatile__("" ::: "memory");
    }
    
    return result;
}

/* Third function with pointer chasing */
int linked_sum(int* data, int* next, int start, int steps) {
    int sum = 0;
    int idx = start;
    
    for (int i = 0; i < steps; i++) {
        if (idx >= 0 && idx < SIZE) {
            sum += data[idx];
            
            /* Memory barrier inline assembly */
            asm volatile (
                "lock addl $0, (%0)\n\t"
                : 
                : "r"(&data[idx])
                : "memory"
            );
            
            idx = next[idx];
        } else {
            break;
        }
    }
    
    return sum;
}

int main() {
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* next = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b || !next) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 17 + 3) % 256;
        b[i] = (i * 23 + 5) % 256;
        next[i] = (i + 1) % SIZE;
    }
    
    /* Call functions to ensure they're not optimized away */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = compute_weighted_sum(a, SIZE);
    int sum3 = linked_sum(a, next, 0, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    /* Print result for verification */
    printf("Result: %d (checksum: %08x)\n", total, total);
    
    /* Clean up */
    free(a);
    free(b);
    free(next);
    
    return 0;
}
