/* test_sel_sched.c - Trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for software pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow creating multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly with multiple operands to create complex RTL */
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
            /* Another inline assembly with clobbers */
            asm volatile (
                "imull $2, %0\n\t"
                : "+r"(temp)
                :
                : "cc"
            );
            sum += temp / 2;
        }
        
        /* Additional arithmetic to create more scheduling opportunities */
        sum = (sum * 3) / 2;
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
void process_matrix(int mat[SIZE][SIZE]) {
    int total = 0;
    
    /* Nested loops create complex control flow */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Conditional with multiple paths */
            if (mat[i][j] > 0) {
                total += mat[i][j];
                
                /* Complex inline assembly with memory operand */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "addl %%eax, %0\n\t"
                    "rorl $3, %0\n\t"
                    : "+r"(total)
                    : "m"(mat[i][j])
                    : "%eax", "cc"
                );
            } else if (mat[i][j] < 0) {
                total -= (-mat[i][j]) * 2;
                
                /* Another inline assembly with immediate and register */
                asm volatile (
                    "subl %1, %0\n\t"
                    "andl $0xFFFF, %0\n\t"
                    : "+r"(total)
                    : "r"((-mat[i][j]) * 2)
                    : "cc"
                );
            }
            
            /* Prevent dead code elimination */
            __builtin_assume(total != 0);
        }
        
        /* Loop-carried dependency */
        total = total % 1000;
    }
    
    /* Use result to prevent optimization */
    asm volatile ("" : : "r"(total));
}

/* Main function with multiple hot loops */
int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 200 - 100;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix);
    
    /* Compute verification value */
    int verification = 0;
    for (int i = 0; i < SIZE; i++) {
        verification += a[i] + b[i];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (verification: %d)\n", sum1, verification);
    
    return (sum1 > verification) ? 0 : 1;
}
