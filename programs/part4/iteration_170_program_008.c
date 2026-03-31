/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline attribute to prevent early optimization */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile local variable for condition to force real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but NOT in the then block */
        /* This ensures the condition changes across iterations */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to create more RTL instructions */
        a = a + i;
        b = b - i;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

int main() {
    /* Initialize with random values to create variation */
    int result = if_conversion_candidate(rand() % 100, rand() % 100);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Also test with different initial conditions */
    global_cond = 0;
    result += if_conversion_candidate(rand() % 100, rand() % 100);
    
    global_cond = -1;
    result += if_conversion_candidate(rand() % 100, rand() % 100);
    
    printf("Final result: %d\n", result);
    return result != 0;
}
