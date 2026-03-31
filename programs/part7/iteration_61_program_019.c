/* Test program for triggering delay slot filling conditions in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2")))
#else
#define OPTIMIZE_O2
#endif

/* Test 1: Simple arithmetic after label */
OPTIMIZE_O2
static int test1(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        goto target_label1;
    }
    
    /* Unreachable code to create jump opportunity */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    c = a + b;  /* next_trial: add operation */
    result = c;
    
    /* Prevent fall-through issues */
    if (result > 0) {
        return result;
    }
    return 1;
}

/* Test 2: Register move/assignment pattern */
OPTIMIZE_O2
static int test2(void) {
    int x = 5, y = 7, z = 0;
    volatile int trigger = 1;
    
    /* Create simple jump to label */
    if (trigger != 0) {
        goto compute;
    }
    
    z = x * y;
    return z;
    
compute:
    /* Candidate: move/assignment operation */
    z = x;  /* next_trial: register move */
    
    /* Use result to prevent elimination */
    return z + y;
}

/* Test 3: Bitwise operation after label */
OPTIMIZE_O2
static int test3(void) {
    unsigned int flags = 0x0F;
    unsigned int mask = 0x03;
    unsigned int result = 0;
    
    /* Loop with internal goto to increase optimization chances */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto apply_mask;
        }
        result += i;
    }
    
    return result;
    
apply_mask:
    /* Candidate: bitwise operation (unlikely to trap) */
    result = flags & mask;  /* next_trial: bitwise AND */
    
    return result + 1;
}

/* Test 4: Stack-based memory operation (load) */
OPTIMIZE_O2
static int test4(void) {
    int array[4] = {1, 2, 3, 4};
    int index = 2;
    int temp = 0;
    
    /* Conditional jump to label */
    if (array[0] < array[3]) {
        goto load_value;
    }
    
    temp = array[1];
    return temp;
    
load_value:
    /* Candidate: safe stack load operation */
    temp = array[index];  /* next_trial: memory load (stack-based, safe) */
    
    return temp * 2;
}

/* Test 5: Comparison operation */
OPTIMIZE_O2
static int test5(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Simple unconditional jump pattern */
    if (p != q) {
        goto compare;
    }
    
    return 0;
    
compare:
    /* Candidate: comparison operation */
    cmp_result = (p < q);  /* next_trial: comparison sets condition codes */
    
    return cmp_result ? 10 : 20;
}

/* Test 6: Multiple operations with split opportunities */
OPTIMIZE_O2
static int test6(void) {
    volatile int counter = 0;
    int a = 1, b = 2, c = 3, d = 4;
    int r1 = 0, r2 = 0;
    
    /* Nested control flow */
    while (counter < 2) {
        if (counter == 1) {
            goto process;
        }
        counter++;
    }
    
    return a + b;
    
process:
    /* Multiple simple operations - try_split may work on these */
    r1 = a + b;    /* First operation */
    r2 = c - d;    /* Second operation */
    
    /* Use both results */
    return r1 * r2;
}

/* Test 7: Avoid resource conflicts with distinct variables */
OPTIMIZE_O2
static int test7(void) {
    /* Use distinct variable sets to avoid resource conflicts */
    int jump_var = 42;      /* Used only for jump condition */
    int delay_var1 = 10;    /* Used only in delay candidate */
    int delay_var2 = 20;    /* Used only in delay candidate */
    int result = 0;
    
    /* Jump condition uses only jump_var */
    if (jump_var > 0) {
        goto safe_operation;
    }
    
    result = jump_var;
    return result;
    
safe_operation:
    /* Delay candidate uses different variables */
    result = delay_var1 + delay_var2;  /* No conflict with jump condition */
    
    return result + jump_var;  /* Combine only after operation */
}

/* Test 8: Increment operation */
OPTIMIZE_O2
static int test8(void) {
    int count = 0;
    volatile int limit = 5;
    
    /* Loop with goto exit */
    for (int i = 0; i < limit; i++) {
        if (i == limit - 1) {
            goto increment;
        }
        count += i;
    }
    
    return count;
    
increment:
    /* Simple increment operation */
    count++;  /* next_trial: increment */
    
    return count;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    printf("Running delay slot filling tests...\n");
    
    /* Execute all test functions */
    total += test1();
    total += test2();
    total += test3();
    total += test4();
    total += test5();
    total += test6();
    total += test7();
    total += test8();
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    }
    return 1;
}
