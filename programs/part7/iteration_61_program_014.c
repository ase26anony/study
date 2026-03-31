/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
static int test_arithmetic_delay(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target_label1;
    }
    
    /* This should be dead code */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This is next_trial - should be movable into delay slot */
    c = a + b + 5;  /* Simple integer arithmetic */
    
    /* Use result to prevent elimination */
    result = c * 2;
    return result;
}

/* Test function 2: Register move/swap pattern */
__attribute__((optimize("O2")))
static int test_register_move(void) {
    int x = 5, y = 10, z = 15;
    int temp;
    
    /* Force a jump */
    if (x != 0) {
        goto move_label;
    }
    
    return -1;
    
move_label:
    /* Candidate: register move operation */
    temp = y;  /* Simple move - likely becomes register transfer */
    y = x;
    x = temp;
    
    /* Use all variables to prevent optimization */
    return x + y + z;
}

/* Test function 3: Bitwise operations after label */
__attribute__((optimize("O2")))
static int test_bitwise_ops(void) {
    unsigned int flags = 0x0F;
    unsigned int mask = 0x33;
    unsigned int result = 0;
    
    /* Create jump with simple condition */
    if (flags != 0) {
        goto bitwise_label;
    }
    
    return 0;
    
bitwise_label:
    /* Candidate: bitwise operations (no trapping) */
    result = flags & mask;  /* Safe bitwise operation */
    result = result | 0x40;
    result = result ^ 0x11;
    
    return (int)result;
}

/* Test function 4: Safe stack memory operation */
__attribute__((optimize("O2")))
static int test_stack_operation(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 2;
    int value;
    
    /* Jump to label */
    if (array[0] > 0) {
        goto load_label;
    }
    
    return -1;
    
load_label:
    /* Candidate: stack-based load (shouldn't trap) */
    value = array[index];  /* Stack access - safe */
    
    /* Simple operation on loaded value */
    value = value * 2 + 1;
    
    return value;
}

/* Test function 5: Comparison operation */
__attribute__((optimize("O2")))
static int test_comparison(void) {
    int p = 100, q = 200;
    int cmp_result;
    
    /* Force jump */
    if (p < q) {
        goto compare_label;
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (p < q);  /* Comparison operation */
    
    /* Use in calculation */
    return cmp_result ? p : q;
}

/* Test function 6: Multiple basic blocks with jumps */
__attribute__((optimize("O2")))
static int test_multiple_blocks(void) {
    int counter = 0;
    int i;
    
    /* Loop with internal goto to create multiple jump opportunities */
    for (i = 0; i < 10; i++) {
        if (i & 1) {
            goto odd_label;
        }
        
        /* Even case */
        counter += i * 2;
        continue;
        
    odd_label:
        /* Candidate: simple increment (safe, no traps) */
        counter += i + 1;  /* Simple arithmetic after label */
    }
    
    return counter;
}

/* Test function 7: Nested jumps with safe operations */
__attribute__((optimize("O2")))
static int test_nested_jumps(void) {
    int a = 1, b = 2, c = 3;
    int result = 0;
    
    /* First level jump */
    if (a > 0) {
        goto outer_label;
    }
    
    return -1;
    
outer_label:
    /* Simple operation that could be in delay slot */
    result = b * 3;
    
    /* Another jump */
    if (result > 0) {
        goto inner_label;
    }
    
    return result;
    
inner_label:
    /* Another candidate for delay slot */
    c = result + a;
    
    return c;
}

/* Test function 8: Avoid resource conflicts */
__attribute__((optimize("O2")))
static int test_no_conflict(void) {
    /* Use distinct variables to avoid resource conflicts */
    int jump_var = 10;      /* Used only for jump decision */
    int delay_var1 = 20;    /* Used only after label */
    int delay_var2 = 30;    /* Another distinct variable */
    int output = 0;
    
    /* Jump based on jump_var */
    if (jump_var > 5) {
        goto safe_label;
    }
    
    return 0;
    
safe_label:
    /* Candidate: uses variables not involved in jump condition */
    /* This minimizes resource conflicts with &set and &needed */
    output = delay_var1 + delay_var2;
    output = output * 2;
    
    return output;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Run all test functions and accumulate results */
    total += test_arithmetic_delay();
    total += test_register_move();
    total += test_bitwise_ops();
    total += test_stack_operation();
    total += test_comparison();
    total += test_multiple_blocks();
    total += test_nested_jumps();
    total += test_no_conflict();
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure */
    }
}
