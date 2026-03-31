/* Test program for delay slot filling optimization targeting specific uncovered lines in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
static int test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto arith_label;
    }
    
    /* This should be dead code, but provides alternative path */
    result = 100;
    return result;
    
arith_label:
    /* Candidate for delay slot filling: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c * 2;
    
    /* Prevent tail call optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* Test function 2: Register move operation after label */
__attribute__((optimize("O2")))
static int test_register_move(void) {
    int x = 5, y = 0, z = 0;
    volatile int trigger = 1;
    
    /* Force a simple jump */
    if (trigger != 0) {
        goto move_label;
    }
    
    y = 99;
    return y;
    
move_label:
    /* Candidate: simple register move/assignment */
    y = x;  /* next_trial: move instruction */
    z = y + 3;
    
    /* Use result to prevent elimination */
    asm volatile("" : "+r"(z));
    return z;
}

/* Test function 3: Bitwise operation after label with loop */
__attribute__((optimize("O2")))
static int test_bitwise_in_loop(void) {
    int i, counter = 0;
    int mask = 0xFF;
    int value = 0x55;
    
    for (i = 0; i < 10; i++) {
        /* Create jump within loop */
        if (i & 1) {
            goto bitwise_label;
        }
        
        counter++;
        continue;
        
    bitwise_label:
        /* Candidate: bitwise operation */
        value = value & mask;  /* next_trial: and instruction */
        counter += value;
    }
    
    return counter;
}

/* Test function 4: Stack-based memory operation (safe load/store) */
__attribute__((optimize("O2")))
static int test_stack_operation(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    volatile int selector = 2;
    
    /* Simple conditional jump */
    if (selector > 0) {
        goto mem_label;
    }
    
    temp = array[0];
    return temp;
    
mem_label:
    /* Candidate: stack memory load (shouldn't trap) */
    temp = array[selector];  /* next_trial: load instruction */
    
    /* Also store to avoid being pure load */
    array[0] = temp + 1;
    
    return temp + array[0];
}

/* Test function 5: Comparison operation after label */
__attribute__((optimize("O2")))
static int test_comparison(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    volatile int flag = 1;
    
    /* Unconditional jump disguised as conditional */
    if (flag) {
        goto cmp_label;
    }
    
    return -1;
    
cmp_label:
    /* Candidate: comparison operation */
    cmp_result = (p < q);  /* next_trial: compare instruction */
    
    /* Use the comparison result */
    return cmp_result ? p : q;
}

/* Test function 6: Multiple basic blocks with jumps */
__attribute__((optimize("O2")))
static int test_nested_jumps(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* First jump */
    if (a < b) {
        goto first_label;
    }
    
    result = 50;
    goto end;
    
first_label:
    /* First candidate instruction */
    c = a + b;  /* Potential next_trial */
    
    /* Another jump to create more opportunities */
    if (c > 0) {
        goto second_label;
    }
    
    result = c;
    goto end;
    
second_label:
    /* Second candidate instruction */
    d = c * 2;  /* Another potential next_trial */
    result = d;
    
end:
    return result;
}

/* Test function 7: Avoid resource conflicts by using fresh variables */
__attribute__((optimize("O2")))
static int test_no_resource_conflict(void) {
    /* Variables for jump condition - in different "resource set" */
    volatile int cond_a = 10;
    volatile int cond_b = 20;
    
    /* Separate variables for the delay slot candidate */
    int slot_x, slot_y, slot_z;
    
    if (cond_a < cond_b) {
        goto safe_label;
    }
    
    return 0;
    
safe_label:
    /* Candidate: uses completely different variables */
    slot_x = 30;
    slot_y = 40;
    slot_z = slot_x + slot_y;  /* next_trial: independent operation */
    
    return slot_z;
}

/* Test function 8: Try to trigger try_split with simple pattern */
__attribute__((optimize("O2")))
static int test_splittable_pattern(void) {
    int v1 = 100;
    int v2 = 200;
    int v3 = 300;
    int r1, r2;
    
    /* Force predictable jump */
    asm volatile("" : "+r"(v1));
    
    if (v1 > 0) {
        goto split_label;
    }
    
    return v1;
    
split_label:
    /* Pattern that might be split: compound operation */
    r1 = v1 + v2;      /* Part 1 */
    r2 = r1 - v3;      /* Part 2 - might be split */
    
    /* Make both results live */
    asm volatile("" : "+r"(r1), "+r"(r2));
    return r1 + r2;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Run all test functions and accumulate results */
    total += test_arithmetic_after_label();
    total += test_register_move();
    total += test_bitwise_in_loop();
    total += test_stack_operation();
    total += test_comparison();
    total += test_nested_jumps();
    total += test_no_resource_conflict();
    total += test_splittable_pattern();
    
    /* Print result to ensure code isn't optimized away */
    printf("Total result: %d\n", total);
    
    /* Also use volatile to force compiler to generate code */
    volatile int check = total;
    if (check > 1000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
