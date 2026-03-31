/* Test program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve jump structure */
__attribute__((noinline, optimize("O0")))
static int simple_arithmetic(int a, int b) {
    return a + b;
}

/* Function with goto pattern that might trigger delay slot filling */
__attribute__((noinline, optimize("O2")))
static int test_delay_slot_filling(int x) {
    volatile int result = 0;
    int temp1 = x;
    int temp2 = x * 2;
    
    /* Use goto to create a simple jump instruction */
    if (x > 100) {
        goto target_label;
    }
    
    /* Some code to prevent optimization merging */
    temp1 += 10;
    
target_label:
    /* Candidate instruction for delay slot filling:
       Simple arithmetic that doesn't trap, doesn't conflict with jump resources */
    asm volatile("" : "+r"(temp1) : : "memory");  /* Compiler barrier */
    
    /* Simple arithmetic operation - good candidate for delay slot */
    temp2 = temp1 + 5;
    
    /* Use the result to prevent dead code elimination */
    result = temp2;
    
    return result;
}

/* Another test with function call after label */
__attribute__((noinline, optimize("O2")))
static int test_with_function_call(int x) {
    int a = x;
    int b = x + 1;
    
    if (x < 0) {
        goto call_target;
    }
    
    a = x * 2;
    
call_target:
    /* Compiler barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Function call that doesn't inline - potential delay slot candidate */
    b = simple_arithmetic(a, 5);
    
    return b;
}

/* Test with memory operation that shouldn't fault */
__attribute__((noinline, optimize("O2")))
static int test_with_safe_memory_op(int x) {
    int array[4] = {x, x+1, x+2, x+3};
    int index = 0;
    int result = 0;
    
    if (x % 2 == 0) {
        goto memory_op;
    }
    
    index = 1;
    
memory_op:
    /* Safe memory access to stack variable */
    asm volatile("" ::: "memory");
    
    /* Access stack memory - should not fault */
    result = array[index];
    
    return result;
}

/* Complex test with multiple jumps and labels */
__attribute__((noinline, optimize("O2")))
static int test_multiple_jumps(int x) {
    int a = x;
    int b = x * 2;
    int c = x * 3;
    
    /* First jump pattern */
    if (a > 50) {
        goto label1;
    }
    
    a += 10;
    
label1:
    /* Simple arithmetic candidate */
    asm volatile("" ::: "memory");
    b = a + 7;
    
    /* Second jump pattern */
    if (b < 100) {
        goto label2;
    }
    
    b -= 5;
    
label2:
    /* Another candidate */
    asm volatile("" ::: "memory");
    c = b * 2;
    
    return c;
}

/* Test that specifically avoids trapping instructions */
__attribute__((noinline, optimize("O2")))
static int test_no_trap_instructions(int x) {
    int a = x;
    int b = 0;
    
    /* Avoid division (can trap) */
    if (x != 0) {
        goto safe_instruction;
    }
    
    a = 1;
    
safe_instruction:
    /* Use bitwise operations instead of division */
    asm volatile("" ::: "memory");
    
    /* Bitwise operations don't trap */
    b = a & 0xFF;
    b = b | 0x80;
    b = b ^ 0x40;
    
    return b;
}

/* Main function to run all tests */
int main(int argc, char *argv[]) {
    int test_value = 42;
    int results[5];
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all tests */
    results[0] = test_delay_slot_filling(test_value);
    results[1] = test_with_function_call(test_value);
    results[2] = test_with_safe_memory_op(test_value);
    results[3] = test_multiple_jumps(test_value);
    results[4] = test_no_trap_instructions(test_value);
    
    /* Print results to prevent optimization */
    for (int i = 0; i < 5; i++) {
        printf("Test %d result: %d\n", i, results[i]);
    }
    
    /* Additional test with different values to explore more paths */
    if (argc > 1) {
        int val = atoi(argv[1]);
        printf("Additional test with %d: %d\n", 
               val, test_delay_slot_filling(val));
    }
    
    return 0;
}
