/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fno-tree-loop-if-convert -o ifcvt_test ifcvt_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure the function body remains intact for if-conversion */
__attribute__((noinline))
static int process_conditional(int init_a, int init_b, volatile int cond_init) {
    volatile int cond = cond_init;  /* Force real memory read */
    int a = init_a;
    int b = init_b;
    volatile int iterations = 100;  /* Prevent loop unrolling */
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* This is the test_expr - uses 'cond' but doesn't modify it in the then block */
        if (cond > 0) {
            /* THEN BLOCK: These instructions do NOT modify 'cond' */
            /* They only work on 'a' and 'b', keeping 'cond' unchanged */
            a = b + 1;      /* First non-label, non-debug instruction in then block */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify 'cond' here - outside the then/else blocks */
        /* This ensures the condition variable changes across iterations */
        /* but is not modified within the then block being validated */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return a value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(volatile int x, volatile int y) {
    int result = 0;
    int tmp1 = 1, tmp2 = 2;
    volatile int limit = 50;
    int j;
    
    for (j = 0; j < limit; j++) {
        /* Different condition expression using both x and y */
        if (x != y) {
            /* THEN BLOCK: Multiple instructions that don't modify x or y */
            tmp1 = tmp2 + j;
            tmp2 = tmp1 * 3;
            tmp1 = tmp1 | tmp2;
            tmp2 = tmp2 << 2;
            result += tmp1;
        } else {
            tmp1 = tmp2 - j;
            tmp2 = tmp1 / 2;
            result -= tmp2;
        }
        
        /* Modify condition variables outside the blocks */
        x = (x + j) & 0xFF;
        y = (y - j) & 0xFF;
    }
    
    return result;
}

int main(void) {
    volatile int seed = 42;  /* Force real memory read */
    int result1, result2;
    
    /* First test case */
    result1 = process_conditional(1, 2, seed);
    
    /* Second test case with different inputs */
    result2 = test_comparison(100, 200);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) != 0 ? 0 : 1;
}
