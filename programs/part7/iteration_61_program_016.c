/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((noinline))
static int test_arithmetic_delay(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create control flow with goto to label */
    if (a < b) {
        goto compute;
    }
    
    /* Dead code path to create jump opportunity */
    result = -1;
    return result;
    
compute:
    /* This instruction should be candidate for delay slot */
    /* Simple arithmetic, no side effects */
    c = a + b;  /* next_trial candidate */
    
    /* Use result to prevent elimination */
    result = c * 2;
    return result;
}

/* Test function 2: Bitwise operations after label */
__attribute__((noinline))
static int test_bitwise_delay(void) {
    unsigned int x = 0x1234, y = 0x5678;
    unsigned int mask = 0xFF;
    int res = 0;
    
    /* Force a jump */
    if (x != 0) {
        goto bitwise_op;
    }
    
    return -1;
    
bitwise_op:
    /* Safe bitwise operation - good delay slot candidate */
    mask = x & y;  /* next_trial candidate */
    
    res = mask | 0x1000;
    return res;
}

/* Test function 3: Stack-based memory operation */
__attribute__((noinline))
static int test_memory_delay(void) {
    int local1 = 100;
    int local2 = 200;
    int local3 = 300;
    int sum = 0;
    
    /* Create jump with simple condition */
    if (local1 > 50) {
        goto memory_op;
    }
    
    return -1;
    
memory_op:
    /* Stack load/store - unlikely to trap */
    local3 = local1 + local2;  /* next_trial candidate */
    
    sum = local1 + local2 + local3;
    return sum;
}

/* Test function 4: Comparison operation */
__attribute__((noinline))
static int test_compare_delay(void) {
    int p = 42, q = 84;
    int cmp_result;
    
    /* Unconditional goto to create simple jump */
    if (p > 0) {
        goto compare_label;
    }
    
    return -1;
    
compare_label:
    /* Comparison sets condition codes without side effects */
    cmp_result = (p < q);  /* next_trial candidate */
    
    return cmp_result ? 100 : 200;
}

/* Test function 5: Multiple operations in loop with goto */
__attribute__((noinline))
static int test_loop_delay(void) {
    int i, total = 0;
    int temp1 = 5, temp2 = 10;
    
    for (i = 0; i < 10; i++) {
        /* Conditional jump inside loop */
        if (i % 2 == 0) {
            goto loop_label;
        }
        
        total += i;
        continue;
        
    loop_label:
        /* Simple operation after label */
        temp1 = temp2 + i;  /* next_trial candidate */
        
        total += temp1;
    }
    
    return total;
}

/* Test function 6: Register move-like operation */
__attribute__((noinline))
static int test_move_delay(void) {
    int src = 999;
    int dst;
    int counter = 0;
    
    /* Multiple jumps to same label */
    if (src > 500) {
        goto move_op;
    }
    
    if (src < 1000) {
        goto move_op;
    }
    
    return -1;
    
move_op:
    /* Simple move-like operation */
    dst = src;  /* next_trial candidate */
    
    counter = dst / 2;  /* Division by constant is safe */
    return counter;
}

/* Test function 7: Shift operation */
__attribute__((noinline))
static int test_shift_delay(void) {
    unsigned int val = 0xABCD;
    unsigned int shifted;
    int ret = 0;
    
    /* Create basic block with goto */
    if (val != 0) {
        goto shift_label;
    }
    
    return 0;
    
shift_label:
    /* Shift operation - no trapping */
    shifted = val << 2;  /* next_trial candidate */
    
    ret = (shifted & 0xFF);
    return ret;
}

/* Test function 8: Complex control flow with multiple labels */
__attribute__((noinline))
static int test_complex_flow(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* First jump */
    if (a < b) {
        goto label1;
    }
    
    return -1;
    
label1:
    /* First candidate instruction */
    c = a + b;  /* next_trial candidate for first jump */
    
    /* Another jump */
    if (c > 0) {
        goto label2;
    }
    
    return -2;
    
label2:
    /* Second candidate instruction */
    d = c * 2;  /* next_trial candidate for second jump */
    
    result = a + b + c + d;
    return result;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test_arithmetic_delay();
    total += test_bitwise_delay();
    total += test_memory_delay();
    total += test_compare_delay();
    total += test_loop_delay();
    total += test_move_delay();
    total += test_shift_delay();
    total += test_complex_flow();
    
    printf("Total result: %d\n", total);
    printf("All delay slot tests executed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure */
    }
}
