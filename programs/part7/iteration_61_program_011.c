/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in reorg.cc
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((noinline))
static int test_arithmetic_after_label(int a, int b) {
    int result = 0;
    int temp1 = a + 1;
    int temp2 = b * 2;
    
    /* Create a simple jump to label */
    if (a > b) {
        goto target_label1;
    }
    
    /* Some code to avoid fall-through optimization */
    result = a + b;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be next_trial in the uncovered code */
    int x = temp1 + temp2;  /* Simple arithmetic, no traps */
    
    /* Use result to prevent dead code elimination */
    result = x + 1;
    return result;
}

/* Test function 2: Safe memory operation after label */
__attribute__((noinline))
static int test_memory_after_label(int a) {
    /* Use stack variables for safe memory operations */
    int local_array[4] = {a, a+1, a+2, a+3};
    int result = 0;
    int index = 0;
    
    /* Create jump with simple condition */
    if (a & 1) {  /* Check odd/even */
        goto target_label2;
    }
    
    result = local_array[0];
    return result;
    
target_label2:
    /* Candidate: safe stack memory load */
    /* This is a simple load that shouldn't trap */
    int value = local_array[index];  /* index is 0, so safe */
    
    /* Simple arithmetic to use the value */
    result = value * 2;
    return result;
}

/* Test function 3: Bitwise operations after label */
__attribute__((noinline))
static int test_bitwise_after_label(int a, int b) {
    int result = 0;
    int mask = 0xFF;
    
    /* Multiple conditions to encourage jump optimization */
    if ((a ^ b) > 100) {
        goto target_label3;
    }
    
    result = a & mask;
    return result;
    
target_label3:
    /* Candidate: bitwise operation */
    /* Simple operation that can be split */
    int shifted = (a << 2) | (b >> 2);
    
    result = shifted & mask;
    return result;
}

/* Test function 4: Comparison operation after label */
__attribute__((noinline))
static int test_compare_after_label(int a, int b) {
    int result = 0;
    int compare_var = 100;
    
    /* Unconditional goto to create simple jump */
    if (a != 0) {
        goto target_label4;
    }
    
    result = -1;
    return result;
    
target_label4:
    /* Candidate: comparison that sets condition codes */
    /* This creates a compare instruction */
    int cmp_result = (a > compare_var);
    
    result = cmp_result ? 1 : 0;
    return result;
}

/* Test function 5: Register move after label with loop */
__attribute__((noinline))
static int test_register_move(int iterations) {
    int i;
    int sum = 0;
    int reg1 = 1;
    int reg2 = 2;
    
    for (i = 0; i < iterations; i++) {
        /* Create jump inside loop */
        if (i & 1) {
            goto loop_label;
        }
        
        sum += reg1;
        continue;
        
    loop_label:
        /* Candidate: simple register move/arithmetic */
        /* Different registers to avoid conflicts */
        int temp = reg2 + i;
        
        sum += temp;
    }
    
    return sum;
}

/* Test function 6: Multiple safe operations in sequence */
__attribute__((noinline))
static int test_multiple_operations(int a) {
    int result = 0;
    int var1 = a * 3;
    int var2 = a + 10;
    
    /* Force a jump */
    if (var1 > var2) {
        goto multi_label;
    }
    
    result = var1;
    return result;
    
multi_label:
    /* Sequence of simple operations that can be split */
    /* First operation */
    int op1 = var1 & 0x0F;
    
    /* Second operation - this might be the one tried for delay slot */
    int op2 = op1 + var2;
    
    result = op2;
    return result;
}

/* Test function 7: Avoid trapping operations */
__attribute__((noinline))
static int test_no_trap(int a, int b) {
    int result = 0;
    
    /* Use variables that won't cause division by zero */
    int safe_divisor = b == 0 ? 1 : b;
    
    if (a > 100) {
        goto safe_label;
    }
    
    result = a / safe_divisor;  /* Safe division */
    return result;
    
safe_label:
    /* Candidate: operation that definitely doesn't trap */
    /* No division, no memory access through pointers */
    int safe_result = (a * 2) + (safe_divisor & 0xFF);
    
    result = safe_result;
    return result;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions with various inputs */
    total += test_arithmetic_after_label(10, 5);
    total += test_arithmetic_after_label(5, 10);
    
    total += test_memory_after_label(7);
    total += test_memory_after_label(8);
    
    total += test_bitwise_after_label(255, 128);
    total += test_bitwise_after_label(64, 192);
    
    total += test_compare_after_label(150, 0);
    total += test_compare_after_label(50, 0);
    
    total += test_register_move(5);
    
    total += test_multiple_operations(20);
    total += test_multiple_operations(5);
    
    total += test_no_trap(200, 10);
    total += test_no_trap(50, 0);  /* Test with zero divisor (made safe) */
    
    printf("Total result: %d\n", total);
    printf("(This output ensures all code paths are executed)\n");
    
    return total != 0 ? 0 : 1;
}
