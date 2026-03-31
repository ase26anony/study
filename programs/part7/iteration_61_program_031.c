/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x = a;
    int y = b;
    int result = 0;
    
    if (x > y) {
        goto compute;
    }
    
    /* This should not be moved */
    x = y - 1;
    return x;
    
compute:
    /* Candidate for delay slot: simple arithmetic */
    result = x + y;  /* next_trial: add instruction */
    return result;
}

/* Test 2: Bitwise operations after label */
__attribute__((noinline))
static int test2(int val) {
    int mask = 0xFF;
    int shifted;
    
    if (val == 0) {
        goto apply_mask;
    }
    
    /* Different path */
    return val << 2;
    
apply_mask:
    /* Candidate: bitwise AND - safe, non-trapping */
    shifted = val & mask;  /* next_trial: and instruction */
    return shifted;
}

/* Test 3: Stack-based load/store operations */
__attribute__((noinline))
static int test3(int *arr, int idx) {
    int local[4] = {1, 2, 3, 4};
    int temp;
    
    if (idx < 0 || idx >= 4) {
        goto safe_load;
    }
    
    /* Array access in safe region */
    return arr[idx];
    
safe_load:
    /* Candidate: stack load - unlikely to trap */
    temp = local[0];  /* next_trial: load from stack */
    return temp + idx;
}

/* Test 4: Comparison operation after label */
__attribute__((noinline))
static int test4(int a, int b) {
    int cmp_result;
    
    if (a == b) {
        goto compare;
    }
    
    return a - b;
    
compare:
    /* Candidate: comparison sets condition codes */
    cmp_result = (a < b);  /* next_trial: compare instruction */
    return cmp_result ? 1 : 0;
}

/* Test 5: Multiple operations in loop with goto */
__attribute__((noinline))
static int test5(int n) {
    int i = 0;
    int sum = 0;
    int temp;
    
loop_start:
    if (i >= n) {
        goto done;
    }
    
    if (i % 2 == 0) {
        goto even_case;
    }
    
    /* Odd case */
    sum += i * 3;
    i++;
    goto loop_start;
    
even_case:
    /* Candidate: simple arithmetic in delay slot position */
    temp = i * 2;  /* next_trial: multiply/shift */
    sum += temp;
    i++;
    goto loop_start;
    
done:
    return sum;
}

/* Test 6: Register move operation */
__attribute__((noinline))
static int test6(int a, int b) {
    int reg1 = a;
    int reg2 = b;
    int result;
    
    if (reg1 > 100) {
        goto move_op;
    }
    
    return reg1 + reg2;
    
move_op:
    /* Candidate: register move/copy */
    result = reg2;  /* next_trial: move instruction */
    return result;
}

/* Test 7: Safe shift operation */
__attribute__((noinline))
static int test7(int value) {
    int shifted;
    
    if (value < 0) {
        goto shift_right;
    }
    
    return value;
    
shift_right:
    /* Candidate: logical shift - non-trapping */
    shifted = value >> 2;  /* next_trial: shift instruction */
    return shifted;
}

/* Test 8: Complex pattern with multiple labels */
__attribute__((noinline))
static int test8(int x) {
    int a = x;
    int b = x * 2;
    int c;
    
    if (a > 0) {
        goto label1;
    }
    
    return -1;
    
label1:
    /* First candidate */
    c = a + 5;  /* next_trial: add immediate */
    
    if (c > 10) {
        goto label2;
    }
    
    return c;
    
label2:
    /* Second candidate */
    b = c - 3;  /* Another potential next_trial */
    return b;
}

/* Main function to execute all tests */
int main(void) {
    int arr[4] = {10, 20, 30, 40};
    int total = 0;
    
    /* Execute all test functions */
    total += test1(5, 3);    /* Should take goto path */
    total += test1(2, 8);    /* Should not take goto path */
    
    total += test2(0x1234);  /* Should take goto path */
    total += test2(42);      /* Should not take goto path */
    
    total += test3(arr, -1); /* Should take goto path */
    total += test3(arr, 2);  /* Should not take goto path */
    
    total += test4(5, 5);    /* Should take goto path */
    total += test4(3, 7);    /* Should not take goto path */
    
    total += test5(10);      /* Mixed paths */
    
    total += test6(150, 25); /* Should take goto path */
    total += test6(50, 25);  /* Should not take goto path */
    
    total += test7(-8);      /* Should take goto path */
    total += test7(8);       /* Should not take goto path */
    
    total += test8(3);       /* Should take first goto */
    total += test8(10);      /* Should take both gotos */
    
    printf("Total result: %d\n", total);
    printf("All tests executed\n");
    
    return total != 0 ? 0 : 1;
}
