/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch test_delay_slot.c -o test_delay_slot
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 test_delay_slot.c -o test_delay_slot
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x = 0, y = 0, z = 0;
    
    /* Initialize variables to avoid undefined behavior */
    x = a + 1;
    y = b * 2;
    
    if (x > y) {
        goto target_label1;
    }
    
    /* Some code to make the jump non-trivial */
    z = x + y;
    if (z < 100) {
        return z;
    }
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    z = x - y;  /* next_trial: simple integer operation */
    
    /* Use result to prevent dead code elimination */
    return z + 1;
}

/* Test 2: Bitwise operations after label */
__attribute__((noinline))
static int test2(int a, int b) {
    int result = 0;
    int mask = 0xFF;
    
    if (a == 0) {
        goto compute;
    }
    
    /* Some computation */
    result = a & mask;
    if (result > 128) {
        return result;
    }
    
compute:
    /* Candidate: bitwise operation */
    result = b | 0x3F;  /* next_trial: bitwise OR */
    
    return result ^ 1;
}

/* Test 3: Safe stack load/store operations */
__attribute__((noinline))
static int test3(int val) {
    int local_array[4] = {1, 2, 3, 4};
    int temp = 0;
    int i = 0;
    
    /* Loop with internal goto to create jump opportunities */
    for (i = 0; i < 3; i++) {
        if (val == local_array[i]) {
            goto process;
        }
    }
    
    temp = local_array[0] + local_array[1];
    return temp;
    
process:
    /* Candidate: stack memory operation (should be safe) */
    temp = local_array[2];  /* next_trial: load from stack */
    
    return temp * 2;
}

/* Test 4: Comparison operation after label */
__attribute__((noinline))
static int test4(int a, int b, int c) {
    int cmp_result = 0;
    
    if (a > b) {
        goto compare;
    }
    
    cmp_result = (a < c) ? 1 : 0;
    return cmp_result;
    
compare:
    /* Candidate: comparison operation */
    cmp_result = (b > c);  /* next_trial: comparison sets condition codes */
    
    return cmp_result + 10;
}

/* Test 5: Multiple operations with distinct variables to avoid resource conflicts */
__attribute__((noinline))
static int test5(int p1, int p2, int p3) {
    /* Use distinct variables for jump condition and delay slot candidate */
    int jump_var = p1;
    int delay_var1 = p2;
    int delay_var2 = p3;
    int result = 0;
    
    /* Simple jump condition using only jump_var */
    if (jump_var > 100) {
        goto delay_slot_candidate;
    }
    
    result = delay_var1 + delay_var2;
    return result;
    
delay_slot_candidate:
    /* Candidate: operation on variables not used in jump condition */
    result = delay_var1 * delay_var2;  /* next_trial: multiplication */
    
    /* Ensure result is used */
    return result - 5;
}

/* Test 6: Nested control flow with safe operation */
__attribute__((optimize("O3")))
static int test6(int x) {
    int a = x;
    int b = x * 2;
    int c = 0;
    
    if (a > 50) {
        if (b < 100) {
            goto safe_op;
        }
        c = a - b;
    } else {
        c = a + b;
    }
    
    return c;
    
safe_op:
    /* Candidate: simple assignment/arithmetic */
    c = b >> 1;  /* next_trial: shift operation */
    
    return c + a;
}

/* Test 7: Switch-like pattern with goto */
__attribute__((noinline))
static int test7(int selector) {
    int val1 = 10, val2 = 20, val3 = 30;
    int output = 0;
    
    switch (selector % 3) {
        case 0:
            goto case0;
        case 1:
            goto case1;
        default:
            goto case2;
    }
    
case0:
    output = val1 + val2;  /* next_trial: addition */
    goto finish;
    
case1:
    output = val2 - val1;  /* next_trial: subtraction */
    goto finish;
    
case2:
    output = val1 & val3;  /* next_trial: bitwise AND */
    /* fall through */
    
finish:
    return output + selector;
}

/* Test 8: Loop with multiple goto targets */
__attribute__((noinline))
static int test8(int iterations) {
    int i, sum = 0;
    int temp = 0;
    
    for (i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            goto even_case;
        }
        
        sum += i;
        continue;
        
    even_case:
        /* Candidate: simple operation in loop */
        temp = i * 3;  /* next_trial: multiplication */
        sum += temp;
    }
    
    return sum;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    
    /* Execute all test functions with various inputs */
    total += test1(10, 5);
    total += test2(0, 42);
    total += test3(3);
    total += test4(15, 10, 20);
    total += test5(150, 6, 7);
    total += test6(60);
    total += test7(5);
    total += test8(10);
    
    /* Also test edge cases */
    total += test1(1, 100);
    total += test2(100, 0);
    total += test3(99);
    total += test4(5, 10, 15);
    total += test5(50, 3, 4);
    total += test6(30);
    total += test7(2);
    total += test8(5);
    
    printf("Total result: %d\n", total);
    printf("(This output ensures all code paths are executed)\n");
    
    return 0;
}
