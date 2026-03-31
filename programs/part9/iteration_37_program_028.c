/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) static int simple_operation(int x) {
    return x + 1;
}

__attribute__((noinline)) static int another_op(int x) {
    return x * 2;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0"))) 
static int test_pattern1(void) {
    volatile int a = 5, b = 10, c = 0;
    volatile int result = 0;
    
    /* Create a simple jump scenario */
    if (a < b) {
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a * b;
    
target_label1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap and doesn't conflict with jump */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    c = a + b;  /* Simple integer addition - won't trap */
    asm volatile("" ::: "memory");  /* Another barrier */
    
    /* Use the result to prevent dead code elimination */
    result += c;
    
    return result;
}

/* Another test pattern with function call after label */
__attribute__((optimize("O0")))
static int test_pattern2(void) {
    volatile int x = 42;
    volatile int y = 0;
    
    /* Force a goto to create simple jump */
    if (x > 0) {
        goto compute;
    }
    
    y = x - 1;
    
compute:
    /* Function call as delay slot candidate */
    asm volatile("" ::: "memory");
    y = simple_operation(x);  /* Function call - if not inlined, may be eligible */
    asm volatile("" ::: "memory");
    
    return y;
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
static int test_pattern3(void) {
    volatile int var1 = 100, var2 = 200;
    volatile int sum = 0;
    
    /* Create simple jump */
    if (var1 != 0) {
        goto do_sum;
    }
    
    var2 = var1 + 50;
    
do_sum:
    /* asm statement that modifies a register but doesn't set CC or memory */
    asm volatile("" ::: "memory");
    /* Simple arithmetic in asm - only modifies general purpose register */
    asm volatile("addl %1, %0" : "+r"(sum) : "r"(var1) : /* no clobber */);
    asm volatile("" ::: "memory");
    
    /* Use result */
    sum += var2;
    
    return sum;
}

/* More complex pattern with multiple basic blocks */
__attribute__((optimize("O1")))  /* Slightly higher optimization */
static int test_pattern4(void) {
    volatile int i, j, k;
    volatile int array[4] = {1, 2, 3, 4};
    volatile int total = 0;
    
    /* Loop with goto to create jump to label */
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            goto special_case;
        }
        total += i;
    }
    
    /* This should not execute in normal flow */
    k = total * 2;
    
special_case:
    /* Good delay slot candidate: simple memory load from stack variable */
    asm volatile("" ::: "memory");
    j = array[0];  /* Safe stack access - won't fault */
    asm volatile("" ::: "memory");
    
    total += j;
    
    return total;
}

/* Test with conditional that gets optimized to simple jump */
__attribute__((optimize("O2")))
static int test_pattern5(void) {
    volatile int flag = 1;
    volatile int a = 10, b = 20;
    volatile int res = 0;
    
    /* This should become a simple jump after optimization */
    if (flag) {
        goto process;
    }
    
    /* Dead code that might be optimized away */
    res = a - b;
    
process:
    /* Candidate: simple bit operation */
    asm volatile("" ::: "memory");
    res = a & 0xFF;  /* Simple AND - no trapping */
    asm volatile("" ::: "memory");
    
    /* Make result depend on b to prevent optimization */
    res += b;
    
    return res;
}

/* Main orchestrator */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_pattern1();
    printf("Pattern 1 result: %d\n", results[0]);
    
    results[1] = test_pattern2();
    printf("Pattern 2 result: %d\n", results[1]);
    
    results[2] = test_pattern3();
    printf("Pattern 3 result: %d\n", results[2]);
    
    results[3] = test_pattern4();
    printf("Pattern 4 result: %d\n", results[3]);
    
    results[4] = test_pattern5();
    printf("Pattern 5 result: %d\n", results[4]);
    
    /* Use results to prevent dead code elimination */
    int final = 0;
    for (int i = 0; i < 5; i++) {
        final += results[i];
    }
    
    printf("Final sum: %d\n", final);
    
    return final > 0 ? 0 : 1;
}
