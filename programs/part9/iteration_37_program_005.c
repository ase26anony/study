/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int x) {
    /* Simple operation that shouldn't trap */
    return x * 2;
}

/* Function with optimization disabled to prevent instruction merging */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    volatile int a = 10, b = 20, c = 0;
    
    /* Use goto to create a simple jump */
    if (a > 5) {
        goto target_label;
    }
    
    /* Some code that won't be executed */
    c = 100;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap and doesn't conflict with jump */
    b = a + 5;
    
    /* Use the result to prevent dead code elimination */
    printf("Test 1: a=%d, b=%d\n", a, b);
}

__attribute__((optimize("O0")))
static void test_case_2(void) {
    volatile int x = 42, y = 0;
    volatile int *ptr = &x;  /* Safe stack pointer */
    
    /* Create multiple basic blocks to encourage scheduling */
    if (x > 0) {
        goto compute;
    }
    
    y = -1;
    goto end;
    
compute:
    /* Target label for the jump */
    asm volatile("" ::: "memory");
    
    /* Memory load from safe stack location - shouldn't trap */
    y = *ptr;
    
    /* Simple arithmetic after load */
    y = y + 1;
    
end:
    printf("Test 2: x=%d, y=%d\n", x, y);
}

/* Function with inline asm to control resource usage */
__attribute__((optimize("O0")))
static void test_case_3(void) {
    register int r1 asm("eax") = 100;
    register int r2 asm("ebx") = 200;
    
    /* Force use of specific registers */
    asm volatile("" : "+r"(r1), "+r"(r2) : : "memory");
    
    if (r1 > 50) {
        goto asm_target;
    }
    
    r2 = 0;
    goto finish;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that only modifies r2, not r1 or condition codes */
    asm volatile("addl $1, %0" : "+r"(r2) : : /* no clobbers */);
    
finish:
    printf("Test 3: r1=%d, r2=%d\n", r1, r2);
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O0")))
static void test_case_4(void) {
    volatile int val = 7;
    int result;
    
    /* Multiple conditions to create interesting control flow */
    switch (val) {
        case 1: goto call_func;
        case 7: goto call_func;
        default: goto skip;
    }
    
call_func:
    /* Barrier to prevent merging */
    asm volatile("" ::: "memory");
    
    /* Function call - might be eligible for delay slot */
    result = safe_operation(val);
    
    printf("Test 4: val=%d, result=%d\n", val, result);
    return;
    
skip:
    printf("Test 4: skipped\n");
}

/* Complex test with nested control flow */
__attribute__((noinline, optimize("O1")))
static int nested_condition(int n) {
    int temp = n;
    
    /* Create multiple jumps to same label */
    if (n & 1) {
        goto process;
    }
    
    if (n > 100) {
        goto process;
    }
    
    temp = 0;
    return temp;
    
process:
    /* Candidate instruction: simple assignment */
    temp = n * 3;
    
    return temp;
}

/* Main orchestrator */
int main(void) {
    printf("Starting reorg delay slot tests...\n");
    
    /* Execute all test cases */
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    
    /* Test with return value to ensure code isn't eliminated */
    int result = nested_condition(55);
    printf("Test 5: nested_condition(55) = %d\n", result);
    
    printf("Tests completed.\n");
    return 0;
}
