/* Test program for triggering delay slot filling logic in GCC's reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((optimize("O2")))
int test1(int a, int b) {
    int x = 0, y = 0, z = 0;
    volatile int result = 0; /* volatile to prevent optimization */
    
    /* Initialize variables with distinct values */
    x = a + 1;
    y = b * 2;
    
    /* Create control flow with goto to label */
    if (x > y) {
        goto compute;
    } else {
        x = y - 1;
    }
    
    /* This should be the candidate for delay slot filling */
compute:
    z = x + y;  /* Simple arithmetic - good candidate for next_trial */
    result = z;
    
    /* Use result to prevent dead code elimination */
    return result + (x > y ? 1 : 0);
}

/* Test 2: Bitwise operations after label */
__attribute__((optimize("O3")))
int test2(int a, int b) {
    int temp1 = a, temp2 = b;
    int mask = 0xFF;
    int result = 0;
    
    /* Multiple basic blocks to encourage reorg */
    if (temp1 != 0) {
        if (temp2 > 10) {
            goto bitop;
        }
        temp1 = temp2 + 5;
    }
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < 3; i++) {
        if (i == 2) {
            goto bitop;
        }
        temp1++;
    }
    
bitop:
    /* Bitwise operation - safe, non-trapping */
    result = (temp1 & mask) | (temp2 << 2);
    
    /* Prevent elimination */
    return result ^ (temp1 * temp2);
}

/* Test 3: Safe stack memory operation */
__attribute__((optimize("O2")))
int test3(void) {
    int arr[4] = {1, 2, 3, 4};
    int idx = 0;
    int sum = 0;
    
    /* Control flow with goto */
    if (arr[0] > 0) {
        idx = 1;
        goto load_op;
    }
    
    idx = 2;
    
load_op:
    /* Stack load - less likely to trap than heap/global access */
    int val = arr[idx];  /* Safe: idx is 0,1,2 - within bounds */
    sum = val + idx;
    
    /* Use in computation */
    for (int i = 0; i < 2; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* Test 4: Comparison operation */
__attribute__((optimize("O2")))
int test4(int a, int b, int c) {
    int cmp1 = a, cmp2 = b;
    int flag = 0;
    
    /* Nested control flow */
    if (c > 0) {
        if (cmp1 < cmp2) {
            goto compare;
        } else {
            cmp1 = c;
        }
    }
    
    /* Another path to the label */
    if (c < 100) {
        cmp2 = c * 2;
        goto compare;
    }
    
compare:
    /* Comparison sets condition codes without side effects */
    flag = (cmp1 == cmp2);  /* Comparison operation */
    
    /* Use flag to affect control flow */
    if (flag) {
        return cmp1 + 1;
    }
    return cmp2 - 1;
}

/* Test 5: Register move pattern */
__attribute__((optimize("O3")))
int test5(int p1, int p2, int p3) {
    int r1 = p1, r2 = p2, r3 = p3;
    int out = 0;
    
    /* Complex control flow to create jump to label */
    switch (r1 % 4) {
        case 0:
            r2 = r3 + 1;
            goto move_op;
        case 1:
            r3 = r2 - 1;
            break;
        case 2:
            goto move_op;
        default:
            r1 = r2 + r3;
    }
    
    if (r1 > r2) {
        r3 = 100;
        goto move_op;
    }
    
move_op:
    /* Simple register move operation */
    out = r1;  /* Move operation - excellent candidate */
    
    /* Use out in computation */
    for (int i = 0; i < 2; i++) {
        out += r2 + r3;
    }
    
    return out;
}

/* Test 6: Multiple candidate instructions in sequence */
__attribute__((optimize("O2")))
int test6(int base) {
    int a = base, b = base + 1, c = base + 2;
    int t1, t2, t3;
    
    /* Multiple labels with simple instructions */
    if (a > 10) {
        goto block1;
    }
    
    b = a * 2;
    
block1:
    t1 = b + c;  /* First candidate */
    
    if (t1 < 50) {
        goto block2;
    }
    
    c = t1 / 2;  /* Avoid division in candidate itself */
    
block2:
    t2 = a | b;  /* Second candidate - bitwise OR */
    
    /* Force another jump */
    if (t2 > t1) {
        goto block3;
    }
    
block3:
    t3 = t1 ^ t2;  /* Third candidate - XOR */
    
    return t1 + t2 + t3;
}

/* Test 7: Avoid resource conflicts explicitly */
__attribute__((optimize("O2")))
int test7(int x, int y) {
    /* Use completely separate variables for the candidate */
    int jump_var = x;      /* Used in jump condition */
    int safe_var1 = y;     /* Used after label - no overlap */
    int safe_var2 = 42;    /* Constant - safe */
    int result = 0;
    
    /* The jump condition uses jump_var only */
    if (jump_var > 0) {
        /* Candidate uses different variables */
        goto safe_operation;
    }
    
    /* Modify jump_var in other path */
    jump_var = y * 2;
    
safe_operation:
    /* Uses variables not involved in jump condition */
    result = safe_var1 + safe_var2;  /* No resource conflict */
    
    /* Use result with jump_var to prevent elimination */
    return result * (jump_var > 0 ? 1 : -1);
}

/* Test 8: Try split pattern with simple expression */
__attribute__((optimize("O3")))
int test8(int val) {
    int a = val, b = val * 2, c = val * 3;
    int tmp;
    
    /* Multiple basic blocks */
    if (a < 10) {
        if (b > 5) {
            goto expr;
        }
        a = b + c;
    }
    
    for (int i = 0; i < 2; i++) {
        if (i == 1) {
            b = a + i;
            goto expr;
        }
        c++;
    }
    
expr:
    /* Expression that try_split might successfully split */
    tmp = (a << 2) + (b >> 1);  /* Shift operations */
    
    /* Use in loop to prevent dead code */
    int sum = 0;
    for (int j = 0; j < 3; j++) {
        sum += tmp + j;
    }
    
    return sum;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions with different inputs */
    total += test1(10, 5);
    total += test2(7, 20);
    total += test3();
    total += test4(5, 5, 10);
    total += test5(1, 2, 3);
    total += test6(8);
    total += test7(1, 2);
    total += test8(4);
    
    printf("Total result: %d\n", total);
    
    /* Verify all paths were executed */
    if (total != 0) {
        printf("All tests executed successfully.\n");
    }
    
    return 0;
}
