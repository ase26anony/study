/* test_sel_sched.c - Trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for software pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int prod = a[i] * b[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (prod > THRESHOLD) {
            /* Complex computation with inline assembly */
            int temp = prod;
            
            /* Inline assembly with multiple operands - generates complex RTL */
            asm volatile (
                "imull %1, %0\n\t"
                "addl %2, %0"
                : "+r" (temp)
                : "r" (a[i]), "r" (b[i])
                : "cc"
            );
            
            sum += temp;
        } else {
            /* Alternative path with different operations */
            sum += prod >> 1;
        }
        
        /* Additional inline assembly to create more scheduling complexity */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0"
            : "+r" (sum)
            : "r" (i)
            : "%eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Complex conditional with multiple basic blocks */
            if (mat[i][j] > 0) {
                /* Inline assembly with memory operand */
                asm volatile (
                    "imull %1, %0\n\t"
                    "addl $1, %0"
                    : "+r" (mat[i][j])
                    : "r" (factor)
                    : "cc"
                );
            } else if (mat[i][j] < 0) {
                mat[i][j] = -mat[i][j];
                
                /* More inline assembly with clobbers */
                asm volatile (
                    "xorl %%eax, %%eax\n\t"
                    "testl %0, %0\n\t"
                    "setg %%al\n\t"
                    "addl %%eax, %0"
                    : "+r" (mat[i][j])
                    :
                    : "%eax", "cc"
                );
            }
            
            /* Prevent dead code elimination */
            __asm__ __volatile__("" : : "r"(mat[i][j]) : "memory");
        }
    }
}

/* Function with pointer chasing loop - creates anti-dependencies */
int linked_list_sum(int *data, int *next, int start) {
    int sum = 0;
    int idx = start;
    int count = 0;
    
    /* Loop with pointer chasing - creates complex dependencies */
    while (idx != -1 && count < SIZE) {
        sum += data[idx];
        
        /* Conditional with inline assembly */
        if (sum > 1000000) {
            asm volatile (
                "sarl $2, %0"
                : "+r" (sum)
                :
                : "cc"
            );
        }
        
        idx = next[idx];
        count++;
        
        /* Memory barrier to prevent over-optimization */
        __asm__ __volatile__("" : : : "memory");
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int next[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        next[i] = (i + 1) % SIZE;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 200 - 100;
        }
    }
    next[SIZE-1] = -1;
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 3);
    int sum2 = linked_list_sum(a, next, 0);
    
    /* Use __builtin_assume to provide optimization hints */
    if (sum1 > 0) {
        __builtin_assume(sum1 < 1000000);
    }
    
    /* Compute final result to prevent dead code elimination */
    int final_result = sum1 + sum2;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_result);
    
    /* Additional volatile assembly to ensure code generation */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax"
        : 
        : "r" (final_result)
        : "%eax"
    );
    
    return final_result % 256;
}
