/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_func(int x) {
    return x + 1;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_simple_jump_with_arithmetic(void) {
    volatile int a = 5, b = 10, c = 0;
    
    /* Use volatile to prevent optimization of the jump */
    if (a > 0) {
        goto target_label;
    }
    
    /* Dead code to create separation */
    b = b * 2;
    
target_label:
    /* Compiler barrier to prevent merging with label */
    asm volatile("" ::: "memory");
    
    /* Simple arithmetic that doesn't trap - good delay slot candidate */
    c = a + b;
    
    /* Use result to prevent elimination */
    printf("Result 1: %d\n", c);
}

/* Another test with function call after label */
__attribute__((optimize("O0")))
static void test_jump_with_func_call(void) {
    volatile int x = 42;
    volatile int y = 0;
    
    if (x != 0) {
        goto func_target;
    }
    
    /* Some intermediate code */
    x = x * 2;
    
func_target:
    /* Barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Function call - may be eligible for delay slot */
    y = simple_func(x);
    
    printf("Result 2: %d\n", y);
}

/* Test with asm statement as delay slot candidate */
__attribute__((optimize("O0")))
static void test_jump_with_asm(void) {
    volatile int counter = 0;
    
    /* Force a simple jump */
    if (counter >= 0) {
        goto asm_target;
    }
    
    counter = 100;
    
asm_target:
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Simple asm that modifies a register but doesn't conflict with jump */
    /* Use specific register to avoid resource conflicts */
    register int reg_var asm("ebx") = counter;
    asm volatile("addl $1, %0" : "+r"(reg_var) ::);
    counter = reg_var;
    
    printf("Result 3: %d\n", counter);
}

/* Test with memory operation (stack variable - safe) */
__attribute__((optimize("O0")))
static void test_jump_with_memory_op(void) {
    volatile int arr[4] = {1, 2, 3, 4};
    volatile int sum = 0;
    
    /* Create simple jump */
    if (arr[0] > 0) {
        goto mem_target;
    }
    
    sum = 100;
    
mem_target:
    /* Barrier */
    asm volatile("" ::: "memory");
    
    /* Safe memory access to stack variable */
    sum = arr[1] + arr[2];
    
    printf("Result 4: %d\n", sum);
}

/* Nested jumps to create more complex patterns */
__attribute__((optimize("O0")))
static void test_nested_jumps(void) {
    volatile int flag = 1;
    volatile int result = 0;
    
    if (flag) {
        goto outer_label;
    }
    
    result = 99;
    return;
    
outer_label:
    asm volatile("" ::: "memory");
    
    if (result == 0) {
        goto inner_label;
    }
    
    result = 50;
    
inner_label:
    asm volatile("" ::: "memory");
    
    /* Simple increment operation */
    result++;
    
    printf("Result 5: %d\n", result);
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_simple_jump_with_arithmetic();
    test_jump_with_func_call();
    test_jump_with_asm();
    test_jump_with_memory_op();
    test_nested_jumps();
    
    return 0;
}
