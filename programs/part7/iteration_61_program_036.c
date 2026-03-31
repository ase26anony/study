/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2", "no-gcse", "no-crossjumping")))
#else
#define OPTIMIZE_O2
#endif

/* Global to prevent optimization */
volatile int global_counter = 0;

/* Test 1: Simple arithmetic after label */
OPTIMIZE_O2
static int test1(void) {
    int a = 10, b = 20, c = 30, d = 40;
    int result = 0;
    
    /* Create simple jump condition */
    if (a < b) {
        /* This goto creates a simple jump to label */
        goto target_label1;
    }
    
    /* Dead code to avoid fall-through optimization */
    result = -1;
    return result;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic */
    /* This should be safe, non-trapping, and splittable */
    c = d + 5;  /* Simple add operation */
    
    /* Use result to prevent elimination */
    result = c + a;
    return result;
}

/* Test 2: Bitwise operations after label */
OPTIMIZE_O2  
static int test2(void) {
    unsigned int x = 0x1234, y = 0x5678, z = 0;
    unsigned int mask = 0xFF;
    
    /* Force a jump */
    if (x != 0) {
        goto bitwise_label;
    }
    
    return 0;
    
bitwise_label:
    /* Candidate: bitwise operation - safe and simple */
    z = y & mask;  /* Bitwise AND */
    
    /* Use result */
    return (int)(z | 0x100);
}

/* Test 3: Stack-based memory operation */
OPTIMIZE_O2
static int test3(void) {
    int arr[4] = {1, 2, 3, 4};
    int temp = 0;
    int i = 0;
    
    /* Loop with internal goto to create jump */
    for (i = 0; i < 2; i++) {
        if (arr[i] > 0) {
            goto mem_op_label;
        }
    }
    
    return -1;
    
mem_op_label:
    /* Candidate: stack memory load - should be safe */
    temp = arr[2];  /* Loading from stack array */
    
    /* Simple arithmetic with loaded value */
    return temp * 2;
}

/* Test 4: Comparison operation */
OPTIMIZE_O2
static int test4(void) {
    int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Unconditional goto to create simple jump */
    if (p != 0) {
        goto compare_label;
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison operation - sets condition codes */
    cmp_result = (p < q);  /* Comparison, no trap */
    
    /* Use result */
    return cmp_result ? 100 : 200;
}

/* Test 5: Multiple operations with register moves */
OPTIMIZE_O2
static int test5(void) {
    register int r1 asm("r1") = 1;
    register int r2 asm("r2") = 2;
    register int r3 asm("r3") = 3;
    int output = 0;
    
    /* Create jump with register condition */
    if (r1 > 0) {
        goto reg_op_label;
    }
    
    return 0;
    
reg_op_label:
    /* Candidate: register-to-register operation */
    r3 = r1 + r2;  /* Register arithmetic */
    
    /* Use result */
    output = r3;
    return output;
}

/* Test 6: Safe shift operation */
OPTIMIZE_O2
static int test6(void) {
    unsigned int val = 0x80000000;
    int shift = 4;
    unsigned int shifted = 0;
    
    /* Conditional jump */
    if (shift > 0 && shift < 32) {
        goto shift_label;
    }
    
    return 0;
    
shift_label:
    /* Candidate: shift operation - safe if shift amount is valid */
    shifted = val >> shift;  /* Logical right shift */
    
    return (int)shifted;
}

/* Test 7: Complex pattern with nested jumps */
OPTIMIZE_O2
static int test7(void) {
    int counter = 0;
    int a = 5, b = 10, c = 15;
    
    /* Multiple basic blocks to encourage reorg */
    if (a < b) {
        if (b < c) {
            goto nested_label;
        }
    }
    
    return 0;
    
nested_label:
    /* Candidate: multiple simple operations */
    a = b + 1;      /* First operation */
    c = a * 2;      /* Second operation - depends on first */
    
    return a + c;
}

/* Test 8: Avoid resource conflicts by using fresh variables */
OPTIMIZE_O2
static int test8(void) {
    /* Variables for jump condition */
    int cond_x = 1, cond_y = 2;
    
    /* Fresh variables for the delay slot candidate */
    int fresh_a = 10, fresh_b = 20, fresh_c;
    
    if (cond_x < cond_y) {
        goto fresh_var_label;
    }
    
    return 0;
    
fresh_var_label:
    /* Candidate: uses variables not involved in jump condition */
    fresh_c = fresh_a + fresh_b;  /* No resource conflict */
    
    return fresh_c;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test1();
    total += test2();
    total += test3();
    total += test4();
    total += test5();
    total += test6();
    total += test7();
    total += test8();
    
    /* Also use global to prevent optimization */
    global_counter += total;
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
