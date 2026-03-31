/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_calc(int a, int b) {
    return a + b;
}

__attribute__((noinline))
static void noop_operation(int *ptr) {
    /* Simple store that shouldn't trap */
    *ptr = 0;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    volatile int x = 0;
    volatile int y = 0;
    volatile int result = 0;
    
    /* Create a simple goto that should generate a simplejump_p */
    if (x == 0) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    y = 1;
    
target_label_1:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic that doesn't trap
       - No resource conflicts with the jump
       - Not part of a SEQUENCE
    */
    result = x + 1;
    
    /* Use result to prevent dead code elimination */
    printf("Test 1 result: %d\n", result);
}

/* Another test case with different patterns */
__attribute__((optimize("O0")))
static void test_case_2(void) {
    volatile int a = 5;
    volatile int b = 10;
    int temp = 0;
    
    /* Force a simple jump */
    if (a < 10) {
        goto compute_label;
    }
    
    /* Unreachable code to create separation */
    b = 100;
    
compute_label:
    /* Use inline asm for precise control over generated instruction.
       This adds 1 to 'a' using register operations only.
       The "cc" clobber is omitted to avoid resource conflicts. */
    asm volatile (
        "addl $1, %0"
        : "+r" (a)
        : 
        : /* No clobbers - avoids condition code conflicts */
    );
    
    /* Compiler barrier to prevent merging with following code */
    asm volatile("" ::: "memory");
    
    /* Use the result */
    printf("Test 2 a = %d\n", a);
}

/* Test with function call after label */
__attribute__((optimize("O0")))
static void test_case_3(void) {
    volatile int val1 = 7;
    volatile int val2 = 3;
    int sum = 0;
    
    /* Create jump opportunity */
    if (val1 > 0) {
        goto call_site;
    }
    
    /* Dead code */
    val2 = 99;
    
call_site:
    /* Function call as delay slot candidate.
       simple_calc is noinline, so it remains a call instruction. */
    sum = simple_calc(val1, val2);
    
    printf("Test 3 sum = %d\n", sum);
}

/* Test with memory operation that shouldn't trap */
__attribute__((optimize("O0")))
static void test_case_4(void) {
    int local_var = 42;
    int *safe_ptr = &local_var;  /* Stack address - guaranteed safe */
    
    /* Jump to label */
    if (local_var > 0) {
        goto safe_store;
    }
    
    /* Unreachable */
    local_var = 0;
    
safe_store:
    /* Memory store to stack variable - shouldn't trap */
    *safe_ptr = 99;
    
    /* Use the value */
    printf("Test 4 local_var = %d\n", local_var);
}

/* Complex test mixing different patterns */
__attribute__((optimize("O1")))  /* Slightly higher optimization */
static void test_case_5(void) {
    register int r1 asm("eax") = 1;
    register int r2 asm("ebx") = 2;
    int output = 0;
    
    /* Multiple jumps to create scheduling opportunities */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto process;
        }
        
        /* Some computation */
        r1 += i;
        
        continue;
        
    process:
        /* Inline asm with specific register usage.
           This instruction only modifies r2, not r1 used in the condition. */
        asm volatile (
            "addl $5, %0"
            : "+r" (r2)
            :
            : /* No clobbers */
        );
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    output = r1 + r2;
    printf("Test 5 output = %d\n", output);
}

/* Main orchestrator */
int main(void) {
    printf("Running delay slot filling tests...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    printf("All tests completed.\n");
    return 0;
}
