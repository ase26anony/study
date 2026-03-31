/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to maintain control over instruction placement */
__attribute__((noinline, optimize("O0")))
static int simple_arithmetic(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a + b;
}

__attribute__((noinline, optimize("O0")))
static void noop_operation(volatile int *p) {
    /* Memory operation that shouldn't trap (stack variable) */
    *p = *p + 1;
}

/* Function with goto pattern targeting MIPS delay slots */
__attribute__((optimize("O2")))
static int test_delay_slot_mips(void) {
    volatile int result = 0;
    volatile int a = 5, b = 10;
    volatile int flag = 1;
    
    /* Use asm to prevent certain optimizations */
    asm volatile("" ::: "memory");
    
    if (flag) {
        /* This should generate a simple jump to label */
        goto target_label;
    }
    
    /* Dead code to create separation */
    result = a * b;
    
target_label:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate for delay slot: simple arithmetic in asm
       Modifies only general purpose register, no memory, no condition codes */
    register int temp asm("eax") = a;
    asm volatile("addl $1, %0" : "+r"(temp) ::);
    result = temp;
    
    /* Use result to prevent elimination */
    return result;
}

/* Test with function call after label */
__attribute__((optimize("O2")))
static int test_function_call_delay(void) {
    volatile int x = 42, y = 100;
    volatile int cond = 1;
    
    /* Create simple jump */
    if (cond) {
        goto func_target;
    }
    
    x = x * y; /* Dead code */
    
func_target:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate - must not be inlined */
    int r = simple_arithmetic(x, y);
    
    /* Use result */
    return r + 1;
}

/* Test with memory operation (stack variable - should not trap) */
__attribute__((optimize("O1")))
static int test_memory_op_delay(void) {
    volatile int arr[4] = {1, 2, 3, 4};
    volatile int idx = 0;
    volatile int do_jump = 1;
    
    if (do_jump) {
        goto mem_target;
    }
    
    idx = 10; /* Dead code */
    
mem_target:
    /* Barrier */
    asm volatile("" ::: "memory");
    
    /* Memory operation on stack variable - should not fault */
    int val = arr[idx];
    arr[idx] = val + 1;
    
    return arr[0];
}

/* Complex test with multiple jumps */
__attribute__((optimize("O3")))
static int test_nested_jumps(void) {
    volatile int counter = 0;
    volatile int max = 3;
    
    for (int i = 0; i < max; i++) {
        volatile int inner_flag = (i % 2 == 0);
        
        if (inner_flag) {
            goto inner_label;
        }
        
        counter += 10;
        continue;
        
    inner_label:
        /* Barrier */
        asm volatile("" ::: "memory");
        
        /* Simple increment - good delay slot candidate */
        register int cnt asm("ecx") = counter;
        asm volatile("incl %0" : "+r"(cnt) ::);
        counter = cnt;
    }
    
    return counter;
}

/* Test avoiding trap conditions */
__attribute__((optimize("O2")))
static int test_no_trap(void) {
    volatile int x = 100, y = 200;
    volatile int z = 1;
    
    /* Avoid division (can trap) */
    if (z) {
        goto safe_target;
    }
    
    x = x / y; /* This would trap if y=0, but dead code */
    
safe_target:
    asm volatile("" ::: "memory");
    
    /* Safe: bitwise operations don't trap */
    int result = x & y;
    result = result | 0xFF;
    result = result ^ 0xAA;
    
    return result;
}

/* Main orchestrator */
int main(void) {
    int total = 0;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all tests */
    total += test_delay_slot_mips();
    printf("Test 1 complete\n");
    
    total += test_function_call_delay();
    printf("Test 2 complete\n");
    
    total += test_memory_op_delay();
    printf("Test 3 complete\n");
    
    total += test_nested_jumps();
    printf("Test 4 complete\n");
    
    total += test_no_trap();
    printf("Test 5 complete\n");
    
    printf("Total result: %d (should be non-zero)\n", total);
    
    return total != 0 ? 0 : 1;
}
