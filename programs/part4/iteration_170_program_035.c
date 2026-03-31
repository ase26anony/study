/* Test program for GCC if-conversion pass coverage */
/* Target: ifcvt.cc lines 577-583 - validation that "then" block doesn't modify condition */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve control flow */
__attribute__((noinline)) 
static int process_conditional(int init_a, int init_b, volatile int cond_init) {
    int a = init_a;
    int b = init_b;
    volatile int cond = cond_init;  /* volatile to prevent constant propagation */
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    int i;
    
    for (i = 0; i < N; i++) {
        /* Condition expression using 'cond' - this is the test_expr */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure multiple instructions in block */
            a = a + (b << 2);
            b = b - (a >> 1);
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify 'cond' for next iteration - ensures loop variant condition */
        /* This modification is OUTSIDE the then/else blocks */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(a), "r"(b), "r"(cond));
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(volatile int x, volatile int y) {
    int result = 0;
    int temp1 = 1, temp2 = 2;
    
    /* Different condition form */
    if (x != y) {
        /* Safe then block - no modification of x or y */
        temp1 = temp2 * 3;
        temp2 = temp1 + 5;
        temp1 = temp2 - temp1;
        result = temp1 | temp2;
    } else {
        temp1 = temp2 / 2;
        result = temp1;
    }
    
    return result;
}

/* Test with pointer variables but still safe */
__attribute__((noinline))
static int test_with_pointers(volatile int *ptr1, volatile int *ptr2) {
    int a = 10, b = 20;
    
    /* Condition uses pointer dereference */
    if (*ptr1 > *ptr2) {
        /* Safe: modifies local variables, not the condition variables */
        a = b + *ptr1;    /* Reads but doesn't modify *ptr1 */
        b = a - *ptr2;    /* Reads but doesn't modify *ptr2 */
        a = a * b;
        b = b / 2;
    }
    
    return a + b;
}

int main(void) {
    volatile int seed = 42;
    int result = 0;
    
    /* Test 1: Basic conditional with safe then block */
    result += process_conditional(1, 2, seed);
    
    /* Test 2: Comparison condition */
    volatile int x = 10, y = 20;
    result += test_comparison(x, y);
    
    /* Test 3: Pointer-based condition */
    volatile int p1 = 30, p2 = 15;
    result += test_with_pointers(&p1, &p2);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
