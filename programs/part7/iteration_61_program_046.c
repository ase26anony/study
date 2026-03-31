/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
static int test_arithmetic_delay(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create control flow with goto to label */
    if (a < b) {
        goto compute;
    }
    
    /* Dead code path to create jump opportunity */
    result = -1;
    return result;
    
compute:
    /* This instruction should be candidate for delay slot filling */
    /* Simple arithmetic that doesn't trap */
    c = a + b + 5;  /* next_trial candidate */
    
    /* Use result to prevent elimination */
    result = c * 2;
    return result;
}

/* Test function 2: Bitwise operations after label */
__attribute__((optimize("O2")))
static int test_bitwise_delay(void) {
    unsigned int x = 0xABCD, y = 0x1234;
    unsigned int mask = 0xFF;
    int res = 0;
    
    /* Loop with internal goto to create jump */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto process;
        }
        x = x ^ 0x1111;  /* Modify x in loop */
    }
    
    return -1;
    
process:
    /* Safe bitwise operation - no trapping */
    mask = (x & y) | 0x1;  /* next_trial candidate */
    
    res = mask + i;
    return res;
}

/* Test function 3: Stack-based memory operation */
__attribute__((optimize("O2")))
static int test_memory_delay(void) {
    int local1 = 100;
    int local2 = 200;
    int local3 = 300;
    int temp;
    
    /* Conditional jump to label */
    if (local1 != 0) {
        goto safe_op;
    }
    
    local3 = -1;
    return local3;
    
safe_op:
    /* Stack load/store - less likely to trap */
    temp = local2;      /* next_trial candidate - register move */
    local1 = temp + 50;
    
    return local1 + local3;
}

/* Test function 4: Comparison operation */
__attribute__((optimize("O2")))
static int test_compare_delay(void) {
    int p = 42, q = 84, r = 126;
    int cmp_result;
    
    /* Nested condition with goto */
    if (p > 0) {
        if (q < 100) {
            goto compare_label;
        }
    }
    
    return p;
    
compare_label:
    /* Comparison sets condition codes without side effects */
    cmp_result = (r > q);  /* next_trial candidate */
    
    /* Use result in computation */
    return (cmp_result ? p + q : r);
}

/* Test function 5: Multiple safe operations in sequence */
__attribute__((optimize("O3")))  /* Higher optimization */
static int test_multi_ops_delay(void) {
    volatile int counter = 0;
    int a = 1, b = 2, c = 3, d = 4;
    int sum = 0;
    
    /* Create predictable pattern */
    for (int i = 0; i < 10; i++) {
        counter++;
        if (counter == 5) {
            goto compute_block;
        }
        a = a * 2;  /* Modify in loop */
    }
    
    return sum;
    
compute_block:
    /* Sequence of simple, safe operations */
    b = a + 1;      /* First candidate */
    c = b << 2;     /* Second candidate */
    d = c ^ 0xF;    /* Third candidate */
    
    sum = a + b + c + d;
    return sum;
}

/* Test function 6: Avoid resource conflicts explicitly */
__attribute__((optimize("O2")))
static int test_no_conflict_delay(void) {
    /* Use distinct variables to avoid resource conflicts */
    int jump_var = 10;      /* Used only for jump condition */
    int delay_var1 = 20;    /* Used only in delay candidate */
    int delay_var2 = 30;    /* Another distinct variable */
    int output = 0;
    
    /* Simple jump based on jump_var */
    if (jump_var > 0) {
        goto delay_slot_candidate;
    }
    
    return -1;
    
delay_slot_candidate:
    /* Uses completely different variables than jump condition */
    delay_var1 = delay_var2 + 5;  /* next_trial candidate */
    
    output = delay_var1 * 2;
    return output;
}

/* Test function 7: Loop with multiple jump opportunities */
__attribute__((optimize("O2")))
static int test_loop_jumps_delay(void) {
    int array[4] = {1, 2, 3, 4};
    int total = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Create multiple jump points */
        if (array[i] == 2) {
            goto handle_two;
        }
        if (array[i] == 3) {
            goto handle_three;
        }
        total += array[i];
        continue;
        
    handle_two:
        /* Safe operation after label */
        array[i] = array[i] * 2;  /* next_trial candidate */
        total += array[i];
        continue;
        
    handle_three:
        /* Another safe operation */
        array[i] = array[i] + 10;  /* next_trial candidate */
        total += array[i];
        continue;
    }
    
    return total;
}

/* Main function to execute all tests */
int main(void) {
    int total_result = 0;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute all test functions */
    total_result += test_arithmetic_delay();
    total_result += test_bitwise_delay();
    total_result += test_memory_delay();
    total_result += test_compare_delay();
    total_result += test_multi_ops_delay();
    total_result += test_no_conflict_delay();
    total_result += test_loop_jumps_delay();
    
    printf("Total result: %d\n", total_result);
    printf("All tests executed. Compile with appropriate flags for delay slot architecture.\n");
    
    /* Suggested compilation flags:
     * For MIPS: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -fdump-rtl-reorg this_file.c
     * For RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer this_file.c
     * For debugging: Add -fdump-rtl-all to see RTL dumps
     */
    
    return total_result != 0 ? 0 : 1;
}
