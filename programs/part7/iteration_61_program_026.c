/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
static int __attribute__((optimize("O2"))) test_arithmetic(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target_label1;
    }
    
    /* Dead code to avoid fall-through optimization */
    result = -1;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be safe, non-trapping, and splittable */
    c = a + b;  /* next_trial candidate */
    
    /* Use result to prevent dead code elimination */
    result = c * 2;
    return result;
}

/* Test function 2: Bitwise operations after label */
static int __attribute__((optimize("O2"))) test_bitwise(void) {
    unsigned int x = 0x1234, y = 0x5678, z = 0;
    int counter = 0;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 3; i++) {
        if (x != 0) {
            goto target_label2;
        }
        counter++;
    }
    
    return -1;
    
target_label2:
    /* Candidate: bitwise operation - safe and non-trapping */
    z = x & y;  /* next_trial candidate */
    
    /* Use the result */
    return (int)(z >> 4) + counter;
}

/* Test function 3: Register move pattern */
static int __attribute__((optimize("O3"))) test_register_move(void) {
    int p = 100, q = 200, r = 300;
    int temp;
    
    /* Nested condition to create jump opportunity */
    if (p > 50) {
        if (q < 250) {
            goto target_label3;
        }
    }
    
    return p;
    
target_label3:
    /* Candidate: simple register-to-register move pattern */
    temp = r;  /* next_trial candidate - simple move */
    
    /* Use in computation */
    return temp + p + q;
}

/* Test function 4: Stack-based memory operation */
static int __attribute__((optimize("O2"))) test_stack_ops(void) {
    int local_array[4] = {1, 2, 3, 4};
    int sum = 0;
    int idx = 2;
    
    /* Multiple jumps to same label */
    switch (idx) {
        case 1:
            goto target_label4;
        case 2:
            goto target_label4;
        default:
            break;
    }
    
    return -1;
    
target_label4:
    /* Candidate: stack load operation - should be safe */
    sum = local_array[1];  /* next_trial candidate */
    
    /* Simple use */
    return sum * 10;
}

/* Test function 5: Comparison operation */
static int __attribute__((optimize("O2"))) test_comparison(void) {
    int m = 42, n = 84;
    int cmp_result;
    
    /* Unconditional goto to create simple jump */
    if (m != 0) {
        goto target_label5;
    }
    
    return 0;
    
target_label5:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (m < n);  /* next_trial candidate */
    
    /* Use result */
    return cmp_result ? 100 : 200;
}

/* Test function 6: Multiple safe instructions in sequence */
static int __attribute__((optimize("O3"))) test_multiple_ops(void) {
    volatile int u = 5, v = 7, w = 9;
    int res1, res2;
    
    /* Create jump with simple condition */
    if (u + v > 0) {
        goto target_label6;
    }
    
    return -1;
    
target_label6:
    /* First candidate instruction */
    res1 = v * 2;  /* next_trial candidate */
    
    /* Second instruction (not part of delay slot) */
    res2 = w - u;
    
    return res1 + res2;
}

/* Test function 7: Avoid resource conflicts */
static int __attribute__((optimize("O2"))) test_no_conflict(void) {
    /* Use distinct variables to avoid resource conflicts */
    int jump_var = 1;      /* Used only for jump condition */
    int delay_var1 = 10;   /* Used only in delay candidate */
    int delay_var2 = 20;   /* Used only in delay candidate */
    int output = 0;
    
    /* Simple jump based on jump_var */
    if (jump_var > 0) {
        goto target_label7;
    }
    
    /* Different variables here to avoid conflicts */
    int unrelated = 99;
    return unrelated;
    
target_label7:
    /* Candidate: uses variables not involved in jump condition */
    output = delay_var1 + delay_var2;  /* next_trial candidate */
    
    return output;
}

/* Test function 8: Loop with internal goto */
static int __attribute__((optimize("O3"))) test_loop_goto(void) {
    int total = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            goto loop_label;
        }
        total += i;
    }
    
    return total;
    
loop_label:
    /* Candidate: simple increment - safe and splittable */
    total += 100;  /* next_trial candidate */
    
    /* Continue loop */
    for (; i < 10; i++) {
        total += i;
    }
    
    return total;
}

/* Main function to execute all tests */
int main(void) {
    int total_result = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Execute all test functions */
    total_result += test_arithmetic();
    total_result += test_bitwise();
    total_result += test_register_move();
    total_result += test_stack_ops();
    total_result += test_comparison();
    total_result += test_multiple_ops();
    total_result += test_no_conflict();
    total_result += test_loop_goto();
    
    printf("Total result: %d\n", total_result);
    printf("All tests completed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total_result > 0) {
        return 0;
    } else {
        return 1;
    }
}
