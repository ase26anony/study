/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch test_delay_slot.c -o test_delay_slot
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 test_delay_slot.c -o test_delay_slot
 */

#include <stdio.h>
#include <stdint.h>

// Force optimization on specific functions
#pragma GCC optimize("O2")

/* Test 1: Simple arithmetic after label - likely candidate for delay slot */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10;  // volatile to prevent optimization
    int b = 20;
    int c = 30;
    int result = 0;
    
    if (a > 5) {
        goto target_label1;
    }
    
    // Dead code - won't be executed
    result = -1;
    return result;
    
target_label1:
    /* This instruction should be candidate for delay slot:
     * - Simple arithmetic (add)
     * - No memory access that could trap
     * - Sets result variable distinct from jump condition
     */
    result = b + c;  // next_trial candidate
    
    // Use result to prevent dead code elimination
    return result + 1;
}

/* Test 2: Bitwise operation after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    int x = 0x1234;
    int y = 0x5678;
    int z = 0;
    volatile int flag = 1;
    
    // Loop to increase optimization opportunities
    for (int i = 0; i < 3; i++) {
        if (flag) {
            goto target_label2;
        }
        z = x & y;  // Not executed
    }
    
    return z;
    
target_label2:
    /* Candidate: bitwise operation
     * - Simple, non-trapping
     * - Uses different variables than jump condition
     */
    z = x | y;  // next_trial candidate
    
    // Prevent optimization
    asm volatile("" : "+r"(z));
    return z ^ 0xFFFF;
}

/* Test 3: Register move/assignment pattern */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    int src1 = 100;
    int src2 = 200;
    int dst1, dst2;
    volatile int cond = 1;
    
    if (cond != 0) {
        goto target_label3;
    }
    
    dst1 = src1;
    return dst1;
    
target_label3:
    /* Candidate: simple register-to-register move
     * - Very safe for delay slot
     * - No resource conflicts if registers are different
     */
    dst2 = src2;  // next_trial candidate
    
    // Create dependency to prevent reordering
    return dst2 + src1;
}

/* Test 4: Stack-based memory operation (safe load) */
static int __attribute__((optimize("O2"))) test_safe_load(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    volatile int idx = 0;
    
    // Multiple jumps to same label
    if (idx == 0) {
        goto target_label4;
    } else if (idx == 1) {
        goto target_label4;
    }
    
    return -1;
    
target_label4:
    /* Candidate: stack memory load
     * - Stack address is always valid (won't trap)
     * - Simple memory operation
     */
    temp = array[2];  // next_trial candidate
    
    // Use in computation
    return temp * 2;
}

/* Test 5: Comparison operation */
static int __attribute__((optimize("O3"))) test_comparison(void) {
    int p = 50;
    int q = 60;
    int cmp_result = 0;
    volatile int trigger = 1;
    
    // Nested control flow
    {
        if (trigger > 0) {
            goto target_label5;
        }
    }
    
    return cmp_result;
    
target_label5:
    /* Candidate: comparison operation
     * - Sets condition codes
     * - No side effects
     */
    cmp_result = (p < q);  // next_trial candidate
    
    // Branch based on result to use it
    if (cmp_result) {
        return p;
    } else {
        return q;
    }
}

/* Test 6: Multiple safe instructions in sequence */
static int __attribute__((optimize("O2"))) test_multiple_instructions(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int r1 = 0, r2 = 0;
    volatile int v = 1;
    
    // Switch-like with goto
    if (v == 1) goto label_a;
    if (v == 2) goto label_b;
    
label_a:
    if (v) goto target_label6;
    
label_b:
    return -1;
    
target_label6:
    /* First instruction after label - primary candidate
     * Simple arithmetic with distinct variables
     */
    r1 = a + b;  // next_trial candidate
    
    // Additional instruction (not part of delay slot candidate)
    r2 = c * d;
    
    return r1 + r2;
}

/* Test 7: Avoid resource conflicts explicitly */
static int __attribute__((optimize("O2"))) test_no_conflict(void) {
    // Variables for jump condition
    volatile int cond_var1 = 10;
    volatile int cond_var2 = 20;
    
    // COMPLETELY DIFFERENT variables for delay slot candidate
    int ds_var1 = 30;  // Only used in delay slot candidate
    int ds_var2 = 40;  // Only used in delay slot candidate
    int ds_result = 0;
    
    if (cond_var1 < cond_var2) {
        goto target_label7;
    }
    
    return 0;
    
target_label7:
    /* Candidate: uses variables NOT used in jump condition
     * Minimizes chance of resource conflicts
     */
    ds_result = ds_var1 - ds_var2;  // next_trial candidate
    
    return ds_result;
}

/* Test 8: Loop with internal label jump */
static int __attribute__((optimize("O3"))) test_loop_with_jump(void) {
    int sum = 0;
    int i;
    volatile int limit = 5;
    
    for (i = 0; i < limit; i++) {
        if (i & 1) {
            goto loop_label;
        }
        sum += i;
        continue;
        
    loop_label:
        /* Candidate inside loop
         * - Simple increment
         * - Loop increases optimization opportunities
         */
        sum += 2;  // next_trial candidate
    }
    
    return sum;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    // Run all test functions
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_register_move();
    total += test_safe_load();
    total += test_comparison();
    total += test_multiple_instructions();
    total += test_no_conflict();
    total += test_loop_with_jump();
    
    // Print result to ensure all code is executed
    printf("Total: %d\n", total);
    
    // Also print individual results for debugging
    printf("Results: %d %d %d %d %d %d %d %d\n",
           test_arithmetic_after_label(),
           test_bitwise_after_label(),
           test_register_move(),
           test_safe_load(),
           test_comparison(),
           test_multiple_instructions(),
           test_no_conflict(),
           test_loop_with_jump());
    
    return 0;
}
