/* test_sel_sched.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - encourages pipelining */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
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
        if (i % 16 == 0) {
            /* Nested conditional with memory access */
            int extra = a[i] + b[i];
            asm volatile (
                "movl %1, %%eax\n\t"
                "subl $1, %%eax\n\t"
                "addl %%eax, %0"
                : "+r" (sum)
                : "r" (extra)
                : "%eax", "cc"
            );
        }
    }
    
    return sum;
}

/* Second function with different loop pattern */
int compute_product_sum(int *a, int *b, int *c, int n) {
    int result = 0;
    
    /* Loop with carried dependency chain */
    for (int i = 0; i < n - 1; i++) {
        int prod = a[i] * b[i];
        int next_prod = a[i + 1] * b[i + 1];
        
        /* Complex expression with multiple operations */
        result += prod + next_prod + c[i];
        
        /* Conditional with side effect */
        if (prod > next_prod) {
            /* Inline assembly with multiple clobbers */
            asm volatile (
                "cmpl %1, %0\n\t"
                "jle 1f\n\t"
                "subl $5, %0\n"
                "1:\n\t"
                : "+r" (result)
                : "r" (prod)
                : "cc"
            );
        }
    }
    
    return result;
}

/* Helper function to prevent dead code elimination */
__attribute__((noinline)) 
void use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE], c[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 113;
        c[i] = (i * 11) % 151;
    }
    
    /* Force compiler to assume loop bounds */
    int n = SIZE;
    __builtin_assume(n > 0);
    __builtin_assume(n <= SIZE);
    
    /* Call computation functions */
    int sum1 = compute_sum(a, b, n);
    int sum2 = compute_product_sum(a, b, c, n);
    
    /* Use results to prevent optimization */
    use_result(sum1);
    use_result(sum2);
    
    /* Print verification result */
    printf("Result1: %d, Result2: %d\n", sum1, sum2);
    
    return 0;
}
