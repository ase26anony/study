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
    
    /* Dead code to create fall-through path */
    result = -1;
    return result;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    /* This should be safe to move into jump's delay slot */
    c = a + b;  /* next_trial: simple arithmetic operation */
    
    /* Use result to prevent elimination */
    result = c * 2;
    return result;
}

/* Test function 2: Register move/assignment after label */
__attribute__((optimize("O2")))
static int test_move_delay(void) {
    int x = 5, y = 15, z = 25;
    volatile int trigger = 1;
    
    if (trigger) {
        goto target_label2;
    }
    
    return -1;
    
target_label2:
    /* Candidate: simple register move/assignment */
    /* Should not conflict with jump resources */
    y = x;  /* next_trial: move operation */
    
    /* Use the result */
    z = y * 3;
    return z;
}

/* Test function 3: Bitwise operation after label */
__attribute__((optimize("O2")))
static int test_bitwise_delay(void) {
    unsigned int flags = 0x0F;
    unsigned int mask = 0x03;
    unsigned int result = 0;
    
    /* Force a jump */
    if (flags != 0) {
        goto target_label3;
    }
    
    return 0;
    
target_label3:
    /* Candidate: bitwise operation */
    /* Should be safe and splittable */
    mask = flags & 0x07;  /* next_trial: bitwise AND */
    
    result = mask | 0x10;
    return (int)result;
}

/* Test function 4: Stack-based memory operation */
__attribute__((optimize("O2")))
static int test_memory_delay(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    volatile int idx = 2;
    
    /* Simple conditional jump */
    if (idx < 4) {
        goto target_label4;
    }
    
    return -1;
    
target_label4:
    /* Candidate: stack memory load (should be safe) */
    /* Using stack address which won't fault */
    temp = array[idx];  /* next_trial: memory load from stack */
    
    return temp * 2;
}

/* Test function 5: Comparison operation */
__attribute__((optimize("O2")))
static int test_compare_delay(void) {
    int val1 = 100, val2 = 200;
    int cmp_result = 0;
    
    /* Unconditional jump */
    goto target_label5;
    
    /* Unreachable */
    return -1;
    
target_label5:
    /* Candidate: comparison operation */
    /* Sets condition codes without side effects */
    cmp_result = (val1 < val2);  /* next_trial: comparison */
    
    return cmp_result ? 10 : 20;
}

/* Test function 6: Multiple operations in sequence */
__attribute__((optimize("O2")))
static int test_sequence_delay(void) {
    int counter = 0;
    int a = 1, b = 2, c = 3;
    
    /* Loop with internal goto to increase optimization chances */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto inner_label;
        }
        counter++;
        continue;
        
    inner_label:
        /* Candidate: arithmetic with different registers */
        a = b + c;  /* next_trial: should be eligible */
        
        /* Use result */
        counter += a;
    }
    
    return counter;
}

/* Test function 7: Avoid resource conflicts explicitly */
__attribute__((optimize("O2")))
static int test_no_conflict_delay(void) {
    /* Use completely separate variables for jump and delay slot */
    volatile int jump_var = 1;
    int delay_var1 = 10;  /* Only used in delay slot */
    int delay_var2 = 20;  /* Only used in delay slot */
    int result = 0;
    
    /* Jump condition uses only jump_var */
    if (jump_var > 0) {
        goto conflict_free_label;
    }
    
    return -1;
    
conflict_free_label:
    /* Candidate: uses variables not involved in jump */
    /* Should pass !insn_references_resource_p checks */
    delay_var1 = delay_var2 * 2;  /* next_trial */
    
    result = delay_var1;
    return result;
}

/* Test function 8: Nested jumps */
__attribute__((optimize("O3")))  /* Higher optimization */
static int test_nested_delay(void) {
    int x = 0, y = 0, z = 0;
    
    /* Multiple basic blocks with jumps */
    for (int i = 0; i < 5; i++) {
        if (i & 1) {
            goto odd_label;
        }
        
        x += i;
        continue;
        
    odd_label:
        /* Candidate instruction after label */
        y = i * 2;  /* next_trial */
        
        z += y;
    }
    
    return x + z;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test_arithmetic_delay();
    total += test_move_delay();
    total += test_bitwise_delay();
    total += test_memory_delay();
    total += test_compare_delay();
    total += test_sequence_delay();
    total += test_no_conflict_delay();
    total += test_nested_delay();
    
    printf("Total result: %d\n", total);
    printf("All delay slot patterns executed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
