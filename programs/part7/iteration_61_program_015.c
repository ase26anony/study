/* Test program for delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
int test_arithmetic_delay(int a, int b) {
    int x = 0, y = 0, z = 0;
    
    /* Initialize variables to avoid uninitialized warnings */
    x = a;
    y = b;
    
    /* Create a simple jump to label */
    if (x > y) {
        goto target_label1;
    }
    
    /* Some intermediate code to create basic block boundaries */
    z = x + y;
    if (z < 0) {
        return -1;
    }
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be safe, non-trapping, and eligible for splitting */
    z = x - y;  /* Simple subtraction - safe operation */
    
    /* Use result to prevent dead code elimination */
    return z + 1;
}

/* Test function 2: Bitwise operations after label */
__attribute__((optimize("O2")))
int test_bitwise_delay(int a, int b) {
    int result = 0;
    int mask = 0xFF;
    
    /* Create control flow with goto */
    if ((a & 0x1) == 0) {
        goto bitwise_target;
    }
    
    /* Some computation */
    result = a * 2;
    if (result > 1000) {
        return result;
    }
    
bitwise_target:
    /* Candidate: bitwise operation - safe and splittable */
    result = a & mask;  /* Bitwise AND - no side effects */
    
    /* Follow with another operation to create basic block */
    result = result | 0x80;
    
    return result;
}

/* Test function 3: Register move pattern */
__attribute__((optimize("O2")))
int test_move_delay(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int output = 0;
    
    /* Multiple jumps to increase optimization opportunities */
    for (int i = 0; i < 3; i++) {
        if (temp1 > temp2) {
            goto move_label;
        }
        temp1++;
    }
    
    /* Fall-through path */
    output = temp1 * 2;
    goto end;
    
move_label:
    /* Candidate: simple register move/assignment */
    output = temp2;  /* Simple move - excellent delay slot candidate */
    
    /* Additional safe operation */
    output = output + 1;
    
end:
    return output;
}

/* Test function 4: Stack-based memory operation */
__attribute__((optimize("O2")))
int test_memory_delay(int a) {
    /* Use stack variables for safe memory operations */
    int local_array[4] = {a, a+1, a+2, a+3};
    int index = 0;
    int value = 0;
    
    /* Control flow with goto */
    if (a % 2 == 0) {
        goto memory_op_label;
    }
    
    /* Alternative path */
    index = 1;
    value = local_array[index];
    if (value > 100) {
        return value;
    }
    
memory_op_label:
    /* Candidate: stack load operation - should be safe */
    value = local_array[2];  /* Loading from stack - unlikely to trap */
    
    /* Simple use of the value */
    return value * 2;
}

/* Test function 5: Comparison operation */
__attribute__((optimize("O2")))
int test_compare_delay(int a, int b) {
    int cmp_result = 0;
    
    /* Create jump opportunity */
    if (a == b) {
        goto compare_label;
    }
    
    /* Different path */
    cmp_result = (a < b) ? -1 : 1;
    if (cmp_result > 0) {
        return 10;
    }
    
compare_label:
    /* Candidate: comparison operation - sets condition codes */
    cmp_result = (a > b);  /* Comparison - safe and splittable */
    
    /* Use the comparison result */
    return cmp_result ? 20 : 30;
}

/* Test function 6: Multiple safe operations in sequence */
__attribute__((optimize("O2")))
int test_multi_ops_delay(int a, int b, int c) {
    int x = a, y = b, z = c;
    int result = 0;
    
    /* Complex enough control flow to trigger reorg */
    for (int i = 0; i < 5; i++) {
        if (x > y) {
            if (z < 10) {
                goto multi_ops_label;
            }
        }
        x--;
        y++;
    }
    
    /* Fall through */
    result = x + y + z;
    return result;
    
multi_ops_label:
    /* Series of safe operations - first one is delay slot candidate */
    result = x & y;      /* First op - potential delay slot fill */
    result = result ^ z; /* Second op */
    result = result << 2; /* Third op */
    
    return result;
}

/* Test function 7: Avoid resource conflicts explicitly */
__attribute__((optimize("O2")))
int test_no_conflict_delay(int a) {
    /* Use completely separate variables for jump and delay slot */
    int jump_var = a;
    int delay_var1 = a * 2;  /* Different computation */
    int delay_var2 = a + 5;
    int result = 0;
    
    /* Jump based on jump_var only */
    if (jump_var > 50) {
        goto no_conflict_label;
    }
    
    /* Alternative path uses different variables */
    result = delay_var1 - delay_var2;
    return result;
    
no_conflict_label:
    /* Delay slot uses only delay_var* variables, not jump_var */
    /* This minimizes resource conflict chances */
    result = delay_var1 + delay_var2;  /* Uses different resources */
    
    return result;
}

/* Main function to execute all tests */
int main() {
    int total = 0;
    
    /* Execute all test functions with various inputs */
    total += test_arithmetic_delay(10, 5);
    total += test_arithmetic_delay(5, 10);
    
    total += test_bitwise_delay(255, 128);
    total += test_bitwise_delay(64, 192);
    
    total += test_move_delay(100, 50);
    total += test_move_delay(25, 75);
    
    total += test_memory_delay(42);
    total += test_memory_delay(99);
    
    total += test_compare_delay(10, 20);
    total += test_compare_delay(30, 15);
    
    total += test_multi_ops_delay(1, 2, 3);
    total += test_multi_ops_delay(10, 20, 30);
    
    total += test_no_conflict_delay(60);
    total += test_no_conflict_delay(40);
    
    printf("Total result: %d\n", total);
    printf("All delay slot tests executed.\n");
    
    return total != 0 ? 0 : 1;
}
