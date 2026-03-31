/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int result;
    
    /* Use goto to create a simple jump */
    if (a < b) {
        goto target_label;
    }
    
    /* Dead code that won't be executed but prevents optimization */
    result = a * b;
    
target_label:
    /* Candidate instruction for delay slot filling */
    /* Simple arithmetic that doesn't trap and doesn't create a sequence */
    asm volatile ("" : "+r" (a) : : "memory"); /* Compiler barrier */
    result = a + 5;  /* Simple arithmetic - good candidate */
    
    /* Use result to prevent dead code elimination */
    printf("Test 1 result: %d\n", result);
}

/* Another test case with function call after label */
__attribute__((optimize("O0")))
static void test_case_2(void) {
    int x = 42;
    int y;
    
    /* Create simple jump */
    if (x > 0) {
        goto compute;
    }
    
    y = x - 10;
    
compute:
    /* Function call as delay slot candidate */
    /* The function is noinline, so it remains as a call instruction */
    asm volatile ("" ::: "memory"); /* Prevent merging */
    y = simple_operation(x);
    
    printf("Test 2 result: %d\n", y);
}

/* Test with memory operation that shouldn't trap */
__attribute__((optimize("O0")))
static void test_case_3(void) {
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int index = 5;
    int value;
    
    /* Simple jump */
    if (index < 10) {
        goto load_value;
    }
    
    value = 0;
    
load_value:
    /* Memory load from stack - should not trap */
    asm volatile ("" ::: "memory");
    value = array[index];  /* Safe array access */
    
    printf("Test 3 result: %d\n", value);
}

/* Test with inline asm that has specific register usage */
__attribute__((optimize("O0")))
static void test_case_4(void) {
    register int r1 asm("t0") = 100;
    register int r2 asm("t1") = 200;
    
    /* Create jump */
    if (r1 < r2) {
        goto asm_op;
    }
    
    r2 = 0;
    
asm_op:
    /* Inline asm that modifies only specific registers */
    /* Using temporary registers that shouldn't conflict with jump resources */
    asm volatile (
        "add %0, %0, %1\n\t"
        : "+r" (r1)
        : "r" (r2)
        : /* no clobbers - avoids cc which might conflict */
    );
    
    printf("Test 4 result: %d\n", r1);
}

/* Nested jumps to create more complex patterns */
__attribute__((optimize("O0")))
static void test_case_5(void) {
    volatile int counter = 0;
    volatile int limit = 3;
    
outer_loop:
    if (counter >= limit) {
        goto done;
    }
    
    counter++;
    
inner_label:
    /* Candidate instruction - simple increment */
    asm volatile ("" ::: "memory");
    counter = counter + 1;
    
    goto outer_loop;
    
done:
    printf("Test 5 result: %d\n", counter);
}

/* Main function to run all test cases */
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
