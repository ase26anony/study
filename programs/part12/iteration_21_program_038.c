/* test_sel_sched_coverage.c
 * 
 * This program is designed to trigger GCC's selective scheduler
 * debugging output, specifically the sel_print_insn function
 * that switches dump output to stderr and prints RTL.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 
 *               -fdump-rtl-sched2 -dS -march=native 
 *               test_sel_sched_coverage.c -o test_sel_sched_coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - creates scheduling regions */
    for (int i = 0; i < n; i++) {
        /* Data dependency: sum depends on previous iteration */
        sum += a[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (a[i] > THRESHOLD) {
            /* Complex computation with another dependency */
            sum += b[i] * 2;
            
            /* Inline assembly to generate non-trivial RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl %2, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %0"
                : "+r"(sum)
                : "r"(i), "r"(THRESHOLD)
                : "al", "cc"
            );
        } else {
            /* Different path with arithmetic */
            sum -= b[i] / 3;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "imull %1, %0\n\t"
                "addl $42, %0"
                : "+r"(sum)
                : "r"(i)
                : "cc"
            );
        }
        
        /* Additional computation to increase instruction count */
        sum = (sum * 3) / 2;
    }
    
    return sum;
}

/* Another function with nested loops for more complex scheduling */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    int temp = 0;
    
    /* Nested loops create interesting scheduling regions */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Data-dependent computation */
            temp = mat[i][j] * factor;
            
            /* Conditional with inline assembly */
            if (temp > 1000) {
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "cltd\n\t"
                    "idivl %2\n\t"
                    "movl %%eax, %0"
                    : "=r"(mat[i][j])
                    : "r"(temp), "r"(factor)
                    : "eax", "edx", "cc"
                );
            } else {
                mat[i][j] = temp + j;
            }
            
            /* Prevent dead code elimination */
            asm volatile ("" : : "r"(mat[i][j]) : "memory");
        }
    }
}

/* Function with pointer chasing to create memory dependencies */
int linked_sum(int *data, int *next, int start, int steps) {
    int sum = 0;
    int idx = start;
    
    for (int i = 0; i < steps; i++) {
        if (idx >= 0 && idx < SIZE) {
            sum += data[idx];
            
            /* Memory dependency through pointer chasing */
            idx = next[idx];
            
            /* Inline assembly with memory clobber */
            asm volatile (
                "movl (%1), %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "r"(&data[idx])
                : "eax", "memory"
            );
        }
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int next[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with pattern to create predictable branches */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 1000;          /* Values 0-999 */
        b[i] = (i * 7) % 1000;          /* Values 0-999 */
        next[i] = (i + 1) % SIZE;       /* Simple linked list */
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 2000;
        }
    }
    
    /* Use __builtin_assume to provide optimization hints */
    __builtin_assume(SIZE > 0);
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum_with_conditions(a, b, SIZE);
    process_matrix(matrix, 3);
    int sum2 = linked_sum(a, next, 0, SIZE / 2);
    
    /* Final computation to prevent dead code elimination */
    int result = sum1 + sum2 + matrix[0][0];
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    /* Verify computation (expected value based on initialization) */
    int expected = 0;
    for (int i = 0; i < SIZE; i++) {
        expected += a[i];
        if (a[i] > THRESHOLD) {
            expected += b[i] * 2 + i;
        } else {
            expected -= b[i] / 3 - i;
        }
        expected = (expected * 3) / 2;
    }
    
    /* Simple linked sum calculation */
    int idx = 0;
    for (int i = 0; i < SIZE / 2; i++) {
        expected += a[idx];
        idx = next[idx];
    }
    
    expected += matrix[0][0];
    
    if (result == expected) {
        printf("Computation correct!\n");
        return 0;
    } else {
        printf("Computation mismatch: got %d, expected %d\n", result, expected);
        return 1;
    }
}
