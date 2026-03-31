/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in GCC's reorg.cc
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch test_delay_slot.c -o test
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 test_delay_slot.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - most likely candidate */
__attribute__((optimize("O2")))
static int test_arithmetic_after_label(int a, int b) {
    int x = 0, y = 0, z = 0;
    
    /* Initialize variables to avoid undefined behavior */
    x = a;
    y = b;
    z = x + y;
    
    if (z > 100) {
        /* Simple jump to label */
        goto target_label1;
    }
    
    /* Some other computation to make the function non-trivial */
    x = y * 2;
    return x;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic that doesn't trap */
    /* This should be movable into delay slot */
    z = x - y;  /* Simple arithmetic, no side effects */
    
    /* Use result to prevent dead code elimination */
    return z + 10;
}

/* Test 2: Bitwise operations after label */
__attribute__((optimize("O3")))
static int test_bitwise_after_label(int a, int b) {
    int mask = 0xFF;
    int result = 0;
    
    /* Create condition for jump */
    if ((a & 0x1) == 0) {
        goto bitwise_label;
    }
    
    result = a | b;
    return result;
    
bitwise_label:
    /* Candidate: bitwise operation - safe and splittable */
    result = a & mask;  /* Simple bitwise AND */
    
    /* Follow with another operation to create basic block */
    result = result ^ 0x55;
    return result;
}

/* Test 3: Safe stack-based memory operation */
__attribute__((optimize("O2")))
static int test_memory_after_label(int a) {
    int local_array[4] = {a, a+1, a+2, a+3};
    int temp = 0;
    int i = 0;
    
    /* Loop to encourage optimization */
    for (i = 0; i < 3; i++) {
        if (local_array[i] > 50) {
            goto memory_label;
        }
        temp += local_array[i];
    }
    
    return temp;
    
memory_label:
    /* Candidate: stack memory load - should not trap */
    temp = local_array[2];  /* Loading from stack, safe */
    
    /* Simple arithmetic to use the value */
    return temp * 2;
}

/* Test 4: Comparison operation after label */
__attribute__((optimize("O2")))
static int test_compare_after_label(int a, int b) {
    int cmp_result = 0;
    
    if (a == b) {
        goto compare_label;
    }
    
    /* Different computation path */
    cmp_result = (a > b) ? 1 : -1;
    return cmp_result;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (a < b);  /* Comparison operation */
    
    /* Use the comparison result */
    return cmp_result ? 100 : 200;
}

/* Test 5: Multiple jumps to same label with different conditions */
__attribute__((optimize("O3")))
static int test_multiple_jumps(int a, int b, int c) {
    int result = 0;
    
    /* First potential jump */
    if (a > 10) {
        goto common_label;
    }
    
    /* Second potential jump */
    if (b < 5) {
        goto common_label;
    }
    
    /* Third potential jump */
    if (c == 0) {
        goto common_label;
    }
    
    result = a + b + c;
    return result;
    
common_label:
    /* Candidate: simple move/arithmetic operation */
    result = b * 2;  /* Multiplication by 2 is often shift operation */
    
    /* Additional operation to prevent fall-through issues */
    result += 1;
    return result;
}

/* Test 6: Nested control flow with safe operation */
__attribute__((optimize("O2")))
static int test_nested_flow(int x) {
    int a = x, b = x + 1, c = x + 2;
    
    /* Outer condition */
    if (x > 0) {
        /* Inner condition */
        if (x < 100) {
            goto nested_label;
        }
        c = a - b;
    }
    
    return c;
    
nested_label:
    /* Candidate: safe arithmetic with distinct variables */
    /* Using variables not involved in jump condition */
    int d = b + c;  /* b and c aren't used in the jump condition directly */
    
    /* Ensure value is used */
    return d * 3;
}

/* Test 7: Register move pattern */
__attribute__((optimize("O2")))
static int test_register_move(int val) {
    int reg1 = val;
    int reg2 = 0;
    int reg3 = 0;
    
    /* Create simple jump condition */
    reg2 = reg1 * 2;
    if (reg2 > 50) {
        goto move_label;
    }
    
    reg3 = reg1 + reg2;
    return reg3;
    
move_label:
    /* Candidate: simple register-to-register move */
    reg3 = reg1;  /* Simple move operation */
    
    /* Follow with arithmetic to prevent elimination */
    return reg3 + 100;
}

/* Test 8: Shift operation after label */
__attribute__((optimize("O3")))
static int test_shift_after_label(int value) {
    int shifted = 0;
    
    if (value & 0x80000000) {
        goto shift_label;
    }
    
    shifted = value >> 1;
    return shifted;
    
shift_label:
    /* Candidate: shift operation - safe and simple */
    shifted = value << 2;  /* Shift left by 2 */
    
    /* Mask to prevent undefined behavior */
    shifted = shifted & 0x7FFFFFFF;
    return shifted;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    int i;
    
    /* Seed for pseudo-random but deterministic behavior */
    srand(42);
    
    /* Execute each test multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        
        total += test_arithmetic_after_label(a, b);
        total += test_bitwise_after_label(a, b);
        total += test_memory_after_label(a);
        total += test_compare_after_label(a, b);
        total += test_multiple_jumps(a, b, c);
        total += test_nested_flow(a);
        total += test_register_move(a);
        total += test_shift_after_label(a);
    }
    
    printf("Total result: %d\n", total);
    printf("(This ensures all code paths are executed and not optimized away)\n");
    
    return 0;
}
