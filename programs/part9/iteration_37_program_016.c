/* test_reorg.c - Target uncovered lines 2135-2149 in reorg.cc */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to maintain function boundaries */
__attribute__((noinline, optimize("O0")))
static void simple_arithmetic(int *a, int *b) {
    /* Simple arithmetic that won't trap */
    *a = *b + 1;
}

/* Function with a simple jump pattern */
__attribute__((noinline, optimize("O0")))
static int test_jump_to_label(void) {
    volatile int x = 0, y = 0, z = 0;
    int result = 0;
    
    /* Create a simple goto that should generate a simplejump_p */
    if (x == 0) {
        goto target_label;
    }
    
    /* Unreachable code to make the jump necessary */
    result = 100;
    
target_label:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - No memory references that could fault
       - No resource conflicts with jump
    */
    y = z + 5;
    
    /* Use result to prevent dead code elimination */
    result += y;
    return result;
}

/* Function with asm statement as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int test_asm_candidate(void) {
    volatile int a = 1, b = 2;
    int result = 0;
    
    /* Force a simple jump */
    if (a < b) {
        goto asm_target;
    }
    
    result = 50;
    
asm_target:
    /* asm statement as delay slot candidate:
       - Only modifies general purpose register (eax)
       - No memory access
       - No condition code clobber to avoid conflict
       - Simple operation that won't trap
    */
    asm volatile (
        "addl $7, %0"
        : "+r"(a)
        : /* no inputs */
        /* No clobbers to avoid resource conflicts */
    );
    
    /* Compiler barrier to prevent merging */
    asm volatile("" ::: "memory");
    
    result += a;
    return result;
}

/* Function with function call as delay slot candidate */
__attribute__((noinline, optimize("O1")))
static int test_function_call(void) {
    volatile int val1 = 10, val2 = 20;
    int result = 0;
    
    /* Multiple basic blocks to encourage scheduling */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto call_target;
        }
        result += i;
    }
    
    result += 100;
    
call_target:
    /* Function call as candidate - must not be inlined */
    simple_arithmetic(&val1, &val2);
    
    result += val1;
    return result;
}

/* Complex pattern with multiple jumps */
__attribute__((noinline, optimize("O2")))
static int test_multiple_patterns(void) {
    volatile int counter = 0;
    volatile int data[4] = {1, 2, 3, 4};
    int sum = 0;
    
    /* Pattern 1: Jump with arithmetic after label */
    if (counter == 0) {
        goto pattern1;
    }
    
    sum += 100;
    
pattern1:
    /* Simple arithmetic candidate */
    data[0] = data[1] + data[2];
    
    /* Pattern 2: Another jump in same function */
    if (data[0] > 0) {
        goto pattern2;
    }
    
    sum += 200;
    
pattern2:
    /* Another candidate - register operation */
    asm volatile (
        "movl $42, %0"
        : "=r"(data[3])
        :
        : /* no clobbers */
    );
    
    /* Use results */
    for (int i = 0; i < 4; i++) {
        sum += data[i];
    }
    
    return sum;
}

/* Main orchestrator */
int main(void) {
    int results[4];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute all test patterns */
    results[0] = test_jump_to_label();
    results[1] = test_asm_candidate();
    results[2] = test_function_call();
    results[3] = test_multiple_patterns();
    
    /* Print results to prevent optimization */
    for (int i = 0; i < 4; i++) {
        printf("Test %d result: %d\n", i, results[i]);
    }
    
    return 0;
}
