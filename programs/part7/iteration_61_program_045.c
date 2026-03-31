/* test_delay_slots.c
 * Designed to trigger delay slot optimization logic in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -c test_delay_slots.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - likely eligible for delay slot */
static int __attribute__((optimize("O2"))) test_arithmetic_delay(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        goto arithmetic_label;
    }
    
    return 0;
    
arithmetic_label:
    /* Candidate for next_trial: simple arithmetic, no traps, can be split */
    c = a + b;  /* Simple add - safe for delay slot */
    result = c;
    
    /* Avoid fall-through issues */
    if (result > 0) {
        return result;
    }
    return 1;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_delay(void) {
    volatile int x = 0x1234, y = 0x5678;
    int mask = 0xFF;
    int res = 0;
    
    /* Create simple jump to label */
    if (x != 0) {
        goto bitwise_label;
    }
    
    return 0;
    
bitwise_label:
    /* Safe bitwise operation - no memory access, no traps */
    res = (x & mask) | (y >> 4);  /* Can be split by try_split */
    
    /* Use result to prevent elimination */
    return res + 1;
}

/* Test 3: Register move/swap pattern */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    volatile int r1 = 100, r2 = 200, r3 = 300;
    int temp;
    
    /* Simple conditional jump */
    if (r1 < r2) {
        goto move_label;
    }
    
move_label:
    /* Simple register move operation - very safe for delay slot */
    temp = r1;
    r1 = r2;
    r2 = temp;
    
    /* Compute return value using modified registers */
    return r1 + r2 + r3;
}

/* Test 4: Stack-based memory operation (load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops(void) {
    volatile int stack_var1 = 42;
    volatile int stack_var2 = 99;
    int local1, local2;
    
    /* Force a jump */
    if (stack_var1 > 0) {
        goto stack_label;
    }
    
stack_label:
    /* Stack load/store - less likely to trap than heap access */
    local1 = stack_var1;      /* Load from stack */
    stack_var2 = local1 + 5;  /* Store to stack */
    
    return stack_var2;
}

/* Test 5: Comparison operation setting condition codes */
static int __attribute__((optimize("O2"))) test_comparison(void) {
    volatile int p = 50, q = 60;
    int cmp_result;
    
    /* Jump to label */
    if (p != q) {
        goto compare_label;
    }
    
compare_label:
    /* Comparison operation - sets condition codes, no side effects */
    cmp_result = (p < q);  /* Simple comparison */
    
    /* Use in control flow to prevent dead code elimination */
    if (cmp_result) {
        return p;
    }
    return q;
}

/* Test 6: Multiple basic blocks with jumps in loop */
static int __attribute__((optimize("O3"))) test_loop_jumps(void) {
    volatile int counter = 0;
    volatile int sum = 0;
    volatile int threshold = 5;
    
    while (counter < threshold) {
        /* Jump to label inside loop */
        if (sum < 100) {
            goto loop_label;
        }
        
        sum += 10;
        continue;
        
    loop_label:
        /* Safe operation in delay slot candidate position */
        sum += counter * 2;  /* Multiplication - can be split */
        counter++;
    }
    
    return sum;
}

/* Test 7: Nested jumps with safe operations */
static int __attribute__((optimize("O2"))) test_nested_jumps(void) {
    volatile int a = 1, b = 2, c = 3;
    int result = 0;
    
    if (a) {
        if (b) {
            goto outer_label;
        }
    }
    
    return 0;
    
outer_label:
    /* First safe operation */
    c = a + b;
    
    /* Another jump to inner label */
    if (c > 0) {
        goto inner_label;
    }
    
inner_label:
    /* Another delay slot candidate */
    result = b << 2;  /* Shift operation */
    
    return result + c;
}

/* Test 8: Avoid resource conflicts by using fresh variables */
static int __attribute__((optimize("O2"))) test_no_conflict(void) {
    /* Variables for jump condition */
    volatile int cond_a = 10, cond_b = 20;
    
    /* Separate variables for delay slot operation */
    volatile int ds_x = 30, ds_y = 40;
    int ds_result;
    
    /* Jump using only condition variables */
    if (cond_a < cond_b) {
        goto no_conflict_label;
    }
    
no_conflict_label:
    /* Operation uses completely different variables - no resource conflict */
    ds_result = ds_x * ds_y;  /* Uses different resources than jump */
    
    return ds_result;
}

/* Test 9: Mixed operations that try_split can handle */
static int __attribute__((optimize("O2"))) test_mixed_ops(void) {
    volatile int v1 = 5, v2 = 7, v3 = 9;
    int r1, r2;
    
    if (v1 > 0) {
        goto mixed_label;
    }
    
mixed_label:
    /* Multiple simple operations in sequence */
    r1 = v1 + v2;      /* Addition */
    r2 = v3 - v1;      /* Subtraction */
    r1 = r1 & 0xFF;    /* Bitwise AND */
    
    return r1 + r2;
}

/* Test 10: Function with multiple candidate patterns */
static int __attribute__((optimize("O3"))) test_multiple_patterns(void) {
    volatile int base = 100;
    int total = 0;
    
    /* Pattern A: Arithmetic */
    if (base > 50) {
        goto pattern_a;
    }
    
pattern_a:
    total += base + 10;
    
    /* Pattern B: Bitwise */
    if (total < 200) {
        goto pattern_b;
    }
    
pattern_b:
    total = total ^ 0x55;  /* XOR operation */
    
    /* Pattern C: Comparison */
    if (total != 0) {
        goto pattern_c;
    }
    
pattern_c:
    /* Final simple operation */
    total = total > 0 ? total : -total;
    
    return total;
}

/* Main function that executes all tests */
int main(void) {
    int total_result = 0;
    
    /* Execute all test functions */
    total_result += test_arithmetic_delay();
    total_result += test_bitwise_delay();
    total_result += test_register_move();
    total_result += test_stack_ops();
    total_result += test_comparison();
    total_result += test_loop_jumps();
    total_result += test_nested_jumps();
    total_result += test_no_conflict();
    total_result += test_mixed_ops();
    total_result += test_multiple_patterns();
    
    /* Print result to ensure all code paths are executed */
    printf("Total result: %d\n", total_result);
    
    /* Also use volatile to prevent over-optimization */
    volatile int check = total_result;
    if (check > 0) {
        return 0;
    }
    return 1;
}
