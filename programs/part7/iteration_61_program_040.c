/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x, y, z;
    
    x = a + b;
    y = a - b;
    
    if (x > y) {
        goto target_label1;
    }
    
    /* Dead code to avoid fall-through optimization */
    return 0;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    z = x * 2;  /* This should be movable into delay slot */
    return z + y;
}

/* Test 2: Bitwise operations after label */
__attribute__((noinline))
static int test2(int a, int b) {
    int result = 0;
    int mask = 0xFF;
    
    if (a & 1) {
        goto bitwise_label;
    }
    
    return -1;
    
bitwise_label:
    /* Candidate: bitwise operation */
    result = (a & mask) | (b << 3);
    return result;
}

/* Test 3: Safe stack load/store operations */
__attribute__((noinline))
static int test3(int *arr, int n) {
    int local_arr[4] = {1, 2, 3, 4};
    int temp, i = 0;
    
    if (n > 0) {
        goto compute_label;
    }
    
    return 0;
    
compute_label:
    /* Candidate: stack-based memory operation */
    temp = local_arr[i] + local_arr[i+1];  /* Safe stack access */
    return temp * n;
}

/* Test 4: Comparison operation after label */
__attribute__((noinline))
static int test4(int a, int b, int c) {
    int cmp_result;
    
    if (a > b) {
        goto compare_label;
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (c == 0) ? 1 : 0;
    return cmp_result + a;
}

/* Test 5: Register move pattern */
__attribute__((noinline))
static int test5(int val) {
    int src, dst;
    
    src = val * 3;
    
    if (src > 100) {
        goto move_label;
    }
    
    return src;
    
move_label:
    /* Candidate: simple register move operation */
    dst = src;  /* Should compile to a move instruction */
    return dst + 1;
}

/* Test 6: Loop with internal goto to increase optimization chances */
__attribute__((noinline))
static int test6(int iterations) {
    int sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        if (i & 1) {
            goto loop_label;
        }
        
        sum += i;
        continue;
        
    loop_label:
        /* Candidate in loop context */
        sum += (i * 2);  /* Simple arithmetic */
    }
    
    return sum;
}

/* Test 7: Multiple labels and jumps */
__attribute__((noinline))
static int test7(int a, int b) {
    int x = a, y = b;
    
    if (x > y) goto L1;
    if (x < y) goto L2;
    
    return 0;
    
L1:
    /* First candidate */
    x = x + y;
    goto end;
    
L2:
    /* Second candidate */
    y = y - x;
    /* fall through */
    
end:
    return x + y;
}

/* Test 8: Nested condition with safe operation */
__attribute__((noinline))
static int test8(int flag, int base) {
    int result = base;
    
    if (flag) {
        if (base > 10) {
            goto safe_op;
        }
        return result;
    }
    
    return -1;
    
safe_op:
    /* Candidate: safe shift operation */
    result = result << 2;  /* No trapping possible */
    return result;
}

/* Test 9: Avoid resource conflicts by using fresh variables */
__attribute__((noinline))
static int test9(int in1, int in2) {
    /* Use distinct variables to avoid resource conflicts */
    int cond_var = in1 + in2;
    int work_var1, work_var2;
    
    work_var1 = in1 * 2;
    work_var2 = in2 * 3;
    
    if (cond_var > 50) {
        goto compute;
    }
    
    return work_var1;
    
compute:
    /* Candidate: uses variables not involved in jump condition */
    int fresh_result = work_var1 + work_var2;
    return fresh_result;
}

/* Test 10: Multiple basic blocks to encourage reorg */
__attribute__((noinline))
static int test10(int selector) {
    int a = 1, b = 2, c = 3, d = 4;
    int ret = 0;
    
    switch (selector & 3) {
        case 0:
            goto case0;
        case 1:
            goto case1;
        case 2:
            goto case2;
        default:
            goto default_case;
    }
    
case0:
    ret = a + b;
    goto end;
    
case1:
    ret = b + c;
    goto end;
    
case2:
    ret = c + d;
    goto end;
    
default_case:
    ret = d + a;
    /* fall through */
    
end:
    return ret;
}

/* Main function that exercises all patterns */
int main(void) {
    int arr[4] = {10, 20, 30, 40};
    int total = 0;
    
    /* Execute all tests to ensure code paths are taken */
    total += test1(100, 50);
    total += test2(255, 8);
    total += test3(arr, 4);
    total += test4(15, 10, 0);
    total += test5(35);
    total += test6(10);
    total += test7(25, 15);
    total += test8(1, 12);
    total += test9(20, 15);
    total += test10(2);
    
    printf("Total result: %d\n", total);
    printf("(This output ensures all code paths are executed)\n");
    
    return total != 0 ? 0 : 1;
}
