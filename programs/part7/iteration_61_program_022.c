/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
static int __attribute__((optimize("O2"))) test_arithmetic(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        goto compute;
    }
    
    return -1;
    
compute:
    /* Candidate for next_trial: simple arithmetic */
    c = a + b;  /* Should be safe to move into delay slot */
    result = c * 2;
    
    /* Avoid fall-through to other labels */
    goto finish;
    
finish:
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise(void) {
    unsigned int x = 0xABCD, y = 0x1234;
    unsigned int mask = 0xFF;
    int res = 0;
    
    /* Force a simple jump */
    if (x != 0) {
        goto bit_op;
    }
    
    return -1;
    
bit_op:
    /* Candidate: bitwise operation, no trapping */
    mask = (x & y) | 0x1;  /* Safe operation */
    res = mask ^ 0x55;
    
    /* Use result to prevent elimination */
    if (res > 100) {
        goto done;
    }
    
done:
    return res;
}

/* Test 3: Stack-based memory operation */
static int __attribute__((optimize("O2"))) test_memory_op(void) {
    int local1 = 42;
    int local2 = 100;
    int local3 = 0;
    int temp;
    
    /* Create simple jump */
    if (local1 > 0) {
        goto mem_op;
    }
    
    return -1;
    
mem_op:
    /* Candidate: stack load/store operation */
    temp = local1;      /* Load from stack */
    local3 = temp + 5;  /* Simple arithmetic */
    local2 = local3;    /* Store to stack */
    
    /* Another jump to avoid fall-through issues */
    if (local2 > 50) {
        goto exit_mem;
    }
    
exit_mem:
    return local2;
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_comparison(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Multiple basic blocks to encourage reorg */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto compare_label;
        }
        p++;
    }
    
    return -1;
    
compare_label:
    /* Candidate: comparison operation */
    cmp_result = (p < q);  /* Sets condition codes, no trap */
    
    /* Use different variable to avoid resource conflicts */
    int unused = p * q;
    (void)unused;  /* Prevent warning */
    
    return cmp_result;
}

/* Test 5: Multiple safe operations in sequence */
static int __attribute__((optimize("O3"))) test_sequence(void) {
    volatile int counter = 0;
    int a = 1, b = 2, c = 3, d = 4;
    
    /* Loop with internal goto */
    while (counter < 5) {
        if (counter == 2) {
            goto safe_sequence;
        }
        counter++;
    }
    
    return -1;
    
safe_sequence:
    /* Series of safe operations */
    a = b + 1;      /* Simple arithmetic */
    c = a ^ b;      /* Bitwise, no trap */
    d = c - a;      /* More arithmetic */
    
    /* Ensure all variables are used */
    return a + b + c + d;
}

/* Test 6: Avoid resource conflicts explicitly */
static int __attribute__((optimize("O2"))) test_no_conflict(void) {
    /* Use completely separate variable sets */
    int jump_var = 10;      /* Used only for jump condition */
    int delay_var1 = 20;    /* Used only after label */
    int delay_var2 = 30;    /* Used only after label */
    int output = 0;
    
    /* Simple jump based on jump_var */
    if (jump_var > 5) {
        goto delay_slot_candidate;
    }
    
    return -1;
    
delay_slot_candidate:
    /* These use different variables than the jump condition */
    delay_var1 = delay_var1 * 2;
    delay_var2 = delay_var1 + delay_var2;
    output = delay_var2;
    
    /* Another jump to create more reorg opportunities */
    goto final;
    
final:
    return output;
}

/* Test 7: try_split candidate - operation that can be split */
static int __attribute__((optimize("O2"))) test_splittable(void) {
    int x = 0x12345678;
    int y = 0x87654321;
    int z = 0;
    
    /* Nested conditions to create interesting flow */
    if (x != 0) {
        if (y != 0) {
            goto splittable_op;
        }
    }
    
    return -1;
    
splittable_op:
    /* Operation that might be split by try_split */
    z = (x & 0xFFFF) | (y & 0xFFFF0000);
    
    /* Complex enough for potential splitting but safe */
    return z;
}

/* Test 8: Avoid maybe_never trapping case */
static int __attribute__((optimize("O2"))) test_no_trap(void) {
    int safe_array[4] = {1, 2, 3, 4};
    int index = 2;  /* Always valid index */
    int result = 0;
    
    /* Ensure index is bounds-checked earlier */
    if (index >= 0 && index < 4) {
        goto safe_load;
    }
    
    return -1;
    
safe_load:
    /* Stack access with known-safe index - shouldn't trap */
    result = safe_array[index] + 10;
    
    return result;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Run all test functions */
    total += test_arithmetic();
    total += test_bitwise();
    total += test_memory_op();
    total += test_comparison();
    total += test_sequence();
    total += test_no_conflict();
    total += test_splittable();
    total += test_no_trap();
    
    printf("Total result: %d\n", total);
    printf("(Non-zero result indicates all tests ran)\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
