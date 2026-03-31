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
    
    /* Use volatile for condition variable to force real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These should pass the validation in lines 577-583 */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but outside the then block */
        /* This ensures cond changes but isn't modified in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(a), "r"(b) : "memory");
    }
    
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_test(int x, int y) {
    volatile int test_var = x;
    int result = y;
    
    /* Different condition check */
    if (test_var != 0) {
        /* Multiple safe operations in then block */
        result = result + (x & y);
        result = result | 0x1;
        result = result << 2;
        y = x ^ result;
        result = y - x;
    } else {
        result = x | y;
    }
    
    /* Ensure test_var is used after to prevent elimination */
    return result + test_var;
}

int main() {
    /* Initialize with random values to create varying conditions */
    int seed = 42;
    srand(seed);
    
    /* Call the if-conversion candidate multiple times */
    int total = 0;
    for (int j = 0; j < 10; j++) {
        global_cond = rand() % 100;
        total += if_conversion_candidate(rand() % 100, rand() % 100);
        total += another_test(rand() % 100, rand() % 100);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
