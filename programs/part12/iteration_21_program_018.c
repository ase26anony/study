/* test_sel_sched.c - Trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with data-dependent loop that benefits from software pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Tight loop with carried dependency - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with conditional */
        int prod = a[i] * b[i];
        
        /* Conditional creates multiple basic blocks in scheduling region */
        if (prod > THRESHOLD) {
            sum += prod;
        } else {
            /* Use inline assembly to create complex RTL patterns */
            int temp = prod;
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $50, %0\n\t"
                "jle 1f\n\t"
                "subl $10, %0\n\t"
                "1:\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        }
        
        /* Additional arithmetic to create more scheduling opportunities */
        sum = (sum * 3) / 2;
    }
    
    return sum;
}

/* Another function with nested loops for more scheduling complexity */
void process_matrix(int mat[SIZE][SIZE]) {
    int total = 0;
    
    /* Nested loops create larger scheduling regions */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Complex conditional with multiple paths */
            if (mat[i][j] > 0) {
                /* Inline asm with multiple clobbers */
                int val = mat[i][j];
                asm volatile (
                    "imull $7, %0\n\t"
                    "addl %%eax, %1\n\t"
                    : "+r"(val), "+r"(total)
                    : 
                    : "eax", "cc"
                );
                mat[i][j] = val;
            } else if (mat[i][j] < 0) {
                /* Different asm pattern */
                asm volatile (
                    "negl %0\n\t"
                    "addl %0, %1\n\t"
                    : "+r"(mat[i][j]), "+r"(total)
                    : 
                    : "cc"
                );
            }
            
            /* Prevent dead code elimination */
            __builtin_assume(mat[i][j] != 0);
        }
    }
    
    /* Use result to prevent optimization */
    volatile int dummy = total;
}

/* Main function with multiple optimization targets */
int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with pattern that creates data dependencies */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 200 - 100;  /* Range: -100 to 99 */
        }
    }
    
    /* Force loop count to be known at compile time */
    __builtin_assume(SIZE == 1024);
    
    /* Call functions that should trigger selective scheduling */
    int result1 = compute_sum(a, b, SIZE);
    process_matrix(matrix);
    
    /* Compute verification result */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += a[i] * b[i];
    }
    
    /* Print results to ensure execution */
    printf("Result1: %d\n", result1);
    printf("Verify: %d\n", verify);
    
    /* Return deterministic result for test verification */
    return (result1 > 0 && verify > 0) ? 0 : 1;
}
