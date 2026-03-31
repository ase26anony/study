/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - likely eligible for delay slot */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    /* Use goto to create a simple jump to label */
    if (a < b) {
        goto arith_label;
    }
    
    /* Dead code to avoid fall-through optimization */
    result = -1;
    return result;
    
arith_label:
    /* Candidate for next_trial: simple arithmetic that doesn't trap */
    c = a + b;  /* Should be safe to move into delay slot */
    result = c * 2;
    
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    unsigned int x = 0xABCD, y = 0x1234, z = 0;
    int counter = 0;
    
    for (int i = 0; i < 3; i++) {
        counter++;
        if (counter > 1) {
            goto bitwise_label;
        }
    }
    
    return -1;
    
bitwise_label:
    /* Candidate: bitwise operations are safe and non-trapping */
    z = x & y;      /* Should be splittable by try_split */
    z = z | 0xFF;   /* Another safe operation */
    
    return (int)z;
}

/* Test 3: Safe stack-based memory operation */
static int __attribute__((optimize("O2"))) test_stack_ops_after_label(void) {
    int arr[4] = {1, 2, 3, 4};
    int temp = 0;
    int idx = 2;
    
    /* Create multiple basic blocks to encourage reorg */
    switch (idx) {
        case 1: temp = 10; break;
        case 2: goto mem_label;
        case 3: temp = 30; break;
        default: return -1;
    }
    
    return temp;
    
mem_label:
    /* Candidate: stack load - unlikely to trap */
    temp = arr[2];  /* Safe array access within bounds */
    arr[3] = temp + 1;  /* Safe store to stack */
    
    return temp + arr[3];
}

/* Test 4: Comparison operations that set condition codes */
static int __attribute__((optimize("O2"))) test_comparison_after_label(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Nested condition to create jump opportunity */
    if (p != 0) {
        if (q > 50) {
            goto cmp_label;
        }
    }
    
    return 0;
    
cmp_label:
    /* Candidate: comparison operation - sets flags but no trap */
    cmp_result = (p < q);  /* Simple comparison */
    
    /* Use result to avoid dead code elimination */
    return cmp_result ? p : q;
}

/* Test 5: Multiple safe instructions after label with loop */
static int __attribute__((optimize("O3"))) test_multi_ops_after_label(void) {
    volatile int m = 5, n = 3;
    int acc = 0;
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            goto multi_label;
        }
        acc += i;
    }
    
    return acc;
    
multi_label:
    /* Multiple simple instructions that could be split */
    int t1 = m << 2;    /* Shift operation */
    int t2 = n ^ 0x0F;  /* XOR operation */
    int t3 = t1 - t2;   /* Subtraction */
    
    return t3 + acc;
}

/* Test 6: Avoid resource conflicts with jump condition */
static int __attribute__((optimize("O2"))) test_no_resource_conflict(void) {
    /* Use distinct variables to avoid resource conflicts */
    int jump_var = 42;      /* Used only in jump condition */
    int delay_var1 = 100;   /* Used only after label */
    int delay_var2 = 200;   /* Used only after label */
    int result = 0;
    
    /* Simple jump condition using jump_var */
    if (jump_var > 0) {
        goto noconflict_label;
    }
    
    return -1;
    
noconflict_label:
    /* Candidate: uses completely different variables than jump condition */
    result = delay_var1 * delay_var2;  /* No conflict with jump_var */
    
    /* Force use of result */
    return result + jump_var;  /* Only here do we combine them */
}

/* Test 7: Try to trigger try_split with more complex but safe pattern */
static int __attribute__((optimize("O2"))) test_for_try_split(void) {
    unsigned int flags = 0x00FF00FF;
    unsigned int mask = 0x0F0F0F0F;
    unsigned int rotated;
    
    /* Multiple conditions to encourage jump optimization */
    if ((flags & 0xFF) == 0xFF) {
        if ((mask & 0xF0) != 0) {
            goto split_label;
        }
    }
    
    return 0;
    
split_label:
    /* Pattern that might require splitting but is still safe */
    rotated = (flags << 4) | (flags >> 28);  /* Rotation */
    rotated = rotated & mask;                 /* Masking */
    
    return (int)rotated;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Run all test functions */
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_stack_ops_after_label();
    total += test_comparison_after_label();
    total += test_multi_ops_after_label();
    total += test_no_resource_conflict();
    total += test_for_try_split();
    
    /* Print result to ensure code isn't optimized away */
    printf("Total: %d\n", total);
    
    /* Additional volatile operations to prevent aggressive optimization */
    volatile int check = total;
    if (check > 1000) {
        printf("Unexpected large total\n");
    }
    
    return 0;
}
