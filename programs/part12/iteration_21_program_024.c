/* test_sel_sched.c - Trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - encourages pipelining */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int prod = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
        if (prod > THRESHOLD) {
            sum += prod;
            
            /* Inline assembly to create complex RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:"
                : "+r"(sum)
                : "r"(prod)
                : "cc"
            );
        } else {
            /* Alternative path with different computation */
            sum += prod / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "shrl $1, %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(sum)
                : "r"(prod)
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 8 == 0) {
            /* Memory barrier-like asm to prevent reordering */
            asm volatile ("" ::: "memory");
        }
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
void process_matrix(int mat[SIZE][SIZE]) {
    int temp;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Complex addressing pattern */
            temp = mat[i][j];
            
            /* Conditional with multiple operations */
            if (temp > 0) {
                temp = temp * 3 + 1;
                
                /* Inline asm with multiple outputs */
                asm volatile (
                    "imull $3, %0\n\t"
                    "addl $1, %0"
                    : "+r"(temp)
                    :: "cc"
                );
            } else {
                temp = -temp;
                
                /* Asm with explicit register constraints */
                register int r asm("ebx") = temp;
                asm volatile (
                    "negl %0"
                    : "+r"(r)
                );
                temp = r;
            }
            
            /* Store back with potential aliasing */
            mat[i][j] = temp;
        }
    }
}

/* Helper to prevent dead code elimination */
volatile int sink;

int main() {
    /* Initialize arrays with known patterns */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 512 - 256;
        }
    }
    
    /* Use __builtin_assume to provide loop bound hints */
    if (SIZE > 0) {
        __builtin_assume(SIZE <= 1024);
    }
    
    /* Compute results */
    int result1 = compute_sum(a, b, SIZE);
    process_matrix(matrix);
    
    /* Use results to prevent optimization */
    sink = result1 + matrix[0][0];
    
    /* Print result for verification */
    printf("Result: %d\n", result1);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    return 0;
}
