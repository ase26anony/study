/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
static int test1(void) {
    int a = 10, b = 20, c = 0;
    volatile int trigger = 1; /* Prevent constant propagation */
    
    if (trigger) {
        goto target_label1;
    }
    
    /* Dead code that won't be executed */
    a = 100;
    b = 200;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    
    return c; /* Use result to prevent elimination */
}

/* Test 2: Bitwise operations after label */
static int test2(void) {
    uint32_t x = 0x12345678;
    uint32_t y = 0x87654321;
    uint32_t z = 0;
    volatile int flag = 1;
    
    if (flag > 0) {
        goto compute_label;
    }
    
    x = 0;
    y = 0;
    
compute_label:
    /* Candidate: bitwise operation */
    z = x ^ y;  /* XOR instruction */
    
    return (int)z;
}

/* Test 3: Safe stack load/store operations */
static int test3(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    volatile int cond = 1;
    
    if (cond) {
        goto process_label;
    }
    
    array[0] = 0;
    
process_label:
    /* Candidate: stack load operation */
    temp = array[2];  /* Load from stack (safe) */
    
    return temp;
}

/* Test 4: Comparison operation after label */
static int test4(void) {
    int p = 100, q = 50;
    int result = 0;
    volatile int check = 1;
    
    if (check != 0) {
        goto compare_label;
    }
    
    p = 0;
    q = 0;
    
compare_label:
    /* Candidate: comparison sets condition codes */
    result = (p > q);  /* Comparison instruction */
    
    return result;
}

/* Test 5: Register move operation */
static int test5(void) {
    int src = 999;
    int dst = 0;
    volatile int move_flag = 1;
    
    if (move_flag) {
        goto move_label;
    }
    
    src = 0;
    
move_label:
    /* Candidate: register move */
    dst = src;  /* Move instruction */
    
    return dst;
}

/* Test 6: Multiple operations in loop with goto */
static int test6(void) {
    int i, sum = 0;
    volatile int loop_cond = 5;
    
    for (i = 0; i < loop_cond; i++) {
        if (i & 1) {
            goto odd_case;
        }
        
        sum += i * 2;
        continue;
        
    odd_case:
        /* Candidate: arithmetic in delay slot position */
        sum += i * 3;  /* Multiply-add pattern */
    }
    
    return sum;
}

/* Test 7: Nested jumps with safe operation */
static int test7(void) {
    int val1 = 42, val2 = 17;
    int output = 0;
    volatile int selector = 1;
    
    if (selector == 1) {
        goto case1;
    } else if (selector == 2) {
        goto case2;
    } else {
        goto default_case;
    }

case1:
    /* First candidate */
    output = val1 - val2;  /* Subtract instruction */
    goto end;

case2:
    /* Second candidate */
    output = val1 | val2;  /* Bitwise OR */
    goto end;

default_case:
    output = val1 & val2;  /* Bitwise AND */
    /* Fall through */

end:
    return output;
}

/* Test 8: Avoid resource conflicts by using fresh variables */
static int test8(void) {
    /* Variables for jump condition */
    int cond_a = 10, cond_b = 20;
    
    /* Fresh variables for delay slot candidate */
    int fresh_x = 30, fresh_y = 40, fresh_z;
    volatile int do_jump = 1;
    
    /* Jump condition uses different variables */
    if (do_jump && (cond_a < cond_b)) {
        goto fresh_op_label;
    }
    
    fresh_x = 0;
    fresh_y = 0;
    
fresh_op_label:
    /* Candidate uses fresh variables - no resource conflicts */
    fresh_z = fresh_x * fresh_y;  /* Multiply instruction */
    
    return fresh_z;
}

/* Test 9: Shift operation */
static int test9(void) {
    unsigned int bits = 0xF0F0F0F0;
    unsigned int shifted;
    volatile int shift_amount = 4;
    
    if (shift_amount > 0) {
        goto shift_label;
    }
    
    bits = 0;
    
shift_label:
    /* Candidate: shift operation */
    shifted = bits >> shift_amount;  /* Shift right */
    
    return (int)shifted;
}

/* Test 10: Complex pattern with multiple labels */
__attribute__((optimize("O3")))
static int test10(void) {
    int counter = 0;
    int a = 1, b = 2, c = 3, d = 4;
    volatile int iterations = 3;
    
    while (counter < iterations) {
        if (counter == 0) {
            goto block_a;
        } else if (counter == 1) {
            goto block_b;
        } else {
            goto block_c;
        }
        
    block_a:
        a = b + c;  /* Candidate 1 */
        goto next;
        
    block_b:
        b = c ^ d;  /* Candidate 2 */
        goto next;
        
    block_c:
        c = a << 2; /* Candidate 3 */
        /* Fall through */
        
    next:
        counter++;
    }
    
    return a + b + c + d;
}

int main(void) {
    int total = 0;
    
    /* Execute all tests to ensure code paths are taken */
    total += test1();
    total += test2();
    total += test3();
    total += test4();
    total += test5();
    total += test6();
    total += test7();
    total += test8();
    total += test9();
    total += test10();
    
    printf("Total: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
