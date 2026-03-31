/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
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
    c = a + b;  /* next_trial: add operation */
    result = c;
    
    /* Prevent fall-through to another label */
    goto end1;
    
end1:
    return result;
}

/* Test 2: Bitwise operations after label */
__attribute__((optimize("O3")))
static int test2(void) {
    int x = 5, y = 3, z = 0;
    volatile int trigger = 1;
    
    if (trigger != 0) {
        goto compute;
    }
    
    z = 999;
    return z;
    
compute:
    /* Candidate: bitwise operation */
    z = x & y;  /* next_trial: AND operation */
    
    /* Use result to prevent elimination */
    if (z == 1) {
        return z + 10;
    }
    return z;
}

/* Test 3: Safe stack memory operation */
__attribute__((optimize("O2")))
static int test3(void) {
    int arr[4] = {1, 2, 3, 4};
    int temp = 0;
    volatile int flag = 1;
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < 2; i++) {
        if (flag > 0) {
            goto process;
        }
        
        temp = arr[0];
        break;
        
    process:
        /* Candidate: stack load operation */
        temp = arr[2];  /* next_trial: memory load (safe, stack-based) */
        
        /* Simple arithmetic to use the result */
        temp = temp * 2;
        
        if (i == 0) {
            goto continue_loop;
        }
    }
    
continue_loop:
    return temp;
}

/* Test 4: Comparison operation after label */
__attribute__((optimize("O2")))
static int test4(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    volatile int counter = 1;
    
    /* Multiple basic blocks to encourage reorg */
    if (counter > 0) {
        goto compare;
    }
    
    if (p > q) {
        return 1;
    }
    
compare:
    /* Candidate: comparison operation */
    cmp_result = (p < q);  /* next_trial: compare, sets condition codes */
    
    /* Use different variable to avoid resource conflicts */
    int output = cmp_result ? p : q;
    
    /* Another jump to create control flow */
    if (output > 150) {
        goto finish;
    }
    
finish:
    return output;
}

/* Test 5: Register move pattern */
__attribute__((optimize("O3")))
static int test5(void) {
    register int r1 asm("r0") = 42;
    register int r2 asm("r1") = 17;
    int r3 = 0;
    
    /* Create predictable branch */
    if (r1 != 0) {
        goto move_op;
    }
    
    r3 = -1;
    return r3;
    
move_op:
    /* Candidate: register-to-register move */
    r3 = r1;  /* next_trial: move operation */
    
    /* Simple use */
    r3 = r3 + 1;
    
    return r3;
}

/* Test 6: Multiple safe operations in sequence */
__attribute__((optimize("O2")))
static int test6(void) {
    int a = 7, b = 3, c = 0, d = 0;
    volatile int select = 2;
    
    switch (select) {
        case 1:
            c = a - b;
            break;
        case 2:
            goto case2_label;
        default:
            return -1;
    }
    
    return c;
    
case2_label:
    /* First candidate operation */
    c = a | b;  /* next_trial: bitwise OR */
    
    /* Immediate second operation - still could be moved */
    d = c << 2;
    
    return d;
}

/* Test 7: Avoid trapping operations - safe shift */
__attribute__((optimize("O2")))
static int test7(void) {
    unsigned int u = 0x1234;
    unsigned int v = 0;
    volatile int mode = 1;
    
    /* Nested condition to create jump */
    if (mode) {
        if (u != 0) {
            goto shift_op;
        }
    }
    
    v = 0xFFFF;
    return v;
    
shift_op:
    /* Candidate: safe shift (won't trap) */
    v = u >> 4;  /* next_trial: shift operation */
    
    return v;
}

/* Test 8: Complex control flow with multiple labels */
__attribute__((optimize("O3")))
static int test8(void) {
    int val1 = 10, val2 = 20, val3 = 30;
    int out = 0;
    
    /* Multiple jumps to different labels */
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            goto block_a;
        } else if (i == 1) {
            goto block_b;
        } else {
            goto block_c;
        }
        
    block_a:
        val1 = val1 * 2;
        continue;
        
    block_b:
        /* Candidate in middle of loop */
        val2 = val1 + val3;  /* next_trial: add operation */
        continue;
        
    block_c:
        val3 = val2 - val1;
        out += val3;
    }
    
    return out;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Run all tests and accumulate results */
    total += test1();
    total += test2();
    total += test3();
    total += test4();
    total += test5();
    total += test6();
    total += test7();
    total += test8();
    
    printf("Total result: %d\n", total);
    printf("(This value should be deterministic and non-zero)\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
