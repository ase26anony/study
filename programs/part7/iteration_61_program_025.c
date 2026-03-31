/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x, y, z;
    volatile int result = 0; /* volatile to prevent optimization */
    
    x = a + 1;
    y = b * 2;
    
    if (x > y) {
        /* Simple jump to label */
        goto target_label1;
    }
    
    /* Some code to avoid fall-through optimization */
    z = x - y;
    result += z;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* Doesn't reference resources used by the jump */
    z = a + b;  /* Uses original parameters, not x/y from jump condition */
    result += z;
    return result;
}

/* Test 2: Bitwise operations after label */
__attribute__((noinline))
static int test2(unsigned int mask, unsigned int value) {
    unsigned int temp1, temp2, result = 0;
    
    temp1 = mask & 0xFF;
    if (temp1 == 0) {
        goto bitwise_label;
    }
    
    temp2 = value >> 8;
    result = temp1 + temp2;
    return result;
    
bitwise_label:
    /* Safe bitwise operation - no trapping */
    temp2 = value & 0x0F;  /* Masking operation */
    result = temp2 * 2;
    return result;
}

/* Test 3: Stack-based memory operation (safe load/store) */
__attribute__((noinline))
static int test3(int init) {
    int local1, local2, local3;
    int array[4] = {init, init+1, init+2, init+3};
    
    local1 = array[0];
    local2 = array[1];
    
    if (local1 < local2) {
        goto mem_op_label;
    }
    
    local3 = array[2] + array[3];
    return local3;
    
mem_op_label:
    /* Safe stack memory access - won't fault */
    local3 = array[0] + array[1];  /* Using known-safe indices */
    return local3;
}

/* Test 4: Comparison operation setting condition codes */
__attribute__((noinline))
static int test4(int a, int b, int c) {
    int cmp1, cmp2, result = 0;
    
    cmp1 = a - b;
    if (cmp1 > 0) {
        goto compare_label;
    }
    
    cmp2 = b - c;
    result = cmp2 * 2;
    return result;
    
compare_label:
    /* Comparison operation - sets flags but no side effects */
    cmp2 = (c > a) ? 1 : 0;  /* Simple comparison */
    result = cmp2 + 10;
    return result;
}

/* Test 5: Multiple operations in sequence with loop */
__attribute__((noinline))
static int test5(int iterations) {
    int i, sum = 0;
    int temp1, temp2;
    
    for (i = 0; i < iterations; i++) {
        temp1 = i * 2;
        temp2 = i + 5;
        
        if (temp1 > temp2) {
            goto loop_label;
        }
        
        sum += temp1;
        continue;
        
    loop_label:
        /* Safe arithmetic in loop context */
        sum += temp2 - temp1;  /* Uses loop variables */
    }
    
    return sum;
}

/* Test 6: Register move pattern */
__attribute__((noinline))
static int test6(int val) {
    int a, b, c, d;
    
    a = val;
    b = val + 10;
    
    if (a % 2 == 0) {
        goto move_label;
    }
    
    c = b * 2;
    return c;
    
move_label:
    /* Simple register-to-register move pattern */
    d = a;  /* Move operation */
    c = d + 1;
    return c;
}

/* Test 7: Nested jumps with safe instruction */
__attribute__((noinline))
static int test7(int x, int y) {
    int result = 0;
    
    if (x > 100) {
        if (y < 50) {
            goto nested_label;
        }
        result = x - y;
        return result;
    }
    
    result = x + y;
    return result;
    
nested_label:
    /* Very simple safe instruction */
    result = 1;  /* Constant assignment */
    return result;
}

/* Test 8: Switch-like pattern with goto */
__attribute__((noinline))
static int test8(int code) {
    int result = 0;
    
    switch (code & 3) {
        case 0:
            goto case0_label;
        case 1:
            result = code * 2;
            break;
        case 2:
            result = code + 10;
            break;
        default:
            result = -1;
            break;
    }
    return result;
    
case0_label:
    /* Safe shift operation */
    result = code << 2;  /* Shift, no trapping */
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int total = 0;
    
    /* Execute all test functions with various inputs */
    total += test1(10, 20);
    total += test2(0xABCD, 0x1234);
    total += test3(5);
    total += test4(15, 10, 5);
    total += test5(10);
    total += test6(42);
    total += test7(150, 25);
    total += test8(5);
    
    /* Also test edge cases */
    total += test1(0, 0);
    total += test2(0, 0xFFFFFFFF);
    total += test3(-1);
    total += test4(0, 0, 0);
    total += test5(1);
    total += test6(0);
    total += test7(0, 100);
    total += test8(0);
    
    printf("Total result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
