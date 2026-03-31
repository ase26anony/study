/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target_label1;
    }
    
    /* Dead code to avoid fall-through optimization */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    /* This should be safe, non-trapping, and splittable */
    c = a + b + 5;  /* next_trial candidate */
    
    /* Use result to prevent elimination */
    result = c;
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    unsigned int x = 0x1234, y = 0x5678;
    unsigned int mask = 0xFF;
    int count = 0;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 3; i++) {
        if (x & 1) {
            goto target_label2;
        }
        x >>= 1;
    }
    
    return -1;
    
target_label2:
    /* Candidate: bitwise operation with distinct variables */
    /* Uses different variables than the jump condition */
    mask = (y & 0xF0F0) | 0x0A0A;  /* next_trial candidate */
    
    count = mask;
    return count;
}

/* Test 3: Safe stack load/store operations */
static int __attribute__((optimize("O2"))) test_stack_ops_after_label(void) {
    int arr[4] = {1, 2, 3, 4};
    int temp = 0;
    int i = 0;
    
    /* Simple conditional jump */
    if (arr[0] == 1) {
        goto target_label3;
    }
    
    return 0;
    
target_label3:
    /* Candidate: stack-based memory operation */
    /* Loading from stack is safe and unlikely to trap */
    temp = arr[2] * 2;  /* next_trial candidate */
    
    /* Use in computation to prevent elimination */
    return temp + i;
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_comparison_after_label(void) {
    int p = 50, q = 60, r = 70;
    int flag = 0;
    
    /* Nested conditions to create interesting flow */
    if (p > 0) {
        if (q < 100) {
            goto target_label4;
        }
    }
    
    return flag;
    
target_label4:
    /* Candidate: comparison operation */
    /* Sets condition codes without side effects */
    flag = (r == 70);  /* next_trial candidate */
    
    return flag;
}

/* Test 5: Register move pattern */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    register int src = 42;  /* Hint to use register */
    int dst1 = 0, dst2 = 0;
    
    /* Unconditional jump */
    goto target_label5;
    
    /* Unreachable code */
    dst1 = 999;
    return dst1;
    
target_label5:
    /* Candidate: simple register-to-register move pattern */
    dst2 = src + 1;  /* next_trial candidate */
    
    return dst2;
}

/* Test 6: Multiple basic blocks with safe operation */
static int __attribute__((optimize("O3"))) test_multiple_blocks(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int sum = 0;
    
    /* First jump */
    if (a > 0) {
        goto block1;
    }
    
    return -1;
    
block1:
    /* First candidate instruction */
    b = c + d;  /* Potential next_trial */
    
    /* Another jump to create second opportunity */
    if (b > 5) {
        goto block2;
    }
    
    return b;
    
block2:
    /* Second candidate instruction */
    sum = a + b + 10;  /* Another potential next_trial */
    
    return sum;
}

/* Test 7: Avoid resource conflicts with careful variable usage */
static int __attribute__((optimize("O2"))) test_no_resource_conflict(void) {
    /* Use completely separate variable sets for jump vs operation */
    int jump_var1 = 100;
    int jump_var2 = 200;
    int op_var1 = 300;   /* Only used after label */
    int op_var2 = 400;   /* Only used after label */
    int result = 0;
    
    /* Jump condition uses only jump_var1, jump_var2 */
    if (jump_var1 < jump_var2) {
        goto compute_label;
    }
    
    return 0;
    
compute_label:
    /* Operation uses only op_var1, op_var2 - no conflict with jump resources */
    result = op_var1 * 2 + op_var2;  /* next_trial candidate */
    
    return result;
}

/* Test 8: Loop with internal jump to label */
static int __attribute__((optimize("O3"))) test_loop_with_jump(void) {
    int i, total = 0;
    int values[5] = {10, 20, 30, 40, 50};
    
    for (i = 0; i < 5; i++) {
        if (values[i] > 25) {
            goto process_label;
        }
        total += values[i];
        continue;
        
    process_label:
        /* Candidate in loop body after label */
        total += values[i] * 2;  /* next_trial candidate */
    }
    
    return total;
}

#pragma GCC pop_options

/* Main function to execute all tests */
int main(void) {
    int total_result = 0;
    
    /* Execute all test functions */
    total_result += test_arithmetic_after_label();
    total_result += test_bitwise_after_label();
    total_result += test_stack_ops_after_label();
    total_result += test_comparison_after_label();
    total_result += test_register_move();
    total_result += test_multiple_blocks();
    total_result += test_no_resource_conflict();
    total_result += test_loop_with_jump();
    
    printf("Total result: %d\n", total_result);
    
    /* Use result to prevent dead code elimination */
    if (total_result > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should not happen */
    }
}
