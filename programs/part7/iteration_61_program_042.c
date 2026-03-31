/* test_delay_slot.c
 * Designed to trigger delay slot optimization in GCC's reorg pass
 * Targeting uncovered lines 2135-2149 in reorg.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
static int test1(int a, int b) {
    int x = 0, y = 0, z = 0;
    
    /* Initialize variables to avoid undefined behavior */
    x = a;
    y = b;
    
    /* Create a simple jump to label */
    if (x > y) {
        goto target_label1;
    }
    
    /* Some intermediate code to create basic blocks */
    z = x + y;
    if (z < 0) {
        return z;
    }
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be next_trial in the uncovered code */
    z = x - y;  /* Simple arithmetic, no trap, can be split */
    
    /* Use result to prevent dead code elimination */
    return z + 1;
}

/* Test 2: Bitwise operation after label */
__attribute__((optimize("O2")))
static int test2(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Create control flow with goto */
    if (temp1 != 0) {
        goto compute_label;
    }
    
    /* Different path */
    result = temp1 * 2;
    if (result > 100) {
        return result;
    }
    
compute_label:
    /* Candidate: bitwise operation - safe, no trap */
    result = temp1 & temp2;  /* Bitwise AND, easily splittable */
    
    /* Follow with another operation to create basic block */
    result = result | 0x01;
    
    return result;
}

/* Test 3: Safe memory operation (stack-based) */
__attribute__((optimize("O2")))
static int test3(int a) {
    int local_array[4] = {a, a+1, a+2, a+3};
    int index = 0;
    int value = 0;
    
    /* Control flow with simple jump */
    if (a > 10) {
        goto load_label;
    }
    
    index = 1;
    if (index >= 4) {
        return -1;
    }
    
load_label:
    /* Candidate: stack-based load - unlikely to trap */
    value = local_array[index];  /* Safe stack access */
    
    /* Use value to prevent elimination */
    return value + index;
}

/* Test 4: Comparison operation */
__attribute__((optimize("O2")))
static int test4(int x, int y, int z) {
    int a = x, b = y, c = z;
    int flag = 0;
    
    /* Multiple basic blocks to encourage reorg */
    if (a > b) {
        goto compare_label;
    }
    
    c = a + b;
    if (c < 0) {
        return c;
    }
    
compare_label:
    /* Candidate: comparison operation - sets condition codes */
    flag = (a < b);  /* Comparison, no side effects */
    
    /* Use flag in computation */
    return flag ? c : -c;
}

/* Test 5: Register move pattern */
__attribute__((optimize("O2")))
static int test5(int p1, int p2, int p3) {
    int r1 = p1, r2 = p2, r3 = p3;
    int output = 0;
    
    /* Loop with internal goto to increase optimization chances */
    for (int i = 0; i < 2; i++) {
        if (r1 > r2) {
            goto move_label;
        }
        r1++;
    }
    
    output = r1 + r2;
    if (output > 1000) {
        return output;
    }
    
move_label:
    /* Candidate: simple register operation */
    output = r3;  /* Register move/copy */
    
    /* Simple arithmetic to use the value */
    output = output + 1;
    
    return output;
}

/* Test 6: Shift operation - safe and splittable */
__attribute__((optimize("O2")))
static int test6(int val) {
    int shift_val = val;
    int result = 0;
    
    /* Multiple conditions to create jump opportunities */
    if (shift_val & 1) {
        goto shift_label;
    }
    
    result = shift_val * 3;
    if (result < 0) {
        return result;
    }
    
shift_label:
    /* Candidate: shift operation */
    result = shift_val << 2;  /* Left shift, no trap possible */
    
    return result;
}

/* Test 7: Multiple safe operations in sequence */
__attribute__((optimize("O3")))  /* Higher optimization */
static int test7(int a, int b) {
    int x = a, y = b;
    int tmp1, tmp2, tmp3;
    
    /* Complex enough control flow to trigger reorg */
    if (x == 0) {
        goto sequence_label;
    }
    
    for (int i = 0; i < 3; i++) {
        if (y > i) {
            tmp1 = x * i;
            if (tmp1 > 100) {
                break;
            }
        }
    }
    
    if (x + y < 50) {
        return x + y;
    }
    
sequence_label:
    /* Multiple simple operations - compiler may choose one for delay slot */
    tmp1 = x + 1;    /* Simple arithmetic */
    tmp2 = y - 1;    /* Another simple operation */
    tmp3 = tmp1 ^ tmp2;  /* Bitwise operation */
    
    return tmp3;
}

/* Main function to execute all tests and ensure code runs */
int main(void) {
    int total = 0;
    int iterations = 3;
    
    /* Run multiple iterations to increase chance of optimization */
    for (int i = 0; i < iterations; i++) {
        total += test1(i, i+1);
        total += test2(i*2, i*3);
        total += test3(i*5);
        total += test4(i, i*2, i*3);
        total += test5(i, i+10, i+20);
        total += test6(i+5);
        total += test7(i*3, i*4);
    }
    
    printf("Total result: %d\n", total);
    printf("(This output ensures all code paths are executed)\n");
    
    /* Use result to prevent dead code elimination of entire program */
    if (total > 1000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
