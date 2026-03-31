/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_calc(int a, int b) {
    return a + b;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void safe_operation(int *ptr) {
    /* Simple operation that shouldn't trap */
    *ptr = *ptr + 1;
}

/* Function with optimization disabled to prevent premature sequence formation */
__attribute__((optimize("O0")))
static int test_simple_jump(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int result = 0;
    
    /* Use goto to create simple jump instruction */
    if (a < b) {
        goto target_label;
    }
    
    /* Dead code that won't be executed but prevents optimization */
    c = a * b;
    
target_label:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic that shouldn't trap
       - No resource conflicts with jump
       - Not part of a SEQUENCE
    */
    result = a + b;
    
    /* Use result to prevent dead code elimination */
    return result + c;
}

/* Function with memory operation as delay slot candidate */
__attribute__((optimize("O0")))
static int test_jump_with_mem_op(void) {
    volatile int x = 5, y = 15;
    volatile int arr[4] = {1, 2, 3, 4};
    volatile int *ptr = arr;
    
    /* Force simple jump */
    if (x > 0) {
        goto mem_target;
    }
    
    y = x * 2;  /* Dead code */
    
mem_target:
    /* Memory operation that should be safe (stack access) */
    int temp = arr[2];
    
    /* Compiler barrier to prevent merging */
    asm volatile("" ::: "memory");
    
    /* Simple arithmetic after barrier */
    temp = temp + x;
    
    return temp + y;
}

/* Test with inline asm as delay slot candidate */
__attribute__((optimize("O0")))
static int test_jump_with_asm(void) {
    volatile int reg1 = 100, reg2 = 200;
    volatile int output = 0;
    
    /* Create simple jump */
    if (reg1 != 0) {
        goto asm_target;
    }
    
    reg2 = reg1 / 2;  /* Dead code */
    
asm_target:
    /* Inline asm that:
       - Modifies only general purpose register (eax)
       - Doesn't touch memory
       - Doesn't set condition codes (no "cc" clobber)
       - Simple operation unlikely to trap
    */
    asm volatile(
        "addl %1, %0\n\t"
        : "+r"(output)
        : "r"(reg1)
        : /* No clobbers - avoid resource conflicts */
    );
    
    return output + reg2;
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O0")))
static int test_jump_with_call(void) {
    volatile int val1 = 30, val2 = 40;
    volatile int sum = 0;
    
    /* Simple jump */
    if (val1 < 50) {
        goto call_target;
    }
    
    val2 = val1 * 3;  /* Dead code */
    
call_target:
    /* Function call - may be eligible if it doesn't conflict */
    sum = simple_calc(val1, val2);
    
    return sum;
}

/* Complex test with nested jumps */
__attribute__((optimize("O0")))
static int test_nested_jumps(void) {
    volatile int counter = 0;
    volatile int data[8];
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        data[i] = i * 10;
    }
    
    /* Multiple jumps to create scheduling opportunities */
    if (counter == 0) {
        goto first_label;
    }
    
    data[0] = 999;  /* Dead code */
    
first_label:
    /* First candidate - simple arithmetic */
    int temp = data[1] + data[2];
    
    /* Another jump */
    if (temp > 0) {
        goto second_label;
    }
    
    data[3] = 888;  /* Dead code */
    
second_label:
    /* Second candidate - safe memory operation */
    safe_operation(&data[4]);
    
    return temp + data[4];
}

/* Main function to run all tests */
int main(void) {
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    results[0] = test_simple_jump();
    printf("Test 1 result: %d\n", results[0]);
    
    results[1] = test_jump_with_mem_op();
    printf("Test 2 result: %d\n", results[1]);
    
    results[2] = test_jump_with_asm();
    printf("Test 3 result: %d\n", results[2]);
    
    results[3] = test_jump_with_call();
    printf("Test 4 result: %d\n", results[3]);
    
    results[4] = test_nested_jumps();
    printf("Test 5 result: %d\n", results[4]);
    
    /* Use results to prevent optimization */
    int final = 0;
    for (int i = 0; i < 5; i++) {
        final += results[i];
    }
    
    printf("Final checksum: %d\n", final);
    return final != 0 ? 0 : 1;
}
