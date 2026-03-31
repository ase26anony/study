/* Test program for triggering delay slot optimization conditions in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
static int test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple condition to force jump */
    if (a < b) {
        goto target_label1;
    }
    
    /* This should be dead code, but keeps compiler from optimizing away */
    result = -1;
    return result;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic */
    /* This is safe, non-trapping, and can potentially be moved to delay slot */
    c = a + b + 5;  /* Simple integer arithmetic */
    
    /* Use result to prevent dead code elimination */
    result = c;
    return result;
}

/* Test function 2: Register move pattern */
__attribute__((optimize("O2")))
static int test_register_move(void) {
    int x = 100, y = 200, z = 300;
    int temp;
    
    /* Force jump with simple condition */
    if (x != 0) {
        goto move_label;
    }
    
    return -1;
    
move_label:
    /* Candidate: simple register-to-register move via arithmetic */
    temp = y;  /* Effectively a move operation */
    z = temp + 1;  /* Simple increment */
    
    return z;
}

/* Test function 3: Bitwise operations after label */
__attribute__((optimize("O2")))
static int test_bitwise_ops(void) {
    unsigned int flags = 0xFF00;
    unsigned int mask = 0x00FF;
    unsigned int result = 0;
    
    /* Simple unconditional-like jump pattern */
    if (flags & 0x8000) {
        goto bitwise_label;
    } else {
        goto bitwise_label;  /* Both paths go to same label */
    }
    
    /* Unreachable, but creates control flow */
    return 0;
    
bitwise_label:
    /* Candidate: safe bitwise operation */
    result = flags & mask;  /* Non-trapping bitwise AND */
    
    /* Follow with another safe operation */
    result |= 0x1000;
    
    return (int)result;
}

/* Test function 4: Stack-based memory operation */
__attribute__((optimize("O2")))
static int test_stack_operation(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 2;
    int value;
    
    /* Create predictable jump */
    for (int i = 0; i < 2; i++) {
        if (i == 1) {
            goto load_label;
        }
    }
    
    return -1;
    
load_label:
    /* Candidate: stack load (should be safe, non-trapping) */
    value = array[index];  /* Stack access, won't fault */
    
    /* Simple use of value */
    value *= 2;
    
    return value;
}

/* Test function 5: Comparison operation */
__attribute__((optimize("O2")))
static int test_comparison(void) {
    int p = 50, q = 60;
    int cmp_result;
    
    /* Nested condition to encourage jump optimization */
    if (p > 0) {
        if (q > 0) {
            goto compare_label;
        }
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison operation (sets condition codes) */
    cmp_result = (p < q);  /* Simple comparison */
    
    /* Use result in arithmetic */
    return cmp_result ? p : q;
}

/* Test function 6: Multiple safe instructions sequence */
__attribute__((optimize("O3")))  /* Higher optimization */
static int test_multiple_instructions(void) {
    volatile int counter = 0;
    int a = 5, b = 10, c = 15;
    
    /* Loop with internal goto to create jump opportunities */
    for (int i = 0; i < 3; i++) {
        counter++;
        if (counter > 1) {
            goto sequence_label;
        }
    }
    
    return -1;
    
sequence_label:
    /* Multiple simple instructions that could be candidates */
    a = b + 1;      /* First simple op */
    c = a * 2;      /* Second simple op - compiler might choose one */
    
    return a + c;
}

/* Test function 7: Avoid resource conflicts explicitly */
__attribute__((optimize("O2")))
static int test_no_resource_conflict(void) {
    /* Use distinct variables to avoid resource conflicts */
    int jump_var = 100;      /* Only used in jump condition */
    int delay_var1 = 200;    /* Used after label */
    int delay_var2 = 300;    /* Used after label */
    int output = 0;
    
    /* Simple jump condition using only jump_var */
    if (jump_var > 50) {
        goto safe_label;
    }
    
    return -1;
    
safe_label:
    /* Candidate: uses completely different variables than jump condition */
    delay_var1 = delay_var2 + 5;  /* No conflict with jump_var */
    
    output = delay_var1;
    return output;
}

/* Test function 8: Try split candidate pattern */
__attribute__((optimize("O2")))
static int test_try_split_pattern(void) {
    int x = 0x1234;
    int y = 0x5678;
    int z;
    
    /* Pattern that might be split by try_split */
    if (x != 0) {
        goto split_label;
    }
    
    return 0;
    
split_label:
    /* Operation that might be split into simpler instructions */
    z = (x & 0xFF) | ((y >> 8) & 0xFF);  /* Compound but safe operation */
    
    return z;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling pattern tests...\n");
    
    /* Run all test functions and accumulate results */
    total += test_arithmetic_after_label();
    total += test_register_move();
    total += test_bitwise_ops();
    total += test_stack_operation();
    total += test_comparison();
    total += test_multiple_instructions();
    total += test_no_resource_conflict();
    total += test_try_split_pattern();
    
    printf("Total result: %d\n", total);
    printf("(This ensures all code paths are executed and not optimized away)\n");
    
    /* Use result to affect control flow */
    if (total > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure - likely all code was optimized away */
    }
}
