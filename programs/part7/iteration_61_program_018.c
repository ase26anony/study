/* test_delay_slot.c
 * Designed to trigger delay slot optimization logic in reorg.cc lines 2135-2149
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
    
    if (a < b) {
        goto arith_label;
    }
    
    return 0;
    
arith_label:
    /* Candidate for delay slot: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c * 2;
    
    /* Prevent fall-through to other labels */
    goto end_arith;
    
end_arith:
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    volatile uint32_t x = 0x12345678;
    volatile uint32_t y = 0x87654321;
    uint32_t z = 0;
    int count = 0;
    
    for (int i = 0; i < 3; i++) {
        if (x != y) {
            goto bitwise_label;
        }
        count++;
    }
    
    return count;
    
bitwise_label:
    /* Candidate: bitwise operation - unlikely to trap */
    z = x ^ y;  /* XOR operation */
    z = z | 0x1; /* OR operation */
    
    /* Use result to prevent elimination */
    return (int)(z & 0xFF) + count;
}

/* Test 3: Stack-based memory operation (safe load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops_after_label(void) {
    volatile int arr[4] = {1, 2, 3, 4};
    volatile int temp = 0;
    int sum = 0;
    
    /* Create multiple basic blocks to encourage reorg */
    if (arr[0] > 0) {
        goto stack_label1;
    } else {
        goto stack_label2;
    }

stack_label1:
    /* Candidate: stack load operation */
    temp = arr[1];  /* Load from stack - safe */
    goto merge_point;

stack_label2:
    /* Another candidate: stack store operation */
    arr[2] = 5;  /* Store to stack - safe */
    temp = arr[2];
    goto merge_point;

merge_point:
    sum = temp + arr[0];
    return sum;
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_compare_after_label(void) {
    volatile int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Nested condition to create jump opportunities */
    if (p > 50) {
        if (q < 300) {
            goto compare_label;
        }
    }
    
    return -1;
    
compare_label:
    /* Candidate: comparison operation (sets condition codes) */
    cmp_result = (p < q) ? 1 : 0;  /* Comparison only */
    
    /* Use different variables to avoid resource conflicts */
    int r = p + q;
    return cmp_result + r;
}

/* Test 5: Multiple safe instructions in sequence */
static int __attribute__((optimize("O3"))) test_multi_ops_after_label(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int r1 = 0, r2 = 0;
    
    /* Loop with internal goto to increase optimization chances */
    for (int i = 0; i < 2; i++) {
        if (v1 + i > v2) {
            goto multi_label;
        }
    }
    
    return 0;
    
multi_label:
    /* Multiple simple instructions - one should be eligible */
    r1 = v1 + v2;    /* Simple add */
    r2 = v3 & v4;    /* Bitwise AND */
    
    /* Prevent dead code elimination */
    volatile int sink = r1 + r2;
    (void)sink;
    
    return r1 - r2;
}

/* Test 6: Register move pattern */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    volatile int src = 42;
    int dst1 = 0, dst2 = 0;
    
    /* Unconditional goto to create simple jump */
    if (src != 0) {
        goto move_label;
    }
    
    return 0;
    
move_label:
    /* Candidate: register-to-register move pattern */
    dst1 = src;      /* Move operation */
    dst2 = dst1 + 1; /* Follow-up operation */
    
    return dst1 + dst2;
}

/* Test 7: Shift operations (safe, no trapping) */
static int __attribute__((optimize("O2"))) test_shift_ops(void) {
    volatile unsigned int val = 0x100;
    unsigned int shifted = 0;
    
    /* Multiple conditional jumps */
    if (val > 0) {
        goto shift_label;
    } else if (val == 0) {
        return 0;
    } else {
        return 1;
    }

shift_label:
    /* Candidate: shift operations */
    shifted = val << 2;  /* Left shift */
    shifted = shifted >> 1; /* Right shift */
    
    return (int)shifted;
}

#pragma GCC pop_options

/* Main function that executes all tests */
int main(void) {
    int total_result = 0;
    
    printf("Running delay slot optimization tests...\n");
    
    /* Execute all test functions */
    total_result += test_arithmetic_after_label();
    total_result += test_bitwise_after_label();
    total_result += test_stack_ops_after_label();
    total_result += test_compare_after_label();
    total_result += test_multi_ops_after_label();
    total_result += test_register_move();
    total_result += test_shift_ops();
    
    printf("Total result: %d\n", total_result);
    printf("All tests completed. Check if delay slot optimization was triggered.\n");
    
    /* Use result to prevent dead code elimination */
    if (total_result > 1000) {
        printf("Unexpected large result\n");
    }
    
    return total_result & 0xFF;  /* Return non-zero to indicate execution */
}
