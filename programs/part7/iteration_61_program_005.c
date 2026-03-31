/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2")))
#else
#define OPTIMIZE_O2
#endif

/* Prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple arithmetic after label - likely eligible for delay slot */
OPTIMIZE_O2
static int test_arithmetic_after_label(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target1;
    }
    
    /* Unreachable code to create separate basic block */
    result += 1000;
    
target1:
    /* Candidate for next_trial: simple arithmetic */
    c = d + 1;  /* Should be safe to move into delay slot */
    
    /* Use result to prevent elimination */
    result += c;
    
    /* Another jump to avoid fall-through issues */
    if (c > 0) {
        goto finish1;
    }
    
    result += 100;
    
finish1:
    return result;
}

/* Test 2: Bitwise operations after label */
OPTIMIZE_O2
static int test_bitwise_after_label(void) {
    int x = 0x1234, y = 0x5678, z = 0;
    int mask = 0xFF;
    
    /* Loop with internal goto to increase optimization chances */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto target2;
        }
        z += i;
    }
    
    z += 100;
    
target2:
    /* Candidate: bitwise operation - safe and splittable */
    y = x & mask;  /* Simple operation, no side effects */
    
    /* Use in computation */
    z += y;
    
    /* Prevent tail merging */
    if (z > 0) {
        goto finish2;
    }
    
    z += 200;
    
finish2:
    return z;
}

/* Test 3: Stack-based memory operation (load/store) */
OPTIMIZE_O2
static int test_stack_ops_after_label(void) {
    int arr[4] = {1, 2, 3, 4};
    int temp = 0;
    int idx = 2;
    
    /* Conditional jump to label */
    switch (idx) {
        case 1:
            temp = 10;
            break;
        case 2:
            goto target3;  /* Simple jump */
        default:
            temp = 20;
    }
    
    temp += 50;
    
target3:
    /* Candidate: stack load operation - should not trap */
    int val = arr[1];  /* Safe stack access */
    
    /* Simple arithmetic with loaded value */
    temp = val * 2;
    
    /* Store back to stack */
    arr[2] = temp;
    
    return temp + arr[0];
}

/* Test 4: Comparison operation after label */
OPTIMIZE_O2
static int test_comparison_after_label(void) {
    int p = 10, q = 20, r = 0;
    int cmp_result = 0;
    
    /* Nested condition to create interesting control flow */
    if (p < q) {
        if (q > 15) {
            goto target4;
        }
        r = 5;
    }
    
    r += 10;
    
target4:
    /* Candidate: comparison operation - sets condition codes */
    cmp_result = (p < q);  /* Simple comparison, no side effects */
    
    /* Use comparison result */
    r += cmp_result ? 100 : 200;
    
    /* Another jump to create basic block boundary */
    if (r > 50) {
        goto finish4;
    }
    
    r += 300;
    
finish4:
    return r;
}

/* Test 5: Register move pattern */
OPTIMIZE_O2
static int test_register_move(void) {
    int v1 = 42, v2 = 0, v3 = 0, v4 = 0;
    
    /* Loop with multiple goto targets */
    for (int j = 0; j < 4; j++) {
        if (j == 2) {
            goto target5;
        }
        v2 += j;
    }
    
    v2 += 1000;
    
target5:
    /* Candidate: simple register move/assignment */
    v3 = v1;  /* Should be eligible for delay slot */
    
    /* Use the moved value */
    v4 = v3 + v2;
    
    /* Complex enough to avoid simplification */
    if (v4 & 1) {
        v4 += 111;
    } else {
        v4 += 222;
    }
    
    return v4;
}

/* Test 6: Multiple safe instructions in sequence */
OPTIMIZE_O2
static int test_multiple_safe_ops(void) {
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4;
    int sum = 0;
    
    /* Unconditional goto */
    goto target6a;
    
    /* Dead code */
    sum += 999;
    
target6a:
    /* First candidate instruction */
    a1 = a2 + a3;
    
    /* Immediate second label */
    goto target6b;
    
    sum += 888;
    
target6b:
    /* Second candidate instruction */
    a4 = a1 << 2;  /* Shift operation */
    
    sum = a1 + a2 + a3 + a4;
    
    /* Conditional return to avoid tail duplication */
    if (sum > 0) {
        return sum;
    }
    return -sum;
}

/* Test 7: Avoid resource conflicts by using fresh variables */
OPTIMIZE_O2
static int test_no_resource_conflict(void) {
    /* Variables for jump condition */
    int cond_a = 5, cond_b = 10;
    
    /* Separate variables for delay slot candidate */
    int slot_x = 20, slot_y = 30, slot_z;
    
    if (cond_a < cond_b) {
        goto target7;
    }
    
    slot_z = 100;
    
target7:
    /* Uses completely different variables than the jump condition */
    slot_z = slot_x * slot_y;  /* No resource conflict with jump */
    
    /* Ensure value is used */
    sink = slot_z;
    
    return slot_z + cond_a + cond_b;
}

/* Test 8: Try split pattern with multiple simple operations */
OPTIMIZE_O2
static int test_try_split_pattern(void) {
    int base = 100;
    int offset = 7;
    int result = 0;
    
    /* Multiple basic blocks */
    {
        int t = base;
        if (t > 50) {
            goto calculate;
        }
        result = -1;
    }
    
    result += 500;
    
calculate:
    /* Pattern that might need splitting: compound operation */
    result = base + (offset * 2);  /* Could be split into load+add+shift */
    
    /* Use in different ways to prevent optimization */
    if (result & 1) {
        return result | 0x100;
    }
    return result & 0xFF;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Run all test functions */
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_stack_ops_after_label();
    total += test_comparison_after_label();
    total += test_register_move();
    total += test_multiple_safe_ops();
    total += test_no_resource_conflict();
    total += test_try_split_pattern();
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    /* Use sink to prevent optimization */
    sink = total;
    
    return total != 0 ? 0 : 1;
}
