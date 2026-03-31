/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    if (a < b) {
        goto target_label1;
    }
    
    /* This should be dead code */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c * 2;
    return result;
}

/* Test function 2: Bitwise operations after label */
__attribute__((noinline))
static int test2(int x) {
    int y = 0, z = 0;
    
    if (x > 0) {
        goto compute_label;
    }
    
    return -1;
    
compute_label:
    /* Candidate: bitwise operations */
    y = x & 0xFF;      /* next_trial: AND instruction */
    z = y | 0x80;
    return z;
}

/* Test function 3: Stack-based memory operation */
__attribute__((noinline))
static int test3(void) {
    int arr[4] = {1, 2, 3, 4};
    int sum = 0;
    volatile int flag = 1;
    
    if (flag) {
        goto process_label;
    }
    
    return 0;
    
process_label:
    /* Candidate: load from stack memory (less likely to trap) */
    sum = arr[2];      /* next_trial: load instruction */
    return sum * 10;
}

/* Test function 4: Multiple operations with distinct registers */
__attribute__((noinline))
static int test4(int p1, int p2) {
    int r1 = 0, r2 = 0, r3 = 0;
    
    /* Complex condition to encourage jump generation */
    if ((p1 ^ p2) & 0xF) {
        goto operation_label;
    }
    
    return p1 + p2;
    
operation_label:
    /* Multiple simple operations - compiler may choose one */
    r1 = p1 << 2;      /* Candidate: shift instruction */
    r2 = r1 + p2;
    r3 = r2 ^ 0xABCD;
    return r3;
}

/* Test function 5: Comparison operation */
__attribute__((noinline))
static int test5(int val) {
    int cmp_result = 0;
    volatile int threshold = 100;
    
    if (val != 0) {
        goto compare_label;
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (val > threshold);  /* next_trial: compare/set */
    return cmp_result ? val : -val;
}

/* Test function 6: Loop with internal goto to increase optimization chances */
__attribute__((noinline))
static int test6(int iterations) {
    int i, acc = 0;
    volatile int mod_check = 3;
    
    for (i = 0; i < iterations; i++) {
        if ((i % mod_check) == 0) {
            goto update_label;
        }
        acc += i;
        continue;
        
    update_label:
        /* Candidate in loop context */
        acc += (i * 2);  /* next_trial: multiply/add */
    }
    
    return acc;
}

/* Test function 7: Nested conditions with safe operation */
__attribute__((noinline))
static int test7(int a, int b, int c) {
    int temp = 0;
    
    if (a > b) {
        if (b < c) {
            goto safe_op_label;
        }
        return a;
    }
    
    return b;
    
safe_op_label:
    /* Very safe operation: register move/arithmetic */
    temp = c - a;      /* next_trial: subtract instruction */
    return temp * b;
}

/* Test function 8: Using different variable types to hit different patterns */
__attribute__((noinline))
static int test8(unsigned int mask) {
    unsigned int result = 0;
    volatile int use_mask = 1;
    
    if (use_mask && (mask != 0)) {
        goto mask_label;
    }
    
    return 0xFFFFFFFF;
    
mask_label:
    /* Bit manipulation operations */
    result = mask >> 4;    /* next_trial: shift instruction */
    result = result & 0x0F0F0F0F;
    return (int)result;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    
    /* Execute all test functions with various inputs */
    total += test1();                          /* Expected: 60 */
    total += test2(0x1234);                    /* Expected: 0xB4 */
    total += test3();                          /* Expected: 30 */
    total += test4(5, 7);                      /* Depends on computation */
    total += test5(150);                       /* Expected: 150 */
    total += test6(10);                        /* Sum calculation */
    total += test7(10, 5, 8);                  /* Expected: -10 */
    total += test8(0xDEADBEEF);                /* Bitmask result */
    
    printf("Total result: %d\n", total);
    printf("All test patterns executed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
