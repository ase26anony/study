/* test_delay_slot.c
 * Designed to trigger delay slot optimization logic in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -fno-omit-frame-pointer test_delay_slot.c -o test_delay_slot
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer test_delay_slot.c -o test_delay_slot
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - likely eligible for delay slot */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        goto target_label1;
    }
    
    /* Dead code to create jump opportunity */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic, no side effects */
    c = a + b;  /* This should be movable into delay slot */
    result = c * 2;
    return result;
}

/* Test 2: Register move operation after label */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    int x = 5, y = 15, z = 25;
    int temp;
    
    /* Create jump with simple condition */
    if (x != 0) {
        goto move_label;
    }
    
    return -1;
    
move_label:
    /* Candidate: simple register-to-register move pattern */
    temp = y;  /* Simple move that can be optimized */
    z = temp + x;
    return z;
}

/* Test 3: Bitwise operation after label - safe and splittable */
static int __attribute__((optimize("O2"))) test_bitwise_ops(void) {
    unsigned int flags = 0xFF00;
    unsigned int mask = 0x00FF;
    unsigned int result = 0;
    
    /* Force a jump */
    if (flags & 0x8000) {
        goto bitwise_label;
    }
    
    return 0;
    
bitwise_label:
    /* Candidate: bitwise operation, no memory access */
    result = flags & mask;  /* Safe operation for delay slot */
    result = result | 0x0100;
    return (int)result;
}

/* Test 4: Stack-based memory operation (load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 2;
    int sum = 0;
    
    /* Conditional jump */
    if (index < 4) {
        goto load_label;
    }
    
    return -1;
    
load_label:
    /* Candidate: stack load operation - should not trap */
    sum = array[index];  /* Stack access is safe */
    sum += array[0];
    return sum;
}

/* Test 5: Comparison operation that sets condition codes */
static int __attribute__((optimize("O2"))) test_comparison(void) {
    int val1 = 100, val2 = 200;
    int cmp_result;
    
    /* Nested control flow to encourage optimization */
    for (int i = 0; i < 3; i++) {
        if (val1 < val2) {
            goto compare_label;
        }
        val1 += 10;
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison operation - sets flags but safe */
    cmp_result = (val1 < val2);  /* Comparison for delay slot */
    return cmp_result ? 1 : 0;
}

/* Test 6: Multiple basic blocks with jumps to same label */
static int __attribute__((optimize("O3"))) test_multiple_jumps(void) {
    int counter = 0;
    int a = 1, b = 2, c = 3;
    
    /* Multiple jump paths to same label */
    if (a > 0) {
        goto common_label;
    } else if (b > 0) {
        goto common_label;
    } else {
        c = 10;
    }
    
    return counter;
    
common_label:
    /* Candidate: simple arithmetic with distinct registers */
    counter = b + c;  /* Uses different vars than jump condition */
    counter *= a;
    return counter;
}

/* Test 7: Loop with internal goto - increases optimization opportunities */
static int __attribute__((optimize("O2"))) test_loop_with_goto(void) {
    int i, total = 0;
    int values[5] = {1, 2, 3, 4, 5};
    
    for (i = 0; i < 5; i++) {
        if (values[i] > 3) {
            goto process_label;
        }
        total += values[i];
        continue;
        
    process_label:
        /* Candidate: safe operation in loop context */
        total += values[i] * 2;  /* Simple arithmetic */
    }
    
    return total;
}

/* Test 8: Avoid resource conflicts by using fresh variables */
static int __attribute__((optimize("O2"))) test_no_resource_conflict(void) {
    int jump_cond = 1;
    int var1 = 10, var2 = 20;
    /* Fresh variables not used in jump condition */
    int fresh1 = 30, fresh2 = 40;
    
    /* Simple jump based on condition */
    if (jump_cond) {
        goto fresh_op_label;
    }
    
    return var1 + var2;
    
fresh_op_label:
    /* Candidate: uses only fresh variables to avoid resource conflicts */
    fresh1 = fresh2 + 5;  /* No overlap with jump condition resources */
    return fresh1 + fresh2;
}

/* Test 9: try_split candidate - operation that can be split */
static int __attribute__((optimize("O2"))) test_splittable_pattern(void) {
    int x = 100, y = 200;
    int result = 0;
    
    /* Create predictable jump */
    if (x != y) {
        goto split_label;
    }
    
    return -1;
    
split_label:
    /* Candidate: operation that try_split can handle */
    result = (x << 2) | (y >> 1);  /* Compound but splittable operation */
    return result;
}

/* Test 10: Avoid trapping operations - no division, no pointer deref */
static int __attribute__((optimize("O2"))) test_no_trap(void) {
    int safe1 = 1000;
    int safe2 = 2000;
    int output = 0;
    
    /* Safe jump condition */
    if (safe1 > 0) {
        goto safe_label;
    }
    
    return 0;
    
safe_label:
    /* Candidate: guaranteed non-trapping operation */
    output = safe1 * safe2;  /* Multiplication doesn't trap */
    output = output + 1;
    return output;
}

#pragma GCC pop_options

/* Main function to execute all tests and ensure code runs */
int main(void) {
    int total = 0;
    
    /* Execute all test functions to trigger compilation paths */
    total += test_arithmetic_after_label();
    total += test_register_move();
    total += test_bitwise_ops();
    total += test_stack_ops();
    total += test_comparison();
    total += test_multiple_jumps();
    total += test_loop_with_goto();
    total += test_no_resource_conflict();
    total += test_splittable_pattern();
    total += test_no_trap();
    
    printf("Total result: %d\n", total);
    printf("All delay slot patterns executed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should not happen */
    }
}
