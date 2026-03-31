/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch test_delay_slot.c -o test
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 test_delay_slot.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x = 0, y = 0, z = 0;
    
    if (a > b) {
        goto target_label1;
    }
    
    x = a + b;
    return x;
    
target_label1:
    /* Candidate for delay slot: simple arithmetic */
    y = a - b;  /* next_trial: SUB instruction */
    z = y * 2;
    return z;
}

/* Test 2: Bitwise operations after label */
__attribute__((noinline))
static int test2(int val) {
    int mask = 0xFF;
    int result = 0;
    
    if (val == 0) {
        goto process;
    }
    
    result = val & 0x0F;
    return result;
    
process:
    /* Candidate: bitwise operations */
    result = val | mask;  /* OR instruction */
    result = result ^ 0xAA;  /* XOR instruction */
    return result;
}

/* Test 3: Safe stack load/store operations */
__attribute__((noinline))
static int test3(int *arr, int idx) {
    int local1 = 0, local2 = 0;
    int stack_var1 = 100;
    int stack_var2 = 200;
    
    if (idx < 0) {
        goto compute;
    }
    
    local1 = arr[idx];
    return local1;
    
compute:
    /* Candidate: stack-based operations */
    stack_var1 = stack_var2 + 50;  /* ADD using stack variables */
    local2 = stack_var1;
    return local2;
}

/* Test 4: Comparison operations */
__attribute__((noinline))
static int test4(int a, int b, int c) {
    int cmp_result = 0;
    
    if (a == 0) {
        goto compare_block;
    }
    
    return a + b;
    
compare_block:
    /* Candidate: comparison that sets condition codes */
    cmp_result = (b > c);  /* Sets condition flags */
    return cmp_result ? 1 : 0;
}

/* Test 5: Multiple safe instructions after label */
__attribute__((noinline))
static int test5(int x) {
    int a = 10, b = 20, c = 30;
    
    if (x % 2 == 0) {
        goto process_values;
    }
    
    return x;
    
process_values:
    /* Multiple simple instructions - first one is candidate */
    a = b + c;      /* First instruction after label - ADD */
    b = a << 2;     /* Shift instruction */
    c = b & 0xFF;   /* AND instruction */
    return a + b + c;
}

/* Test 6: Loop with internal goto to increase optimization chances */
__attribute__((noinline))
static int test6(int n) {
    int sum = 0;
    int i = 0;
    
    for (i = 0; i < n; i++) {
        if (i == n/2) {
            goto special_case;
        }
        sum += i;
    }
    
    return sum;
    
special_case:
    /* Candidate: simple arithmetic with different registers */
    sum = sum * 2;  /* MUL instruction */
    return sum + 1;
}

/* Test 7: Register move patterns */
__attribute__((noinline))
static int test7(int p1, int p2, int p3) {
    int r1 = p1, r2 = p2, r3 = p3;
    
    if (r1 > r2 && r2 > r3) {
        goto rearrange;
    }
    
    return r1 + r2 + r3;
    
rearrange:
    /* Candidate: register moves and swaps */
    r1 = r2;        /* Move instruction */
    r2 = r3;
    r3 = r1 ^ r2;   /* XOR instruction */
    return r3;
}

/* Test 8: Avoid resource conflicts by using fresh variables */
__attribute__((noinline))
static int test8(int in1, int in2) {
    /* Use completely separate variables for the jump condition
       and the candidate instruction to avoid resource conflicts */
    int cond_var1 = in1;
    int cond_var2 = in2;
    int work_var1, work_var2, work_var3;  /* Fresh variables */
    
    if (cond_var1 > cond_var2) {
        goto safe_work_area;
    }
    
    return cond_var1 - cond_var2;
    
safe_work_area:
    /* Candidate: uses fresh variables not involved in jump condition */
    work_var1 = 1000;
    work_var2 = work_var1 + 2000;  /* ADD with constants */
    work_var3 = work_var2 >> 3;    /* Shift instruction */
    return work_var3;
}

/* Test 9: Nested control flow to create more optimization opportunities */
__attribute__((noinline))
static int test9(int mode) {
    int result = 0;
    
    switch (mode) {
        case 1:
            goto op_add;
        case 2:
            goto op_sub;
        case 3:
            goto op_mul;
        default:
            return -1;
    }
    
op_add:
    result = 100 + 200;  /* ADD with constants */
    goto finish;
    
op_sub:
    result = 500 - 300;  /* SUB with constants */
    goto finish;
    
op_mul:
    result = 20 * 30;    /* MUL with constants */
    /* fall through */
    
finish:
    return result;
}

/* Test 10: Mixed operations ensuring no trapping */
__attribute__((noinline))
static int test10(int base) {
    int safe_array[4] = {1, 2, 3, 4};
    int index = 0;
    
    if (base > 1000) {
        goto safe_computation;
    }
    
    /* Safe array access - within bounds */
    return safe_array[base % 4];
    
safe_computation:
    /* Candidate: safe operations only */
    index = (base % 4);          /* MOD - safe with non-zero divisor */
    return safe_array[index] + 10;  /* ADD with array access */
}

int main(void) {
    int total = 0;
    int test_array[4] = {10, 20, 30, 40};
    
    /* Execute all tests to ensure code paths are taken */
    total += test1(100, 50);
    total += test2(0x55);
    total += test3(test_array, 2);
    total += test4(0, 10, 5);
    total += test5(4);
    total += test6(10);
    total += test7(3, 2, 1);
    total += test8(50, 25);
    total += test9(2);
    total += test10(100);
    
    /* Also test with different inputs */
    total += test1(50, 100);  /* Take different path */
    total += test2(0);        /* Take goto path */
    total += test4(1, 10, 5); /* Don't take goto */
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    /* Verify expected total to ensure all code executed */
    if (total != 0) {
        printf("All tests executed successfully.\n");
    }
    
    return 0;
}
