/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in GCC's reorg.cc
 * Specifically targets lines 2135-2149 for coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
static int test_arithmetic_after_label(int a, int b) {
    volatile int result = 0;  /* volatile to prevent optimization */
    int x = a;
    int y = b;
    
    if (x > y) {
        goto target_label;
    }
    
    /* Some code to avoid fall-through optimization */
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return result;

target_label:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be safe, non-trapping, and splittable */
    int z = x + y;  /* next_trial candidate */
    result = z * 2;
    
    return result;
}

/* Test 2: Bitwise operations after label */
__attribute__((optimize("O3")))
static int test_bitwise_after_label(uint32_t mask, uint32_t value) {
    uint32_t result = 0;
    uint32_t temp = value;
    
    if ((temp & 0xFF) == 0) {
        goto bitwise_label;
    }
    
    /* Different path */
    temp >>= 4;
    return (int)result;

bitwise_label:
    /* Candidate: bitwise operation - safe and non-trapping */
    uint32_t cleared = temp & mask;  /* next_trial candidate */
    result = cleared | 0x80000000;
    
    return (int)result;
}

/* Test 3: Safe memory operation (stack-based) */
__attribute__((optimize("O2")))
static int test_stack_operation(int base) {
    int array[4] = {base, base+1, base+2, base+3};
    int sum = 0;
    int index = 0;
    
    if (base % 2 == 0) {
        goto load_label;
    }
    
    /* Alternative computation */
    for (int i = 0; i < 4; i++) {
        sum += array[i];
    }
    return sum;

load_label:
    /* Candidate: stack load - should not trap */
    int loaded = array[index];  /* next_trial candidate */
    sum = loaded * 3;
    
    return sum;
}

/* Test 4: Comparison operation after label */
__attribute__((optimize("O2")))
static int test_comparison_after_label(int a, int b, int c) {
    int result = 0;
    
    if (a > 100) {
        goto compare_label;
    }
    
    /* Different computation */
    result = b + c;
    return result;

compare_label:
    /* Candidate: comparison - sets condition codes */
    int cmp_result = (b < c);  /* next_trial candidate */
    result = cmp_result ? a : b;
    
    return result;
}

/* Test 5: Multiple jumps to same label with different conditions */
__attribute__((optimize("O3")))
static int test_multiple_jumps(int val) {
    int x = val;
    int y = x * 2;
    int z = 0;
    
    /* Multiple jump opportunities */
    if (x < 0) {
        goto compute_label;
    }
    
    if (x > 1000) {
        goto compute_label;
    }
    
    if ((x & 1) == 0) {
        goto compute_label;
    }
    
    /* Default path */
    z = x + y;
    return z;

compute_label:
    /* Candidate: arithmetic with distinct variables */
    int temp = y - x;  /* next_trial candidate */
    z = temp * 3;
    
    return z;
}

/* Test 6: Nested control flow with safe operation */
__attribute__((optimize("O2")))
static int test_nested_flow(int limit) {
    int counter = 0;
    int accumulator = 0;
    
    while (counter < limit) {
        if (counter == limit / 2) {
            goto mid_loop_label;
        }
        
        accumulator += counter;
        counter++;
        
        continue;
        
    mid_loop_label:
        /* Candidate: simple increment - safe to move */
        int increment = 5;  /* next_trial candidate */
        accumulator += increment;
        counter++;
    }
    
    return accumulator;
}

/* Test 7: Register move pattern */
__attribute__((optimize("O2")))
static int test_register_move(int a, int b) {
    int reg1 = a;
    int reg2 = b;
    int output = 0;
    
    if (reg1 != reg2) {
        goto move_label;
    }
    
    output = reg1 * reg2;
    return output;

move_label:
    /* Candidate: register-to-register "move" via addition */
    int moved = reg1 + 0;  /* next_trial candidate - effectively a move */
    output = moved - reg2;
    
    return output;
}

/* Test 8: Shift operation after label */
__attribute__((optimize("O3")))
static int test_shift_operation(unsigned int value) {
    unsigned int result = 0;
    
    if (value == 0) {
        goto shift_label;
    }
    
    result = value >> 1;
    return (int)result;

shift_label:
    /* Candidate: shift operation - non-trapping */
    unsigned int shifted = value << 2;  /* next_trial candidate */
    result = shifted | 1;
    
    return (int)result;
}

/* Main function to execute all tests and ensure code runs */
int main(void) {
    int total = 0;
    
    /* Execute all test functions with various inputs */
    total += test_arithmetic_after_label(10, 20);
    total += test_arithmetic_after_label(30, 15);
    
    total += test_bitwise_after_label(0x00FF00FF, 0x12345678);
    total += test_bitwise_after_label(0xFFFFFFFF, 0x00000000);
    
    total += test_stack_operation(5);
    total += test_stack_operation(8);
    
    total += test_comparison_after_label(150, 50, 75);
    total += test_comparison_after_label(50, 100, 75);
    
    total += test_multiple_jumps(-5);
    total += test_multiple_jumps(1500);
    total += test_multiple_jumps(42);
    
    total += test_nested_flow(10);
    total += test_nested_flow(5);
    
    total += test_register_move(10, 20);
    total += test_register_move(15, 15);
    
    total += test_shift_operation(0);
    total += test_shift_operation(100);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    /* Also use the result in a conditional to ensure all paths are considered */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
