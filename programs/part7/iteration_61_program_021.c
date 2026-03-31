/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch test_delay_slot.c -o test
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 test_delay_slot.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - most likely candidate */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    if (a < b) {
        goto target_label1;
    }
    
    /* This should be dead code, but ensures the goto is conditional */
    result = -1;
    return result;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic, no side effects */
    c = a + b;  /* This should be movable into delay slot */
    result = c * 2;
    
    /* Prevent tail call optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O2"))) test_bitwise_after_label(void) {
    volatile int x = 0x55AA55AA;
    volatile int y = 0x00FF00FF;
    int z = 0;
    int ret = 0;
    
    /* Create a simple jump condition */
    if (x != 0) {
        goto bitwise_label;
    }
    
    return -1;
    
bitwise_label:
    /* Candidate: bitwise operation, no memory access */
    z = x & y;  /* Safe operation, no trapping */
    ret = z | 0x01010101;
    
    /* Use result to prevent elimination */
    asm volatile("" : "+r"(ret));
    return ret;
}

/* Test 3: Stack-based memory operation (load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops_after_label(void) {
    int local1 = 100;
    int local2 = 200;
    int local3 = 0;
    int local4 = 0;
    int sum = 0;
    
    /* Use different variables for condition to avoid conflicts */
    int cond_var = local1;
    
    if (cond_var > 50) {
        goto stack_label;
    }
    
    return -1;
    
stack_label:
    /* Candidate: stack variable operations - safe from faults */
    local3 = local1 + local2;  /* Load and arithmetic */
    local4 = local3 * 2;       /* Another operation */
    sum = local3 + local4;
    
    /* Ensure the operations aren't optimized away */
    asm volatile("" : "+r"(sum));
    return sum;
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_comparison_after_label(void) {
    volatile int p = 1000;
    volatile int q = 2000;
    int cmp_result = 0;
    int final = 0;
    
    /* Force a jump */
    if (p != q) {
        goto compare_label;
    }
    
    return -1;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (p < q);  /* Comparison operation */
    final = cmp_result ? p : q;
    
    asm volatile("" : "+r"(final));
    return final;
}

/* Test 5: Multiple operations in sequence after label */
static int __attribute__((optimize("O2"))) test_sequence_after_label(void) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int t1, t2, t3;
    int out = 0;
    
    /* Complex condition to ensure jump is generated */
    if ((v1 + v2) < (v3 + v4)) {
        goto sequence_label;
    }
    
    return -1;
    
sequence_label:
    /* Multiple simple operations - try_split should handle these */
    t1 = v1 + v2;    /* First operation */
    t2 = v3 - v4;    /* Second operation */
    t3 = t1 * t2;    /* Third operation */
    out = t3 >> 2;   /* Fourth operation */
    
    asm volatile("" : "+r"(out));
    return out;
}

/* Test 6: Loop with internal goto to increase optimization opportunities */
static int __attribute__((optimize("O2"))) test_loop_with_goto(void) {
    int i;
    int accumulator = 0;
    int temp = 0;
    
    for (i = 0; i < 10; i++) {
        if (i & 1) {  /* Jump on odd numbers */
            goto loop_label;
        }
        accumulator += i;
        continue;
        
    loop_label:
        /* Candidate inside loop: simple operation */
        temp = i * 2;  /* Safe operation */
        accumulator += temp;
    }
    
    return accumulator;
}

/* Test 7: Nested conditions with simple operations */
static int __attribute__((optimize("O2"))) test_nested_conditions(void) {
    int a = 5, b = 10, c = 15;
    int r1 = 0, r2 = 0;
    
    if (a < b) {
        if (b < c) {
            goto nested_label;
        }
        return -1;
    }
    return -2;
    
nested_label:
    /* Very simple operation to maximize eligibility */
    r1 = a + 1;      /* Increment operation */
    r2 = r1 + b;     /* Another simple operation */
    
    asm volatile("" : "+r"(r2));
    return r2;
}

/* Test 8: Switch-like construct with goto labels */
static int __attribute__((optimize("O2"))) test_switch_goto(void) {
    int selector = 2;
    int val1 = 10, val2 = 20, val3 = 30;
    int result = 0;
    
    if (selector == 1) goto case1;
    if (selector == 2) goto case2;
    if (selector == 3) goto case3;
    goto default_case;
    
case1:
    result = val1 + 5;
    goto end;
    
case2:
    /* Candidate in switch case */
    result = val2 * 2;  /* Simple multiplication */
    goto end;
    
case3:
    result = val3 - 5;
    goto end;
    
default_case:
    result = -1;
    
end:
    return result;
}

#pragma GCC pop_options

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_stack_ops_after_label();
    total += test_comparison_after_label();
    total += test_sequence_after_label();
    total += test_loop_with_goto();
    total += test_nested_conditions();
    total += test_switch_goto();
    
    /* Print result to ensure all code is executed */
    printf("Total result: %d\n", total);
    
    /* Verify expected total to ensure all paths were taken */
    if (total != 0) {
        printf("All tests executed successfully\n");
    }
    
    return 0;
}
