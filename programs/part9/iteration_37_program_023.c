/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) static int simple_func(int x) {
    return x + 1;
}

/* Another noinline function for delay slot candidate */
__attribute__((noinline)) static int another_func(int x) {
    return x * 2;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0"))) 
static void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int *ptr = &a;
    
    /* Use goto to create a simple jump instruction */
    if (a > 5) {
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    c = a + b;
    
target_label1:
    /* Candidate instruction for delay slot - simple arithmetic */
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Simple arithmetic that doesn't trap and doesn't use complex resources */
    b = a + 1;
    
    /* Use the result to prevent dead code elimination */
    printf("Pattern1 result: %d\n", b);
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O0")))
static void test_pattern2(void) {
    volatile int x = 5, y = 0;
    
    /* Create simple jump */
    if (x != 0) {
        goto target_label2;
    }
    
    y = x * 2;  /* Unreachable but prevents optimization */
    
target_label2:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    y = simple_func(x);
    
    printf("Pattern2 result: %d\n", y);
}

/* Test with inline asm as delay slot candidate */
__attribute__((optimize("O0")))
static void test_pattern3(void) {
    register int r1 asm("eax") = 10;
    register int r2 asm("ebx") = 20;
    
    /* Simple conditional to create jump */
    if (r1 > 0) {
        goto target_label3;
    }
    
    r2 = r1 * 3;
    
target_label3:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that only modifies a general purpose register */
    /* Doesn't reference memory, doesn't set condition codes explicitly */
    asm volatile("addl $5, %0" : "+r"(r2) :: /* no clobbers */);
    
    printf("Pattern3 result: %d\n", r2);
}

/* Test with memory operation that shouldn't trap */
__attribute__((optimize("O0")))
static void test_pattern4(void) {
    volatile int stack_var1 = 100;
    volatile int stack_var2 = 200;
    volatile int result = 0;
    
    /* Stack address is guaranteed safe */
    volatile int *safe_ptr = &stack_var1;
    
    if (stack_var1 > 50) {
        goto target_label4;
    }
    
    result = stack_var1 + stack_var2;
    
target_label4:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Simple memory store to stack - shouldn't trap */
    *safe_ptr = stack_var2;
    
    printf("Pattern4 result: %d\n", *safe_ptr);
}

/* Complex pattern with nested jumps */
__attribute__((optimize("O1")))  /* Slightly higher optimization */
static void test_pattern5(void) {
    volatile int i, j, k;
    
    for (i = 0; i < 10; i++) {
        /* Multiple basic blocks to give reorg more opportunities */
        if (i % 2 == 0) {
            goto even_label;
        }
        
        j = i * 3;
        continue;
        
    even_label:
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Simple operation - good delay slot candidate */
        k = i + 1;
        
        /* Use result */
        printf("Pattern5[%d]: %d\n", i, k);
    }
}

/* Main function to run all tests */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    return 0;
}
