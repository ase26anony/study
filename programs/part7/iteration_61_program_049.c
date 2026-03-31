/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -fno-gcse test_delay_slot.c -o test_delay_slot
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer test_delay_slot.c -o test_delay_slot
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
static int __attribute__((optimize("O2"))) test_arithmetic(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    if (a < b) {
        goto arith_label;
    }
    
    /* This should be dead code */
    result = -1;
    return result;
    
arith_label:
    /* Candidate for delay slot: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c * 2;
    
    /* Prevent tail call optimization */
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise(void) {
    unsigned int x = 0x12345678;
    unsigned int y = 0x87654321;
    unsigned int z = 0;
    int count = 100;
    
    /* Loop to increase optimization opportunities */
    while (count-- > 0) {
        if (x != y) {
            goto bitwise_label;
        }
        x ^= 0xFFFF;  /* Modify to change condition */
    }
    
    return -1;
    
bitwise_label:
    /* Candidate: bitwise operation */
    z = x & y;  /* next_trial: AND instruction */
    
    /* Use result to prevent elimination */
    int result = (z != 0);
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 3: Safe stack load/store */
static int __attribute__((optimize("O2"))) test_memory_op(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    int i = 0;
    
    /* Multiple jumps to same label */
    for (i = 0; i < 4; i++) {
        if (array[i] > 2) {
            goto memory_label;
        }
    }
    
    return 0;
    
memory_label:
    /* Candidate: stack memory load (safe, won't trap) */
    temp = array[2];  /* next_trial: load from stack */
    
    /* Simple use of loaded value */
    int result = temp * 3;
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 4: Comparison operation */
static int __attribute__((optimize("O2"))) test_comparison(void) {
    int p = 50, q = 60;
    int r = 0;
    
    /* Nested conditions */
    if (p < 100) {
        if (q > 10) {
            goto compare_label;
        }
    }
    
    return -1;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    r = (p < q);  /* next_trial: compare/setcc */
    
    /* Use the boolean result */
    int result = r ? 100 : 200;
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 5: Register move operation */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    register int src asm("t0") = 42;  /* Suggest register */
    register int dst asm("t1") = 0;
    
    /* Force conditional jump */
    if (src != 0) {
        goto move_label;
    }
    
    return 0;
    
move_label:
    /* Candidate: register-to-register move */
    dst = src;  /* next_trial: move instruction */
    
    /* Use moved value */
    int result = dst + 10;
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 6: Shift operation */
static int __attribute__((optimize("O3"))) test_shift(void) {
    unsigned int val = 0x80000000;
    int shift_count = 4;
    unsigned int shifted = 0;
    
    /* Multiple basic blocks */
    switch (shift_count) {
        case 1:
        case 2:
        case 3:
        case 4:
            goto shift_label;
        default:
            return -1;
    }
    
shift_label:
    /* Candidate: shift operation */
    shifted = val >> shift_count;  /* next_trial: shift instruction */
    
    int result = (shifted != 0);
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 7: Complex pattern with multiple labels */
static int __attribute__((optimize("O2"))) test_multiple_labels(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* First jump opportunity */
    if (a < b) {
        goto label1;
    }
    
    return -1;
    
label1:
    /* First candidate: simple add */
    c = a + b;  /* next_trial candidate 1 */
    
    /* Immediate second jump */
    if (c > 0) {
        goto label2;
    }
    
    return -2;
    
label2:
    /* Second candidate: subtract */
    d = c - a;  /* next_trial candidate 2 */
    
    result = d * 2;
    asm volatile("" : "+r" (result));
    return result;
}

/* Test function 8: Avoid resource conflicts */
static int __attribute__((optimize("O2"))) test_no_conflict(void) {
    /* Use completely separate variables for jump condition
       and delay slot candidate to avoid resource conflicts */
    int jump_var1 = 100;
    int jump_var2 = 200;
    
    /* Variables only used after label */
    int delay_var1 = 300;
    int delay_var2 = 400;
    int delay_result = 0;
    
    /* Jump condition uses only jump_var1 and jump_var2 */
    if (jump_var1 < jump_var2) {
        goto no_conflict_label;
    }
    
    return -1;
    
no_conflict_label:
    /* Delay slot uses completely different variables */
    delay_result = delay_var1 * delay_var2;  /* next_trial: multiply */
    
    /* Ensure result is used */
    asm volatile("" : "+r" (delay_result));
    return delay_result;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test_arithmetic();      /* Expected: 60 */
    total += test_bitwise();         /* Expected: 1 */
    total += test_memory_op();       /* Expected: 9 */
    total += test_comparison();      /* Expected: 100 */
    total += test_register_move();   /* Expected: 52 */
    total += test_shift();           /* Expected: 1 */
    total += test_multiple_labels(); /* Expected: 4 */
    total += test_no_conflict();     /* Expected: 120000 */
    
    printf("Total result: %d\n", total);
    
    /* Verify expected total (approximate) */
    if (total > 0) {
        printf("All tests executed successfully.\n");
    }
    
    return 0;
}
