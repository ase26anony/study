/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

__attribute__((noinline))
static int another_op(int x) {
    return x * 2;
}

/* Use optimize attribute to control optimization level within functions */
__attribute__((optimize("O2")))
static void test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 30;
    
    /* Create a simple jump to a label */
    if (a > 5) {
        goto target_label_1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    b = b + c;
    
target_label_1:
    /* Candidate instruction for delay slot filling */
    /* Simple arithmetic that doesn't trap and doesn't conflict with jump */
    /* Compiler barrier prevents merging with label */
    asm volatile("" ::: "memory");
    
    /* This should be a good candidate for delay slot filling */
    /* It's a simple non-jump instruction that doesn't trap */
    c = a + b;
    
    /* Use the result to prevent dead code elimination */
    printf("Test 1 result: %d\n", c);
}

__attribute__((optimize("O2")))
static void test_case_2(void) {
    volatile int x = 100;
    volatile int y = 200;
    volatile int z = 300;
    
    /* Force a simple jump */
    if (x != 0) {
        goto compute_target;
    }
    
    /* Dead code to create separation */
    y = y * 2;
    
compute_target:
    /* Compiler barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Good delay slot candidate: register-only operation */
    /* Using asm to ensure specific instruction pattern */
    int temp = y;
    asm volatile("addl $1, %0" : "+r"(temp) :: "cc");
    z = temp;
    
    /* Use result */
    printf("Test 2 result: %d\n", z);
}

__attribute__((optimize("O2")))
static void test_case_3(void) {
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Multiple jumps to same label to create optimization opportunities */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            goto process_even;
        } else {
            counter++;
            continue;
        }
        
    process_even:
        /* Barrier to prevent sequence */
        asm volatile("" ::: "memory");
        
        /* Simple non-trapping operation */
        result = simple_operation(counter);
        
        counter = result;
    }
    
    printf("Test 3 result: %d\n", result);
}

__attribute__((optimize("O1")))  /* Lower optimization to preserve jumps */
static void test_case_4(void) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    
    /* Nested jumps to create complex flow */
    if (a > 0) {
        if (b > 1) {
            goto label_outer;
        }
    }
    
    c = 999;
    return;
    
label_outer:
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Safe memory access (stack variable) */
    d = a + b;
    
    /* Another jump to create more opportunities */
    if (d > 0) {
        goto label_inner;
    }
    
label_inner:
    asm volatile("" ::: "memory");
    
    /* Another candidate instruction */
    c = another_op(d);
    
    printf("Test 4 result: %d %d\n", d, c);
}

/* Test with function call as candidate */
__attribute__((optimize("O2")))
static void test_case_5(void) {
    volatile int base = 42;
    int computed;
    
    /* Simple conditional to force jump generation */
    if (base != 0) {
        goto call_site;
    }
    
    /* Unreachable but prevents optimization */
    base = 0;
    
call_site:
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    /* Function is noinline, so it remains as call instruction */
    computed = simple_operation(base);
    
    printf("Test 5 result: %d\n", computed);
}

/* Main orchestrator */
int main(void) {
    printf("Starting delay slot filling tests...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    printf("All tests completed.\n");
    return 0;
}
