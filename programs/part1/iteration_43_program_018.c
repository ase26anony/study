/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_c = 99;

/* Barrier to prevent reordering across jumps */
#define JUMP_BARRIER() __asm__ volatile ("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump to label */
    if (x != 0) {
        JUMP_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Should use register not live across jump */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Create a simple conditional jump */
    if (temp1 > temp2) {
        JUMP_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    result = temp1 - temp2;
    return result;
    
target2:
    /* Candidate: Logical operation with temporaries */
    result = temp1 & 0xFF;  /* Bitwise operation - safe and non-trapping */
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multiple_candidates(int val) {
    int tmp1 = val;
    int tmp2 = global_a;
    int result = 0;
    
    /* Use volatile to force register usage */
    volatile int force_reg = tmp1;
    
    if (force_reg > 10) {
        JUMP_BARRIER();
        goto target3;
    }
    
    result = tmp2 * 3;
    return result;
    
target3:
    /* Multiple simple instructions that could be candidates */
    tmp1 = tmp1 + global_b;    /* First candidate */
    result = tmp1 | 0x1;       /* Second candidate - logical OR */
    return result;
}

/* Test 4: Nested jumps to create different patterns */
int test_nested_jumps(int x, int y, int z) {
    int a = x;
    int b = y;
    int c = z;
    
    /* First level condition */
    if (a > 0) {
        /* Second level condition */
        if (b > 0) {
            JUMP_BARRIER();
            goto target4;
        }
        c = a + b;
    }
    
    return c * 2;
    
target4:
    /* Safe arithmetic with local temporaries */
    a = a + c;      /* Uses only local variables */
    return a;
}

/* Test 5: Jump with memory operation (load) at target */
int test_with_load(int *ptr, int idx) {
    int local1 = idx;
    int local2 = 0;
    
    if (local1 < 100) {
        JUMP_BARRIER();
        goto target5;
    }
    
    return local1 * 2;
    
target5:
    /* Load from memory - might be safe if address is valid */
    local2 = ptr[local1 % 10];  /* Bounded array access */
    return local2 + local1;
}

/* Test 6: Function with switch that generates jumps */
int test_switch_jump(int code) {
    int result = 0;
    int tmp = code;
    
    switch (tmp & 0x3) {  /* Mask to few cases */
        case 0:
            JUMP_BARRIER();
            goto target6;
        case 1:
            result = tmp + 1;
            break;
        default:
            result = tmp - 1;
            break;
    }
    
    return result;
    
target6:
    /* Simple increment - good candidate */
    result = tmp + 2;
    return result;
}

/* Test 7: Loop with break to label */
int test_loop_break(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        if (i == 5) {
            JUMP_BARRIER();
            goto target7;  /* Break from loop to label */
        }
        sum += i;
    }
    
    return sum;
    
target7:
    /* Arithmetic at target */
    sum = sum * 2;
    return sum;
}

/* Test 8: Use inline assembly to hint at delay slot filling on supported arches */
int test_asm_hint(int x) {
    int result = x;
    
#if HAS_DELAY_SLOTS
    /* On delay-slot architectures, use asm to influence codegen */
    if (x > 0) {
        /* This asm should generate a simple jump */
        __asm__ volatile (
            "bgtz %0, 1f\n\t"
            "nop\n\t"  /* Traditional delay slot nop */
            "1:\n\t"
            : : "r" (x) : "memory"
        );
        goto target8;
    }
#endif
    
    result = x * 3;
    return result;
    
target8:
    result = result + 7;
    return result;
}

/* Main driver that runs all tests */
int main(void) {
    int results[8];
    int final_result = 0;
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all test functions */
    results[0] = test_unconditional_jump(global_a, global_b);
    results[1] = test_conditional_jump(global_a, global_b);
    results[2] = test_multiple_candidates(global_c);
    results[3] = test_nested_jumps(10, 20, 30);
    results[4] = test_with_load(test_array, 3);
    results[5] = test_switch_jump(2);
    results[6] = test_loop_break(10);
    results[7] = test_asm_hint(5);
    
    /* Combine results to prevent optimization */
    for (int i = 0; i < 8; i++) {
        final_result ^= results[i];  /* XOR all results */
        printf("Test %d result: %d\n", i + 1, results[i]);
    }
    
    printf("Final checksum: %d\n", final_result);
    
#if HAS_DELAY_SLOTS
    printf("Compiled for architecture with delay slots\n");
#else
    printf("Compiled for architecture without delay slots (generic fallback)\n");
#endif
    
    return final_result != 0 ? 0 : 1;
}
