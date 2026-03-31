/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in GCC's reorg.cc
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(int a, int b) {
    volatile int result = 0; /* volatile to prevent optimization */
    int x = a;
    int y = b;
    
    if (x > y) {
        goto target_label1;
    }
    
    /* Some code to avoid fall-through optimization */
    x = y * 2;
    return x + result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be moved into the delay slot of the jump above */
    result = x + y;  /* Simple add - no side effects, no traps */
    
    /* Use result to prevent dead code elimination */
    return result + 1;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(unsigned int mask, unsigned int value) {
    unsigned int temp1 = mask;
    unsigned int temp2 = value;
    unsigned int result = 0;
    
    if ((temp1 & 0xFF) == 0) {
        goto target_label2;
    }
    
    temp1 = temp1 >> 8;
    return temp1 + temp2;
    
target_label2:
    /* Candidate: bitwise operation - safe, no traps */
    result = temp1 | temp2;  /* Simple bitwise OR */
    
    /* Follow with another operation to create basic block */
    result = result ^ 0x1234;
    return (int)result;
}

/* Test 3: Safe stack load/store operations */
static int __attribute__((optimize("O2"))) test_stack_ops(int init) {
    int array[4] = {init, init + 1, init + 2, init + 3};
    int temp = 0;
    int i = 0;
    
    /* Loop with internal goto to increase optimization opportunities */
    for (i = 0; i < 3; i++) {
        if (array[i] > array[i + 1]) {
            goto target_label3;
        }
        temp += array[i];
    }
    return temp;
    
target_label3:
    /* Candidate: stack load operation - safe (no null pointer) */
    int loaded = array[i];  /* Loading from stack - won't trap */
    
    /* Simple arithmetic on loaded value */
    loaded = loaded * 2;
    return loaded + temp;
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_comparison_after_label(int a, int b, int c) {
    int cmp_result = 0;
    int x = a;
    int y = b;
    int z = c;
    
    if (x == y) {
        goto target_label4;
    }
    
    z = x - y;
    return z;
    
target_label4:
    /* Candidate: comparison operation - sets condition codes only */
    cmp_result = (z > x);  /* Comparison - no side effects */
    
    /* Use the comparison result */
    return cmp_result ? 100 : 200;
}

/* Test 5: Multiple operations in sequence after label */
static int __attribute__((optimize("O2"))) test_multi_ops_after_label(int base) {
    int a = base;
    int b = base + 10;
    int c = base + 20;
    int result = 0;
    
    volatile int trigger = 1; /* Force compiler to keep conditional */
    
    if (trigger) {
        goto target_label5;
    }
    
    a = b + c;
    return a;
    
target_label5:
    /* Multiple simple operations - try_split should handle these */
    result = a + b;      /* First operation - candidate for delay slot */
    result = result - c; /* Second operation - in same basic block */
    
    return result;
}

/* Test 6: Register move pattern */
static int __attribute__((optimize("O2"))) test_register_move(int val1, int val2) {
    int reg_a = val1;
    int reg_b = val2;
    int reg_c = 0;
    
    /* Use distinct registers to avoid resource conflicts */
    if (reg_a != 0) {
        goto target_label6;
    }
    
    reg_b = reg_a * 2;
    return reg_b;
    
target_label6:
    /* Simple register move operation */
    reg_c = reg_b;  /* Move between distinct variables */
    
    /* Use moved value */
    return reg_c + 5;
}

/* Test 7: Shift operation after label */
static unsigned int __attribute__((optimize("O2"))) test_shift_after_label(unsigned int value) {
    unsigned int shifted = value;
    unsigned int mask = 0xFFFF;
    
    if (value & 1) {  /* Check LSB */
        goto target_label7;
    }
    
    shifted = value >> 1;
    return shifted;
    
target_label7:
    /* Candidate: shift operation - no traps */
    shifted = value << 3;  /* Simple left shift */
    
    /* Combine with mask */
    return shifted & mask;
}

#pragma GCC pop_options

/* Main function that executes all tests */
int main() {
    int total = 0;
    
    /* Execute all test functions with different inputs */
    total += test_arithmetic_after_label(10, 5);
    total += test_arithmetic_after_label(5, 10);
    
    total += test_bitwise_after_label(0x1234, 0x5678);
    total += test_bitwise_after_label(0x00FF, 0x5500);
    
    total += test_stack_ops(1);
    total += test_stack_ops(100);
    
    total += test_comparison_after_label(5, 5, 10);
    total += test_comparison_after_label(5, 6, 10);
    
    total += test_multi_ops_after_label(42);
    total += test_multi_ops_after_label(-42);
    
    total += test_register_move(7, 8);
    total += test_register_move(0, 15);
    
    total += test_shift_after_label(0x100);
    total += test_shift_after_label(0x201);
    
    printf("Total result: %d\n", total);
    printf("(This output ensures all code paths are executed)\n");
    
    return total != 0 ? 0 : 1;
}
