/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline to prevent inlining and preserve the conditional */
__attribute__((noinline)) 
int process_conditional(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile to force actual memory read */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression that becomes test_expr in RTL */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify 'cond' */
            /* These will be checked by the uncovered validation logic */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration - but NOT inside the then block */
        /* This ensures the then block passes the validation check */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = cond;
    }
    
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_comparison(int x, int y) {
    volatile int cmp_var = x;
    int result = 0;
    volatile int iterations = 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Different condition type: equality comparison */
        if (cmp_var == y) {
            /* Safe then block - doesn't modify cmp_var */
            result = result + (x & y);
            x = x ^ result;
            y = y | 0x1;
        } else {
            result = result - 1;
        }
        
        /* Modify condition variable outside then block */
        cmp_var = (cmp_var + i) % 100;
    }
    
    return result;
}

/* Test with pointer variables but still safe */
__attribute__((noinline))
int test_with_pointers(int val) {
    int data1 = val;
    int data2 = val * 2;
    int *ptr1 = &data1;
    int *ptr2 = &data2;
    volatile int selector = global_cond;
    
    volatile int limit = 75;
    for (int i = 0; i < limit; i++) {
        if (selector < 50) {
            /* Safe operations using pointers but not modifying selector */
            *ptr1 = *ptr2 + 1;
            *ptr2 = *ptr1 - *ptr2;
            data1 = data1 & data2;
        } else {
            *ptr2 = *ptr1 * 2;
        }
        
        selector = (selector * 3) % 100;
    }
    
    return data1 + data2;
}

int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* First test case */
    int result1 = process_conditional(rand() % 100, rand() % 100);
    printf("Result 1: %d\n", result1);
    
    /* Second test case */
    int result2 = test_comparison(rand() % 100, rand() % 100);
    printf("Result 2: %d\n", result2);
    
    /* Third test case */
    int result3 = test_with_pointers(rand() % 100);
    printf("Result 3: %d\n", result3);
    
    return 0;
}
