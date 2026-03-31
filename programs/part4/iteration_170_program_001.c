/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fno-tree-loop-if-convert-stores -o ifcvt_test ifcvt_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve control flow */
__attribute__((noinline))
static int process_data(int iterations) {
    volatile int cond = rand() % 100;  /* Volatile to prevent constant propagation */
    int a = 0;
    int b = 1;
    int result = 0;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int limit = iterations;
    
    for (int i = 0; i < limit; i++) {
        /* Condition variable 'cond' is used in test but NOT modified in then block */
        if (cond > 50) {
            /* THEN BLOCK: These instructions do NOT modify 'cond' */
            /* They only work on 'a' and 'b' */
            a = b + 1;      /* First non-label, non-debug instruction in then block */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            /* All these should pass the modified_in_p(test_expr, insn) check */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify 'cond' outside the conditional blocks */
        /* This ensures the condition changes but isn't modified inside then/else */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        result += a + b;
    }
    
    return result;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_complex_condition(int seed) {
    volatile int test_var = seed;
    int x = 0, y = 0, z = 0;
    
    volatile int N = 1000;
    for (int i = 0; i < N; i++) {
        /* Multiple conditions in expression */
        if ((test_var & 0xF) == 0 || test_var < 100) {
            /* Safe then block - no modification of test_var */
            x = y + z;
            y = x << 2;
            z = y | 0xAA;
            x = z - y;
        } else {
            /* Else block */
            y = z + x;
            z = y >> 1;
        }
        
        /* Modify condition variable only here, outside blocks */
        test_var = (test_var + i) % 1000;
    }
    
    return x + y + z;
}

/* Test with pointer operations but still safe */
__attribute__((noinline))
static int test_with_pointers(int init) {
    volatile int condition = init;
    int data1 = 1, data2 = 2, data3 = 3;
    int *p1 = &data1;
    int *p2 = &data2;
    
    volatile int count = 500;
    for (int i = 0; i < count; i++) {
        if (condition != 0) {
            /* Safe operations using pointers, but not modifying condition */
            *p1 = *p2 + data3;
            data3 = *p1 - *p2;
            *p2 = data3 * 2;
        } else {
            data3 = *p1 + *p2;
            *p1 = data3 / 2;
        }
        
        condition = (condition * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    return data1 + data2 + data3;
}

int main() {
    srand(42);  /* Fixed seed for reproducibility */
    
    /* Run all test cases to increase coverage chances */
    int result1 = process_data(1000);
    int result2 = test_complex_condition(rand());
    int result3 = test_with_pointers(rand());
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Also test with different optimization patterns */
    volatile int quick_test = 1;
    if (quick_test) {
        int a = 0, b = 0;
        volatile int cond = 5;
        
        /* Simple straight-line test */
        if (cond > 0) {
            a = b + 1;
            b = a * 2;
        }
        printf("Quick test: %d\n", a + b);
    }
    
    return 0;
}
