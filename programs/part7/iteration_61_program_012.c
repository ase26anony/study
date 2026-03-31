/* Test program to trigger delay slot optimization conditions in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((noinline))
static int test_arithmetic_delay(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        /* Create a simple jump to label */
        goto target_label1;
    }
    
    /* Dead code to create separation */
    result = a + b + c;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be safe, non-trapping, and splittable */
    c = a + b;  /* next_trial: simple arithmetic operation */
    result = c * 2;
    return result;
}

/* Test function 2: Bitwise operations after label */
__attribute__((noinline))
static int test_bitwise_delay(void) {
    volatile int x = 0x55AA55AA, y = 0xAA55AA55;
    int mask = 0xFF;
    int output = 0;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 2; i++) {
        if (x != y) {
            goto bitwise_label;
        }
        output += i;
    }
    
    return output;
    
bitwise_label:
    /* Candidate: bitwise operation - safe and non-trapping */
    mask = x & y;  /* next_trial: bitwise AND */
    output = mask | 0x1;
    return output;
}

/* Test function 3: Register move pattern */
__attribute__((noinline))
static int test_move_delay(void) {
    volatile int src1 = 100, src2 = 200;
    int temp1, temp2, temp3;
    
    /* Multiple conditions to create jump opportunity */
    if (src1 > 50 && src2 < 300) {
        goto move_target;
    }
    
    temp1 = src1 + src2;
    return temp1;
    
move_target:
    /* Candidate: simple register-to-register move pattern */
    temp2 = src1;    /* next_trial: move operation */
    temp3 = temp2 + 1;
    return temp3;
}

/* Test function 4: Stack-based memory operation */
__attribute__((noinline))
static int test_memory_delay(void) {
    volatile int array[4] = {1, 2, 3, 4};
    int local1 = 5, local2 = 6;
    int sum = 0;
    
    /* Nested condition to create jump */
    if (array[0] == 1) {
        if (local1 < local2) {
            goto memory_label;
        }
    }
    
    sum = array[0] + array[1];
    return sum;
    
memory_label:
    /* Candidate: stack memory load - should be safe */
    local1 = array[2];  /* next_trial: memory load from stack */
    sum = local1 + array[3];
    return sum;
}

/* Test function 5: Comparison operation */
__attribute__((noinline))
static int test_compare_delay(void) {
    volatile int p = 42, q = 24;
    int cmp_result;
    
    /* Simple jump condition */
    if (p != q) {
        goto compare_label;
    }
    
    return p + q;
    
compare_label:
    /* Candidate: comparison operation - sets condition codes */
    cmp_result = (p > q) ? 1 : 0;  /* next_trial: comparison */
    return cmp_result + p;
}

/* Test function 6: Multiple safe operations in sequence */
__attribute__((noinline))
static int test_sequence_delay(void) {
    volatile int counter = 0;
    int a = 1, b = 2, c = 3;
    
    /* Loop with internal goto */
    while (counter < 3) {
        if (counter == 1) {
            goto sequence_label;
        }
        counter++;
    }
    
    return a + b + c;
    
sequence_label:
    /* Multiple simple operations - first one is candidate */
    a = b + c;      /* next_trial: first simple operation */
    b = a * 2;
    c = b - 1;
    return a + b + c;
}

/* Test function 7: Avoid resource conflicts */
__attribute__((noinline))
static int test_no_conflict_delay(void) {
    /* Use completely separate variables for jump condition
       and delay slot candidate to avoid resource conflicts */
    volatile int jump_var = 10;
    int delay_var1 = 20, delay_var2 = 30;  /* Only used after label */
    int result = 0;
    
    /* jump_var only used here, delay_vars not used yet */
    if (jump_var > 5) {
        goto no_conflict_label;
    }
    
    result = jump_var * 2;
    return result;
    
no_conflict_label:
    /* Candidate: uses variables not referenced before the jump */
    delay_var1 = delay_var2 + 5;  /* next_trial: no conflict with jump */
    result = delay_var1 * 3;
    return result;
}

/* Test function 8: Shift operations */
__attribute__((noinline))
static int test_shift_delay(void) {
    volatile int base = 256;
    int shift_amount = 2;
    int shifted;
    
    if (base >= 128) {
        goto shift_label;
    }
    
    return base;
    
shift_label:
    /* Candidate: shift operation - safe and non-trapping */
    shifted = base >> shift_amount;  /* next_trial: shift operation */
    return shifted + 1;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test_arithmetic_delay();
    total += test_bitwise_delay();
    total += test_move_delay();
    total += test_memory_delay();
    total += test_compare_delay();
    total += test_sequence_delay();
    total += test_no_conflict_delay();
    total += test_shift_delay();
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    /* Also use volatile to ensure all code paths are considered */
    volatile int check = total;
    if (check > 0) {
        return 0;
    }
    return 1;
}
