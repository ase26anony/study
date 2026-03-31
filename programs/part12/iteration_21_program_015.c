/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex computation with inline assembly */
            int adjusted;
            asm volatile (
                "imull %1, %0\n\t"           /* Multiply */
                "addl %2, %0\n\t"           /* Add constant */
                : "=r"(adjusted)
                : "r"(temp), "i"(5)
                : "cc"
            );
            sum += adjusted;
        } else {
            /* Another inline assembly with different pattern */
            int reduced;
            asm volatile (
                "subl %1, %0\n\t"
                "andl $0xFF, %0\n\t"
                : "=r"(reduced)
                : "r"(temp)
                : "cc"
            );
            sum += reduced;
        }
        
        /* Additional inline assembly with multiple clobbers */
        asm volatile (
            "addl $1, %0\n\t"
            : "+r"(sum)
            :
            : "cc", "memory"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling complexity */
int matrix_multiply(int n) {
    int result = 0;
    
    /* Nested loops create complex scheduling regions */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = i * j;
            
            /* Switch statement for additional control flow */
            switch (val % 4) {
                case 0:
                    asm volatile ("addl $1, %0" : "+r"(result) :: "cc");
                    break;
                case 1:
                    asm volatile ("subl $1, %0" : "+r"(result) :: "cc");
                    break;
                case 2:
                    asm volatile ("imull $2, %0" : "+r"(result) :: "cc");
                    break;
                default:
                    asm volatile ("andl $0x0F, %0" : "+r"(result) :: "cc");
            }
        }
    }
    
    return result;
}

/* Main function with predictable computation */
int main() {
    int a[SIZE], b[SIZE];
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i % 256;
        b[i] = (i * 3) % 256;
    }
    
    /* Call functions to ensure they're not optimized away */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(32);
    
    /* Use __builtin_assume to provide optimization hints */
    if (sum1 > 0) {
        __builtin_assume(sum1 > 0);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", sum1);
    printf("Result 2: %d\n", sum2);
    
    /* Verify computation is correct */
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected_sum += (temp * 5) + 1;
        } else {
            expected_sum += (temp & 0xFF) + 1;
        }
    }
    
    printf("Expected: %d\n", expected_sum);
    printf("Verification: %s\n", (sum1 == expected_sum) ? "PASS" : "FAIL");
    
    return 0;
}
