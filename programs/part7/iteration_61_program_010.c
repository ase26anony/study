/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in GCC's reorg.cc
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch test_delay_slot.c -o test
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 test_delay_slot.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - most likely candidate */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target_label1;
    }
    
    /* This should be optimized away */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    c = a + b;          /* next_trial: add instruction */
    result = c * 2;     /* Use result to prevent elimination */
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    unsigned int x = 0xABCD, y = 0x1234;
    unsigned int mask = 0xFF;
    int count = 0;
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < 3; i++) {
        if (x > y) {
            goto bitwise_label;
        }
        x += 1;
    }
    
    return -1;
    
bitwise_label:
    /* Candidate: bitwise operations - safe and non-trapping */
    unsigned int z = x & mask;      /* AND operation */
    unsigned int w = z | 0x100;     /* OR operation */
    return (int)(w ^ 0x55);         /* XOR and return */
}

/* Test 3: Register move/swap pattern */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    int r1 = 42, r2 = 17, r3 = 99;
    int temp;
    
    /* Conditional jump to label */
    if (r1 != 0) {
        goto move_label;
    }
    
    return 0;
    
move_label:
    /* Candidate: register moves - very safe for delay slot */
    temp = r1;      /* Move r1 to temp */
    r1 = r2;        /* Move r2 to r1 */
    r2 = temp;      /* Move temp to r2 */
    
    /* Use all variables to prevent elimination */
    return r1 + r2 + r3;
}

/* Test 4: Stack-based memory operation (safe load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 2;
    int value;
    
    /* Simple jump based on array value */
    if (array[0] == 1) {
        goto stack_label;
    }
    
    return -1;
    
stack_label:
    /* Candidate: stack load operation - should not trap */
    value = array[index];       /* Load from stack */
    array[1] = value + 10;      /* Store to stack */
    return array[1] + array[2];
}

/* Test 5: Comparison operation setting condition codes */
static int __attribute__((optimize("O2"))) test_comparison(void) {
    int p = 100, q = 200;
    int cmp_result;
    
    /* Nested conditions to create jump opportunities */
    if (p > 50) {
        if (q < 300) {
            goto compare_label;
        }
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison instruction - sets flags only */
    cmp_result = (p < q);      /* Comparison, no trap possible */
    
    /* Use comparison result in computation */
    return cmp_result ? p : q;
}

/* Test 6: Multiple basic blocks with labels */
static int __attribute__((optimize("O3"))) test_multiple_blocks(void) {
    int counter = 0;
    int sum = 0;
    
    /* Create multiple jumps to different labels */
    for (int i = 0; i < 5; i++) {
        counter++;
        
        if (counter % 2 == 0) {
            goto even_label;
        } else {
            goto odd_label;
        }
        
    even_label:
        /* Candidate 1: Simple increment */
        sum += i * 2;
        continue;
        
    odd_label:
        /* Candidate 2: Decrement operation */
        sum -= i;
        continue;
    }
    
    return sum;
}

/* Test 7: Avoid resource conflicts - use distinct variables */
static int __attribute__((optimize("O2"))) test_no_conflict(void) {
    /* Variables for jump condition */
    int cond_a = 10, cond_b = 20;
    
    /* Distinct variables for delay slot candidate */
    int slot_x = 30, slot_y = 40, slot_z;
    
    if (cond_a < cond_b) {
        goto no_conflict_label;
    }
    
    return 0;
    
no_conflict_label:
    /* Candidate: uses completely different variables than jump condition */
    slot_z = slot_x * slot_y;      /* No conflict with cond_a/cond_b */
    return slot_z + 5;
}

/* Test 8: Try split candidate - more complex but splittable pattern */
static int __attribute__((optimize("O2"))) test_splittable(void) {
    unsigned int val = 0x87654321;
    unsigned int rot;
    
    /* Force a jump */
    if (val != 0) {
        goto split_label;
    }
    
    return 0;
    
split_label:
    /* Candidate: operation that try_split might want to split */
    rot = (val << 4) | (val >> 28);    /* Rotate left by 4 bits */
    
    /* Additional operation to make pattern interesting */
    rot ^= 0xF0F0F0F0;
    return (int)(rot & 0xFFFF);
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Run all test functions */
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_register_move();
    total += test_stack_ops();
    total += test_comparison();
    total += test_multiple_blocks();
    total += test_no_conflict();
    total += test_splittable();
    
    printf("Total result: %d\n", total);
    printf("(This value should be deterministic and non-zero)\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        printf("All tests executed successfully.\n");
    }
    
    return 0;
}
