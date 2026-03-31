/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * for checking that instructions in the "then" block don't modify
 * the condition expression.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function attribute to prevent inlining and preserve CFG */
__attribute__((noinline)) 
static int process_data(int start_a, int start_b, int iterations) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile to prevent loop unrolling */
    volatile int N = iterations;
    
    /* Loop to increase chances of if-conversion analysis */
    for (volatile int i = 0; i < N; i++) {
        /* Read condition from volatile global - this creates a real conditional */
        int cond = global_cond;
        
        /* Conditional with test_expr: (cond > 0) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Line 1: modifies a, not cond */
            b = a * 2;      /* Line 2: modifies b, not cond */
            a = a ^ b;      /* Line 3: bitwise operation, not cond */
            b = b & 0xFF;   /* Line 4: masking, not cond */
            
            /* More safe operations to ensure non-empty block */
            a = a + (b >> 3);
            b = b - (a & 0xF);
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (but not inside then/else blocks) */
        /* This ensures the condition changes but doesn't affect the validation */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test function with different pattern */
__attribute__((noinline))
static int test_complex_condition(int x, int y) {
    volatile int threshold = 100;
    int result = 0;
    int temp1 = x;
    int temp2 = y;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Complex condition using multiple variables */
        int check = (temp1 * temp2) > threshold;
        
        if (check) {
            /* Safe then block - no modification of check, temp1, or temp2 */
            result = result + (temp1 & 0xF);
            result = result | (temp2 << 4);
            
            /* More arithmetic that doesn't touch check */
            temp1 = temp1 + 1;
            temp2 = temp2 - 1;
        } else {
            /* Else block with different safe operations */
            result = result ^ (temp1 | temp2);
            temp1 = temp1 >> 1;
            temp2 = temp2 << 1;
        }
        
        /* Modify threshold to change condition */
        threshold = (threshold + i) & 0xFF;
    }
    
    return result;
}

int main(void) {
    int result1, result2;
    
    /* Seed random for variability */
    srand(42);
    
    /* Test 1: Simple case */
    printf("Test 1: Simple conditional with safe then block\n");
    result1 = process_data(rand() % 100, rand() % 100, 100);
    
    /* Test 2: More complex case */
    printf("Test 2: Complex conditional\n");
    result2 = test_complex_condition(rand() % 256, rand() % 256);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Final check to use both results */
    return (result1 + result2) > 0 ? 0 : 1;
}
