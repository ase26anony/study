/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2")))
#else
#define OPTIMIZE_O2
#endif

/* Test 1: Simple arithmetic after label */
OPTIMIZE_O2
static int test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        goto compute;
    }
    
    /* Dead code to create alternative path */
    result = -1;
    return result;
    
compute:
    /* Candidate for next_trial: simple arithmetic */
    c = a + b;  /* This should be safe to move into delay slot */
    result = c * 2;
    
    return result;
}

/* Test 2: Register move/swap pattern */
OPTIMIZE_O2  
static int test_register_move(void) {
    int x = 5, y = 10, z = 15;
    int temp;
    
    if (x != 0) {
        goto process;
    }
    
    return 0;
    
process:
    /* Candidate: simple register operation */
    temp = y;  /* Move operation - likely safe */
    y = x;
    x = temp;
    
    return x + y + z;
}

/* Test 3: Bitwise operations after label */
OPTIMIZE_O2
static int test_bitwise_ops(void) {
    unsigned int flags = 0x0F;
    unsigned int mask = 0x03;
    unsigned int result = 0;
    
    if (flags & 0x01) {
        goto apply_mask;
    }
    
    return 0;
    
apply_mask:
    /* Candidate: bitwise operation */
    result = flags & mask;  /* Simple bitwise AND - no traps */
    result |= 0x10;
    
    return (int)result;
}

/* Test 4: Stack-based memory operation */
OPTIMIZE_O2
static int test_stack_operation(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 2;
    int value;
    
    if (index < 4) {
        goto load_value;
    }
    
    return -1;
    
load_value:
    /* Candidate: stack load (should be safe) */
    value = array[index];  /* Stack access - shouldn't fault */
    value += 10;
    
    return value;
}

/* Test 5: Comparison operation */
OPTIMIZE_O2
static int test_comparison(void) {
    int a = 100, b = 200;
    int cmp_result;
    
    if (a > 50) {
        goto compare;
    }
    
    return 0;
    
compare:
    /* Candidate: comparison operation */
    cmp_result = (a < b);  /* Sets condition codes */
    return cmp_result ? a : b;
}

/* Test 6: Multiple basic blocks with jumps */
OPTIMIZE_O2
static int test_nested_jumps(void) {
    int i = 0, sum = 0;
    int values[3] = {10, 20, 30};
    
start_loop:
    if (i >= 3) {
        goto done;
    }
    
    if (values[i] > 15) {
        goto process_large;
    }
    
    /* Small value */
    sum += 1;
    i++;
    goto start_loop;
    
process_large:
    /* Candidate: arithmetic on local variable */
    sum += values[i];  /* Simple addition */
    i++;
    goto start_loop;
    
done:
    return sum;
}

/* Test 7: Avoid resource conflicts with distinct variables */
OPTIMIZE_O2
static int test_no_resource_conflict(void) {
    /* Use completely separate variables for jump condition
       and the operation after label to avoid conflicts */
    int jump_cond_var = 1;
    int delay_slot_var1 = 100;
    int delay_slot_var2 = 200;
    int result_var = 0;
    
    if (jump_cond_var > 0) {
        goto safe_operation;
    }
    
    return -1;
    
safe_operation:
    /* Candidate: uses variables not involved in jump condition */
    result_var = delay_slot_var1 + delay_slot_var2;
    return result_var;
}

/* Test 8: Try to trigger try_split with simple pattern */
OPTIMIZE_O2
static int test_simple_split_pattern(void) {
    int x = 5, y = 3;
    int r1, r2, r3;
    
    if (x > y) {
        goto calculate;
    }
    
    return 0;
    
calculate:
    /* Multiple simple operations that might be split */
    r1 = x + y;
    r2 = x - y;
    r3 = r1 * r2;
    
    return r3;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Run all test functions */
    total += test_arithmetic_after_label();
    total += test_register_move();
    total += test_bitwise_ops();
    total += test_stack_operation();
    total += test_comparison();
    total += test_nested_jumps();
    total += test_no_resource_conflict();
    total += test_simple_split_pattern();
    
    printf("Total result: %d\n", total);
    printf("All tests completed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure */
    }
}
