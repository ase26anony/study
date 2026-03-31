/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern with inline assembly */
            int adjusted;
            asm volatile (
                "imull %2, %1\n\t"
                "addl %1, %0"
                : "+r" (sum), "=&r" (adjusted)
                : "r" (temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "addl %1, %0"
                : "+r" (sum)
                : "r" (temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 2 == 0) {
            /* Memory barrier to prevent reordering */
            asm volatile ("" ::: "memory");
        }
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
void process_matrix(int matrix[SIZE][SIZE], int factor) {
    for (int i = 0; i < SIZE; i++) {
        int row_sum = 0;
        
        /* Inner loop with software pipelining opportunities */
        for (int j = 0; j < SIZE; j++) {
            int val = matrix[i][j];
            
            /* Conditional with arithmetic */
            if (val > 0) {
                /* Complex operation with multiple dependencies */
                int scaled = val * factor;
                
                /* Inline assembly with multiple clobbers */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %%eax, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r" (row_sum)
                    : "r" (scaled)
                    : "%eax", "cc"
                );
            } else {
                row_sum += val;
            }
            
            /* Prevent dead code elimination */
            matrix[i][j] = val + 1;
        }
        
        /* Use result to prevent optimization */
        if (row_sum < 0) {
            matrix[i][0] = row_sum;
        }
    }
}

/* Third function with pointer chasing and loop unrolling hint */
int linked_sum(int* data, int* next, int start, int steps) {
    int sum = 0;
    int pos = start;
    
    /* Loop with pointer chasing dependency */
    for (int i = 0; i < steps; i++) {
        __builtin_assume(pos >= 0 && pos < SIZE);
        
        sum += data[pos];
        
        /* Inline assembly with memory operand */
        asm volatile (
            "addl $1, %0"
            : "+m" (data[pos])
            :
            : "cc"
        );
        
        pos = next[pos];
        
        /* Conditional to create control flow */
        if (sum > 1000000) {
            sum = sum / 2;
        }
    }
    
    return sum;
}

int main() {
    /* Initialize data arrays */
    int a[SIZE], b[SIZE];
    int next[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with predictable but non-constant values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        next[i] = (i + 1) % SIZE;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 200 - 100;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 3);
    int sum2 = linked_sum(a, next, 0, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d\n", sum1, sum2);
    
    /* Verify computation (optional) */
    int verify_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        verify_sum += a[i] * b[i];
    }
    
    if (sum1 != verify_sum) {
        printf("Mismatch: %d vs %d\n", sum1, verify_sum);
        return 1;
    }
    
    return 0;
}
