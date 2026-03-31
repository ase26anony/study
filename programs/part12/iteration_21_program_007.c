/* test_sel_sched.c - Test program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from software pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (a[i] > THRESHOLD) {
            /* Complex computation with inline assembly to generate non-trivial RTL */
            int x = temp;
            int y = b[i];
            
            /* Inline assembly with multiple operands and clobbers */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $0, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %0"
                : "+r"(x)
                : "r"(y)
                : "al", "cc"
            );
            
            sum += x;
        } else {
            sum += temp;
        }
        
        /* Additional inline assembly to create more scheduling opportunities */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "addl %%eax, %0"
            : "+r"(sum)
            : "r"(a[i]), "r"(b[i])
            : "eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more complex scheduling regions */
void process_matrix(int matrix[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access pattern */
        for (int j = 0; j < cols; j += 2) {
            int val = matrix[i][j];
            
            /* Conditional with side effects */
            if (val > 0) {
                /* More inline assembly with memory operand */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "shrl $2, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(row_sum)
                    : "m"(matrix[i][j+1])
                    : "eax", "cc"
                );
            }
            
            /* Another arithmetic operation */
            row_sum += val * 3;
        }
        
        /* Store result back with barrier */
        asm volatile ("" ::: "memory");
        matrix[i][0] = row_sum;
    }
}

/* Helper function with switch statement for control flow variety */
int process_with_switch(int x) {
    int result = 0;
    
    switch (x % 4) {
        case 0:
            asm volatile ("movl $1, %0" : "=r"(result));
            break;
        case 1:
            asm volatile ("movl $2, %0" : "=r"(result));
            break;
        case 2:
            asm volatile ("movl $3, %0" : "=r"(result));
            break;
        case 3:
            asm volatile ("movl $4, %0" : "=r"(result));
            break;
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 5) % 256;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * j) % 128;
        }
    }
    
    /* Call functions to ensure they're not optimized away */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 8, 16);
    int sum2 = process_with_switch(sum1 % 100);
    
    /* Use __builtin_assume to provide optimization hints */
    if (sum1 > 0) {
        __builtin_assume(sum1 > 0);
    }
    
    /* Compute final result with mixed operations */
    int final_result = sum1 + sum2 + matrix[0][0];
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Additional loop with known trip count for better scheduling */
    int known_sum = 0;
    for (int i = 0; i < 1000; i++) {
        known_sum += i * i;
        
        /* Insert memory barrier to prevent reordering */
        asm volatile ("" ::: "memory");
        
        /* Conditional with both paths having side effects */
        if (i % 7 == 0) {
            asm volatile (
                "movl %1, %%eax\n\t"
                "negl %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(known_sum)
                : "r"(i)
                : "eax", "cc"
            );
        }
    }
    
    printf("Known sum: %d\n", known_sum);
    
    return final_result > 0 ? 0 : 1;
}
