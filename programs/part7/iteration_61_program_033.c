/* Test program for triggering delay slot optimization conditions in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
int test1(int a, int b) {
    int x = 0, y = 0, z = 0;
    
    /* Initialize variables to avoid undefined behavior */
    x = a;
    y = b;
    
    /* Create a simple jump to label */
    if (x > y) {
        goto target1;
    }
    
    /* Some intermediate code */
    z = x + y;
    
    /* This is the jump target with simple instruction */
target1:
    /* Candidate for next_trial: simple arithmetic */
    z = x - y;  /* Simple subtraction - likely safe to move */
    
    /* Use result to prevent dead code elimination */
    return z + 1;
}

/* Test 2: Bitwise operations after label */
__attribute__((optimize("O2")))
int test2(int a, int b) {
    int result = 0;
    int temp1 = a, temp2 = b;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 3; i++) {
        if (temp1 & 0x1) {
            goto compute;
        }
        temp1 >>= 1;
    }
    
    temp2 = temp1 * 2;
    return temp2;

compute:
    /* Candidate: bitwise operation */
    result = temp1 | 0xFF;  /* Simple OR operation */
    
    /* Follow with return to avoid fall-through issues */
    return result;
}

/* Test 3: Safe memory operation (stack-based) */
__attribute__((optimize("O2")))
int test3(int a, int b) {
    /* Use local variables for stack operations */
    int local1 = a;
    int local2 = b;
    int local3 = 0;
    int local_array[4] = {a, b, a+b, a-b};
    
    /* Conditional jump */
    if (local1 != local2) {
        goto process;
    }
    
    local3 = local_array[0] + local_array[1];
    return local3;

process:
    /* Candidate: safe stack memory load */
    local3 = local_array[2];  /* Load from stack array - unlikely to trap */
    
    /* Simple arithmetic to use the value */
    return local3 * 2;
}

/* Test 4: Comparison operation */
__attribute__((optimize("O2")))
int test4(int a, int b, int c) {
    int x = a, y = b, z = c;
    int flag = 0;
    
    /* Multiple jumps to same label */
    if (x > 100) {
        goto check;
    }
    
    if (y < 50) {
        goto check;
    }
    
    z = x + y;
    return z;

check:
    /* Candidate: comparison operation (sets condition codes) */
    flag = (z == 0);  /* Simple comparison */
    
    /* Use the flag */
    return flag ? x : y;
}

/* Test 5: Register move pattern */
__attribute__((optimize("O2")))
int test5(int a, int b) {
    volatile int sink;  /* Prevent optimization */
    int r1 = a, r2 = b, r3 = 0, r4 = 0;
    
    /* Create control flow with goto */
    if (r1 > r2) {
        r3 = r1 - r2;
        if (r3 > 10) {
            goto move_op;
        }
    }
    
    r4 = r1 + r2;
    sink = r4;
    return r4;

move_op:
    /* Candidate: simple register move/assignment */
    r4 = r3;  /* Simple move - very safe */
    
    sink = r4;
    return r4 + 1;
}

/* Test 6: Multiple basic blocks with safe operations */
__attribute__((optimize("O3")))
int test6(int n) {
    int i = n;
    int acc = 0;
    int tmp1, tmp2;
    
    /* Unrolled loop pattern */
    if (i & 0x1) {
        goto block1;
    }
    
    acc += i;
    i--;
    
    if (i > 0) {
        goto block2;
    }
    
    return acc;

block1:
    /* Candidate: safe shift operation */
    tmp1 = i << 2;  /* Shift operation */
    acc += tmp1;
    i--;
    
    if (i <= 0) return acc;
    
block2:
    /* Another candidate: logical operation */
    tmp2 = i & 0x0F;  /* Mask operation */
    acc += tmp2;
    
    return acc;
}

/* Test 7: Avoid resource conflicts by using fresh variables */
__attribute__((optimize("O2")))
int test7(int a, int b) {
    /* Use completely separate variables for the jump condition
       and the operation after the label to avoid conflicts */
    int cond_var1 = a;
    int cond_var2 = b;
    
    /* Variables only used after label */
    int work_var1, work_var2, work_var3;
    
    if (cond_var1 == cond_var2) {
        goto safe_work;
    }
    
    /* Different work here */
    work_var1 = cond_var1 * 2;
    return work_var1;

safe_work:
    /* Candidate: operation on fresh variables */
    work_var2 = a + 1;    /* Using parameter directly */
    work_var3 = work_var2 * 3;
    
    return work_var3;
}

/* Test 8: Nested control flow with simple operation */
__attribute__((optimize("O2")))
int test8(int x) {
    int a = x, b = 0, c = 0;
    
    switch (a % 4) {
        case 0:
            b = a + 1;
            break;
        case 1:
            goto do_op;
        case 2:
            b = a - 1;
            break;
        default:
            goto do_op;
    }
    
    c = b * 2;
    return c;

do_op:
    /* Candidate: simple increment */
    b = a + 2;  /* Very simple operation */
    
    return b;
}

/* Main function to execute all tests */
int main() {
    int total = 0;
    int i;
    
    /* Seed for pseudo-random behavior */
    srand(42);
    
    /* Execute tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int val1 = rand() % 100;
        int val2 = rand() % 100;
        int val3 = rand() % 100;
        
        total += test1(val1, val2);
        total += test2(val2, val3);
        total += test3(val1, val3);
        total += test4(val1, val2, val3);
        total += test5(val2, val1);
        total += test6(val1);
        total += test7(val2, val3);
        total += test8(val3);
    }
    
    printf("Total result: %d\n", total);
    printf("All tests executed.\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
