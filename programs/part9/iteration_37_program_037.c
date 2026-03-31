/* Test program for GCC reorg.cc delay slot filling optimization
 * Targeting lines 2135-2149 in reorg.cc
 * Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline, optimize("O0")))
static int simple_operation(int x) {
    /* Simple arithmetic that won't trap */
    return x + 1;
}

/* Another non-inlineable function */
__attribute__((noinline))
static int another_op(int x) {
    return x * 2;
}

/* Function with a simple jump to label with eligible delay slot candidate */
__attribute__((noinline, optimize("O2")))
static int test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 30;
    int result = 0;
    
    /* Create control flow that might generate a simple jump */
    if (a > 5) {
        /* This goto should generate a simplejump_p instruction */
        goto target_label;
    }
    
    /* Some code that won't be reached but prevents optimization */
    b = b + c;
    
target_label:
    /* Candidate for delay slot filling:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - Doesn't set resources conflicting with the jump
       - Not part of a SEQUENCE
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    result = simple_operation(a);
    
    /* Use result to prevent dead code elimination */
    return result + b;
}

/* Test case with inline asm as delay slot candidate */
__attribute__((noinline, optimize("O2")))
static int test_case_2(void) {
    int x = 42;
    int y = 0;
    
    /* Force a simple jump */
    if (x > 0) {
        goto asm_target;
    }
    
    y = x * 2;  /* Unreachable but prevents optimization */
    
asm_target:
    /* Inline asm candidate for delay slot:
       - Register-only operation (no memory access)
       - Modifies only general purpose register
       - No condition code clobber to avoid resource conflict
    */
    asm volatile("" ::: "memory");  /* Prevent merging with label */
    
    /* Simple arithmetic in asm - won't trap, no memory access */
    asm volatile("add %1, %0" 
                 : "+r" (y) 
                 : "r" (x)
                 : /* no clobbers */);
    
    return y;
}

/* Test with memory operation that shouldn't trap (stack variable) */
__attribute__((noinline, optimize("O2")))
static int test_case_3(void) {
    volatile int stack_var1 = 100;
    volatile int stack_var2 = 200;
    int temp = 0;
    
    /* Multiple conditions to create jump opportunity */
    if (stack_var1 > 50) {
        if (stack_var2 < 300) {
            goto mem_target;
        }
    }
    
    temp = stack_var1 + stack_var2;
    
mem_target:
    /* Memory operation on stack variable - should be safe */
    asm volatile("" ::: "memory");
    
    /* Load from stack - should not fault */
    int loaded = stack_var1;
    
    /* Simple operation on loaded value */
    return loaded + 5;
}

/* Test with function call as delay slot candidate */
__attribute__((noinline, optimize("O2")))
static int test_case_4(void) {
    int val = 123;
    int ret = 0;
    
    /* Create jump */
    if (val != 0) {
        goto call_target;
    }
    
    ret = val + 456;
    
call_target:
    /* Function call as candidate - must not be inlinable */
    asm volatile("" ::: "memory");
    ret = another_op(val);
    
    return ret;
}

/* Complex test with multiple jumps and labels */
__attribute__((noinline, optimize("O1")))
static int test_case_5(void) {
    volatile int counter = 0;
    volatile int accumulator = 0;
    
    for (int i = 0; i < 10; i++) {
        counter++;
        
        /* Create opportunity for simple jump */
        if (counter % 2 == 0) {
            goto loop_target;
        }
        
        accumulator += i;
        continue;
        
    loop_target:
        /* Candidate instruction in loop */
        asm volatile("" ::: "memory");
        accumulator += counter * 2;
    }
    
    return accumulator;
}

/* Main function to run all test cases */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_case_1();
    printf("Test 1 result: %d\n", results[0]);
    
    results[1] = test_case_2();
    printf("Test 2 result: %d\n", results[1]);
    
    results[2] = test_case_3();
    printf("Test 3 result: %d\n", results[2]);
    
    results[3] = test_case_4();
    printf("Test 4 result: %d\n", results[3]);
    
    results[4] = test_case_5();
    printf("Test 5 result: %d\n", results[4]);
    
    /* Verify results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    
    printf("Total sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
