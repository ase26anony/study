/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto arithmetic_label;
    }
    
    return 0;
    
arithmetic_label:
    /* Candidate for delay slot filling: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c * 2;
    
    /* Prevent fall-through issues */
    if (result > 0) {
        return result;
    }
    return 1;
}

/* Test 2: Register move/bitwise operation after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    uint32_t x = 0x12345678;
    uint32_t y = 0x87654321;
    uint32_t z = 0;
    int count = 0;
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < 3; i++) {
        if (x != y) {
            goto bitwise_label;
        }
        count++;
    }
    
    return count;
    
bitwise_label:
    /* Candidate: bitwise operation - unlikely to trap */
    z = x & y;  /* next_trial: and instruction */
    
    /* Use result to prevent elimination */
    return (int)(z >> 16) + count;
}

/* Test 3: Stack-based memory operation (safe load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops_after_label(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    int sum = 0;
    
    /* Simple condition to trigger jump */
    if (array[0] == 1) {
        goto stack_label;
    }
    
    return 0;
    
stack_label:
    /* Candidate: stack load operation - generally safe */
    temp = array[2];  /* next_trial: load from stack */
    sum = temp + array[3];
    
    /* Different path to avoid simple fall-through */
    switch (sum % 3) {
        case 0: return sum + 1;
        case 1: return sum + 2;
        default: return sum + 3;
    }
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_comparison_after_label(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Nested conditions to create interesting flow */
    if (p > 50) {
        if (q < 300) {
            goto compare_label;
        }
    }
    
    return -1;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (p < q);  /* next_trial: compare/setcc */
    
    /* Use in conditional return */
    return cmp_result ? 100 : 200;
}

/* Test 5: Multiple safe instructions in sequence */
static int __attribute__((optimize("O3"))) test_multi_ops_after_label(void) {
    volatile int m = 5, n = 7;
    int r1 = 0, r2 = 0;
    
    /* Unconditional goto to create simple jump */
    if (m > 0) {
        goto multi_label;
    }
    
    return 0;
    
multi_label:
    /* First simple instruction - candidate for delay slot */
    r1 = m * 2;  /* next_trial: multiply/shift */
    
    /* Additional instructions to create basic block */
    r2 = n + 3;
    
    return r1 + r2;
}

/* Test 6: Avoid resource conflicts with distinct variables */
static int __attribute__((optimize("O2"))) test_no_conflict_after_label(void) {
    /* Use distinct variables to avoid resource conflicts */
    int jump_var = 42;      /* Used in jump condition only */
    int delay_var1 = 100;   /* Used in delay candidate only */
    int delay_var2 = 200;   /* Used in delay candidate only */
    int result_var = 0;     /* Result variable */
    
    /* Jump based on jump_var only */
    if (jump_var > 40) {
        goto noconflict_label;
    }
    
    return 0;
    
noconflict_label:
    /* Candidate uses completely different variables */
    result_var = delay_var1 - delay_var2;  /* next_trial: subtract */
    
    /* Ensure result is used */
    return result_var > 0 ? result_var : -result_var;
}

/* Test 7: Try split pattern with simple expression */
static int __attribute__((optimize("O2"))) test_splittable_after_label(void) {
    int base = 1000;
    int offset = 123;
    int index = 4;
    
    /* Multiple conditions to encourage jump optimization */
    if (base > 500 && offset < 200 && index < 10) {
        goto split_label;
    }
    
    return 0;
    
split_label:
    /* Expression that might be split into multiple instructions */
    int addr = base + offset * index;  /* next_trial: multiply-add */
    
    return addr & 0xFF;
}

#pragma GCC pop_options

/* Main function to execute all tests and ensure code runs */
int main(void) {
    int total = 0;
    
    printf("Testing delay slot optimization patterns...\n");
    
    /* Run all test functions */
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_stack_ops_after_label();
    total += test_comparison_after_label();
    total += test_multi_ops_after_label();
    total += test_no_conflict_after_label();
    total += test_splittable_after_label();
    
    printf("Total result: %d\n", total);
    printf("(This ensures all code paths are executed and not optimized away)\n");
    
    return total > 0 ? 0 : 1;
}
