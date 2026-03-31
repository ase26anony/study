/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile condition to force real branch generation */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression (test_expr) - uses cond but doesn't modify it */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            /* All these instructions modify a and b, NOT cond */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but outside the then block */
        /* This ensures cond changes across iterations but isn't modified in then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(a), "r"(b) : "memory");
    }
    
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_test(int x, int y) {
    volatile int flag = global_cond & 1;
    int result = 0;
    
    volatile int iterations = 50;
    for (int i = 0; i < iterations; i++) {
        /* Different condition expression */
        if (flag == 0) {
            /* Safe then block - no modification of flag */
            x = y << 2;
            y = x | 0x0F;
            result += x - y;
        } else {
            y = x >> 1;
            result -= y;
        }
        
        /* Change flag outside the then block */
        flag = (flag + i) & 1;
        
        /* Memory barrier */
        asm volatile("" : : "r"(x), "r"(y) : "memory");
    }
    
    return result;
}

int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Test first function */
    int result1 = if_conversion_candidate(rand() % 100, rand() % 100);
    
    /* Test second function */
    int result2 = another_test(rand() % 100, rand() % 100);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Also test with different global_cond values */
    global_cond = 0;
    result1 = if_conversion_candidate(rand() % 100, rand() % 100);
    
    global_cond = -1;
    result2 = another_test(rand() % 100, rand() % 100);
    
    printf("More results: %d, %d\n", result1, result2);
    
    return (result1 + result2) != 0 ? 0 : 1;
}
