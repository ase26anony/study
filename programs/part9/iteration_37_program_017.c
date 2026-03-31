/* Test program for reorg.cc delay slot filling coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_calc(int a, int b) {
    return a + b;
}

/* Another non-inlineable function */
__attribute__((noinline, optimize("O0")))
static void dummy_operation(int *ptr) {
    *ptr += 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static int test_case_1(void) {
    volatile int a = 10;
    volatile int b = 20;
    int result = 0;
    
    /* Use goto to create a simple jump */
    if (a < b) {
        goto target_label;
    }
    
    /* This should never execute */
    result = -1;
    return result;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - No resource conflicts with jump
       - Not a SEQUENCE pattern
    */
    result = a + b;
    
    /* Use result to prevent dead code elimination */
    return result;
}

/* Test with function call as candidate */
__attribute__((optimize("O0")))
static int test_case_2(void) {
    volatile int x = 5;
    volatile int y = 3;
    int res = 0;
    
    /* Force a simple jump */
    if (x > 0) {
        goto call_target;
    }
    
    res = -1;
    return res;
    
call_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    res = simple_calc(x, y);
    
    return res;
}

/* Test with inline asm as candidate */
__attribute__((optimize("O0")))
static int test_case_3(void) {
    volatile int counter = 0;
    volatile int temp = 0;
    
    /* Create simple jump */
    if (counter == 0) {
        goto asm_target;
    }
    
    temp = -1;
    return temp;
    
asm_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Inline asm that:
       - Modifies only a specific register
       - Doesn't reference memory (no "m" constraint)
       - Doesn't set condition codes (no "cc" clobber)
       - Simple operation that won't trap
    */
    asm volatile(
        "addl $1, %0\n\t"
        : "+r"(temp)
        :
        : /* empty clobber list - no cc */
    );
    
    return temp;
}

/* Test with memory operation (stack variable - safe) */
__attribute__((optimize("O0")))
static int test_case_4(void) {
    volatile int array[4] = {1, 2, 3, 4};
    volatile int idx = 0;
    int sum = 0;
    
    /* Simple jump */
    if (idx >= 0) {
        goto mem_target;
    }
    
    sum = -1;
    return sum;
    
mem_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Memory access to stack variable - should not fault */
    sum = array[idx];
    
    return sum;
}

/* Test with multiple basic blocks to encourage reorg */
__attribute__((optimize("O1")))  /* O1 for some optimization but not too much */
static int test_case_5(void) {
    volatile int a = 100;
    volatile int b = 200;
    volatile int c = 300;
    int result = 0;
    
    /* More complex control flow that might create multiple
       basic blocks for reorg to work with */
    if (a > 50) {
        if (b < 250) {
            goto complex_target;
        } else {
            result = a + c;
        }
    } else {
        result = b - c;
    }
    
    return result;
    
complex_target:
    /* Multiple instructions after label */
    asm volatile("" ::: "memory");
    
    /* Several simple instructions */
    result = a + b;
    result *= 2;
    result -= c;
    
    return result;
}

/* Main orchestrator */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute all test cases */
    results[0] = test_case_1();
    results[1] = test_case_2();
    results[2] = test_case_3();
    results[3] = test_case_4();
    results[4] = test_case_5();
    
    /* Use results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
        printf("Test %d: %d\n", i + 1, results[i]);
    }
    
    printf("Total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
