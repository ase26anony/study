/* test_sel_sched_coverage.c
 * 
 * This test creates conditions that trigger GCC's selective scheduler
 * with pipelining optimizations, forcing it to generate RTL debug output
 * that should execute the uncovered lines in sel_print_insn().
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly to create complex RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:"
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
        
        /* Additional computation to increase scheduling complexity */
        a[i] = (a[i] + b[i]) & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
int matrix_multiply(int size) {
    int result = 0;
    
    /* Use __builtin_assume to provide loop count hints to compiler */
    if (size > 0 && size <= 256) {
        __builtin_assume(size > 0);
        __builtin_assume(size <= 256);
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                int val = i * j;
                
                /* Complex conditional with multiple basic blocks */
                if ((i + j) % 3 == 0) {
                    result += val * 2;
                    
                    /* Mixed precision operations */
                    asm volatile (
                        "movl %1, %%eax\n\t"
                        "imull $2, %%eax\n\t"
                        "addl %%eax, %0"
                        : "+r"(result)
                        : "r"(val)
                        : "%eax", "cc"
                    );
                } else if ((i + j) % 5 == 0) {
                    result -= val;
                    
                    asm volatile (
                        "subl %1, %0\n\t"
                        "testl %0, %0\n\t"
                        "setg %%al\n\t"
                        "movzbl %%al, %%eax\n\t"
                        "addl %%eax, %0"
                        : "+r"(result)
                        : "r"(val)
                        : "%eax", "cc"
                    );
                } else {
                    result += val >> 1;
                }
            }
        }
    }
    
    return result;
}

/* Third function with pointer arithmetic and unrollable loop */
int process_array(int* arr, int n) {
    int sum = 0;
    int* end = arr + n;
    
    /* Pointer-based loop that compiler might software pipeline */
    while (arr < end) {
        int val = *arr;
        
        /* Multiple conditions creating control flow */
        if (val > 0) {
            sum += val * 3;
            
            asm volatile (
                "leal (%1,%1,2), %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "r"(val)
                : "%eax"
            );
        } else if (val < 0) {
            sum -= (-val) * 2;
            
            asm volatile (
                "negl %1\n\t"
                "leal (%1,%1), %%eax\n\t"
                "subl %%eax, %0"
                : "+r"(sum)
                : "r"(val)
                : "%eax", "cc"
            );
        }
        
        arr++;
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(64);
    int sum3 = process_array(a, SIZE);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    printf("Result: %d\n", total);
    
    /* Verify computation is correct (predictable result) */
    int expected = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected += temp;
        } else {
            expected += temp / 2;
        }
    }
    
    /* Simple checksum verification */
    if ((total & 0xFFF) == (expected & 0xFFF)) {
        printf("Verification passed (checksum match)\n");
    } else {
        printf("Verification failed\n");
    }
    
    return 0;
}
