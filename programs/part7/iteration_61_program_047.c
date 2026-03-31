/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -fno-gcse -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x, y, z;
    volatile int result = 0;
    
    x = a + 1;
    y = b * 2;
    
    if (x > y) {
        goto target_label1;
    }
    
    /* Some code to avoid fall-through optimization */
    z = x + y;
    return z;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    z = x - y;  /* next_trial: simple arithmetic operation */
    result = z * 3;
    return result;
}

/* Test 2: Register move/swap pattern */
__attribute__((noinline))
static int test2(int p, int q) {
    int temp1, temp2, temp3;
    volatile int out = 0;
    
    temp1 = p & 0xFF;
    temp2 = q | 0x55;
    
    if (temp1 != temp2) {
        goto target_label2;
    }
    
    temp3 = temp1 << 2;
    return temp3;
    
target_label2:
    /* Candidate: register move operation */
    temp3 = temp2;  /* next_trial: simple move */
    out = temp3 + temp1;
    return out;
}

/* Test 3: Safe stack memory operation */
__attribute__((noinline))
static int test3(int val) {
    int array[4] = {0};
    int idx = 0;
    volatile int sum = 0;
    
    array[0] = val;
    array[1] = val + 1;
    
    if (val > 100) {
        goto target_label3;
    }
    
    idx = 2;
    return array[idx];
    
target_label3:
    /* Candidate: safe stack load */
    idx = array[1];  /* next_trial: stack load (shouldn't trap) */
    sum = idx + array[0];
    return sum;
}

/* Test 4: Bitwise operations */
__attribute__((noinline))
static int test4(unsigned int mask) {
    unsigned int a, b, c;
    volatile int res = 0;
    
    a = mask ^ 0xAAAAAAAA;
    b = mask & 0x55555555;
    
    if ((a & 1) == 0) {
        goto target_label4;
    }
    
    c = a >> 4;
    return c;
    
target_label4:
    /* Candidate: bitwise operation */
    c = b << 1;  /* next_trial: shift operation */
    res = c | 0x1;
    return res;
}

/* Test 5: Comparison operation */
__attribute__((noinline))
static int test5(int x, int y) {
    int diff, abs_diff;
    volatile int ret = 0;
    
    diff = x - y;
    
    if (diff < 0) {
        goto target_label5;
    }
    
    abs_diff = diff;
    return abs_diff;
    
target_label5:
    /* Candidate: comparison/set operation */
    abs_diff = (diff > 0) ? diff : -diff;  /* next_trial: conditional move pattern */
    ret = abs_diff * 2;
    return ret;
}

/* Test 6: Loop with internal goto (increases optimization opportunities) */
__attribute__((noinline))
static int test6(int iterations) {
    int i, acc = 0;
    volatile int total = 0;
    
    for (i = 0; i < iterations; i++) {
        if (i & 1) {
            goto loop_label;
        }
        
        acc += i;
        continue;
        
    loop_label:
        /* Candidate in loop context */
        acc += (i * 2);  /* next_trial: arithmetic in loop */
        total = acc;
    }
    
    return total;
}

/* Test 7: Nested condition with safe operation */
__attribute__((noinline))
static int test7(int a, int b, int c) {
    int tmp1, tmp2, tmp3;
    volatile int out = 0;
    
    tmp1 = a + b;
    tmp2 = b + c;
    
    if (tmp1 > tmp2) {
        if (a > c) {
            goto nested_label;
        }
        tmp3 = tmp1 - tmp2;
        return tmp3;
    }
    
    tmp3 = tmp2 - tmp1;
    return tmp3;
    
nested_label:
    /* Candidate in nested context */
    tmp3 = tmp1 * 2;  /* next_trial: multiplication */
    out = tmp3 + tmp2;
    return out;
}

#pragma GCC pop_options

/* Main function that exercises all patterns */
int main(void) {
    int result = 0;
    volatile int seed = 42;  /* Prevent constant propagation */
    
    /* Call all test functions with different inputs */
    result += test1(seed, seed + 10);
    result += test2(seed, seed * 2);
    result += test3(seed);
    result += test4(seed);
    result += test5(seed, seed - 5);
    result += test6(5);
    result += test7(seed, seed + 1, seed + 2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also test with different inputs in loop */
    for (int i = 0; i < 10; i++) {
        result += test1(i, i * 2);
        result += test4(i * 100);
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
