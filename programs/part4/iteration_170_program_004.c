/* Program to trigger if-conversion validation logic in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int test_if_conversion(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile condition variable - prevents constant folding */
    volatile int cond = global_cond;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Simple arithmetic on different variables */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* More bitwise operations */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way (outside the then block) */
        /* This ensures the condition changes but isn't modified in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to prevent optimization */
        a = a + i;
        b = b - i;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_if_conversion2(int x, int y) {
    volatile int flag = global_cond;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (flag != 0) {
            /* THEN BLOCK: Multiple instructions that don't modify flag */
            x = y * 3;
            y = x + 7;
            result = x & y;
            x = x | 0x0F;
        } else {
            y = x - 5;
            x = y * 2;
        }
        
        /* Modify condition variable outside then block */
        flag = flag ^ (i + 1);
    }
    
    return result + x + y;
}

/* Main function to drive the tests */
int main() {
    int result1, result2;
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* First test */
    global_cond = rand() % 100;
    result1 = test_if_conversion(10, 20);
    
    /* Second test */
    global_cond = rand() % 100;
    result2 = test_if_conversion2(30, 40);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}
