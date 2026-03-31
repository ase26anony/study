/* Test program for delay slot filling optimization targeting reorg.cc lines 2135-2149 */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(void) {
    volatile int a = 10, b = 20, c = 30;
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
    
    /* Prevent tail call optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* Test function 2: Register move/bitwise operation */
__attribute__((noinline))
static int test2(int x) {
    int y = 0, z = 0;
    volatile int trigger = 1;
    
    if (trigger != 0) {
        goto compute_label;
    }
    
    return x;
    
compute_label:
    /* Candidate: bitwise operation, safe and splittable */
    y = x & 0xFF;  /* next_trial: AND instruction */
    z = y | 0x100;
    
    /* Use result to prevent elimination */
    return z + x;
}

/* Test function 3: Stack-based memory operation */
__attribute__((noinline))
static int test3(void) {
    int arr[4] = {1, 2, 3, 4};
    int i = 0;
    volatile int flag = 1;
    
    if (flag) {
        goto process_label;
    }
    
    return arr[0];
    
process_label:
    /* Candidate: stack load operation (less likely to trap) */
    i = arr[2];  /* next_trial: load from stack */
    
    /* Simple arithmetic to use the result */
    return i * 3;
}

/* Test function 4: Comparison operation */
__attribute__((noinline))
static int test4(int p, int q) {
    int cmp_result = 0;
    volatile int choice = p > q;
    
    if (choice) {
        goto compare_label;
    }
    
    return p + q;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (p < q);  /* next_trial: compare instruction */
    
    /* Use the comparison result */
    return cmp_result ? p : q;
}

/* Test function 5: Multiple basic blocks with safe operations */
__attribute__((noinline))
static int test5(int n) {
    int acc = 0;
    int i = 0;
    volatile int limit = n;
    
    /* Loop to create more optimization opportunities */
    for (i = 0; i < limit; i++) {
        if (i & 1) {
            goto odd_case;
        }
        
        acc += i * 2;
        continue;
        
    odd_case:
        /* Candidate: simple increment operation */
        acc += 1;  /* next_trial: add immediate */
        
        /* Another safe operation to prevent fall-through issues */
        acc += i & 0xF;
    }
    
    return acc;
}

/* Test function 6: Avoid resource conflicts with distinct variables */
__attribute__((noinline))
static int test6(void) {
    /* Use distinct variables to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    volatile int cond = (r1 < r2);
    
    if (cond) {
        goto safe_operation;
    }
    
    return r1;
    
safe_operation:
    /* Candidate: operation on variables not used in jump condition */
    r4 = r3 * 2;  /* r3 and r4 not used in condition */
    
    /* Use result in computation */
    return r4 + r1 + r2;
}

/* Test function 7: Nested control flow */
__attribute__((noinline))
static int test7(int val) {
    int temp = val;
    volatile int mode = val % 3;
    
    switch (mode) {
        case 0:
            goto mode0;
        case 1:
            goto mode1;
        default:
            goto mode_default;
    }
    
mode0:
    /* Candidate: shift operation */
    temp = val << 2;  /* next_trial: shift left */
    break;
    
mode1:
    /* Candidate: subtraction */
    temp = val - 5;  /* next_trial: subtract */
    break;
    
mode_default:
    /* Candidate: addition */
    temp = val + 10;  /* next_trial: add */
    break;
    }
    
    return temp;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test1();
    total += test2(42);
    total += test3();
    total += test4(10, 20);
    total += test5(5);
    total += test6();
    total += test7(15);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    /* Also use volatile to ensure all code paths are considered */
    volatile int check = total;
    if (check > 1000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
