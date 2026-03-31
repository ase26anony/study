/* Test program for triggering delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2", "no-gcse", "no-crossjumping")))
#else
#define OPTIMIZE_O2
#endif

/* Test 1: Simple arithmetic after label */
OPTIMIZE_O2
static int test1(int a, int b) {
    volatile int result = 0;
    int x = a, y = b;
    
    if (x > y) {
        goto target_label1;
    }
    
    /* Some code to avoid fall-through optimization */
    x = y * 2;
    return x;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic */
    int z = x + y;  /* Simple add - should be eligible */
    result = z;
    
    /* Avoid tail merging */
    if (result > 1000) {
        return result + 1;
    }
    return result;
}

/* Test 2: Bitwise operations after label */
OPTIMIZE_O2  
static int test2(int a, int b) {
    int temp1 = a, temp2 = b;
    volatile int out = 0;
    
    /* Create jump to label */
    if (temp1 != 0) {
        goto compute_label;
    }
    
    temp1 = temp2 | 0xFF;
    return temp1;
    
compute_label:
    /* Candidate: bitwise operation */
    int mask = temp1 & 0x0F;  /* Simple AND - safe */
    out = mask;
    
    /* Prevent dead code elimination */
    for (int i = 0; i < 2; i++) {
        out += i;
    }
    return out;
}

/* Test 3: Register move pattern */
OPTIMIZE_O2
static int test3(int a, int b, int c) {
    int r1 = a, r2 = b, r3 = c;
    volatile int sum = 0;
    
    /* Multiple jumps to same label */
    if (r1 > r2) {
        goto process;
    }
    
    if (r2 < r3) {
        goto process;
    }
    
    r1 = r2 + r3;
    return r1;
    
process:
    /* Candidate: move between registers */
    int t = r1;  /* Simple move - should pass resource checks */
    sum = t + r3;
    
    /* Use result in computation */
    return sum * 2 - 1;
}

/* Test 4: Stack-based memory operation (safe load) */
OPTIMIZE_O2
static int test4(int a) {
    int array[4] = {a, a+1, a+2, a+3};
    volatile int idx = 0;
    int result = 0;
    
    if (a % 2 == 0) {
        goto load_op;
    }
    
    idx = 1;
    goto load_op;
    
    /* Unreachable but prevents optimization */
    return array[0];
    
load_op:
    /* Candidate: stack load - unlikely to trap */
    int val = array[idx];  /* Stack access, not through pointer */
    result = val * 2;
    
    /* Prevent merging with other blocks */
    switch (result % 3) {
        case 0: return result + 1;
        case 1: return result + 2;
        default: return result + 3;
    }
}

/* Test 5: Comparison operation */
OPTIMIZE_O2
static int test5(int x, int y) {
    volatile int cmp_result = 0;
    int a = x, b = y;
    
    /* Loop with internal goto */
    for (int i = 0; i < 3; i++) {
        if (a == b + i) {
            goto compare_label;
        }
    }
    
    a = b - 1;
    return a;
    
compare_label:
    /* Candidate: comparison operation */
    int is_less = (a < b);  /* Sets condition codes, no memory */
    cmp_result = is_less;
    
    /* Use result */
    return cmp_result ? a : b;
}

/* Test 6: Multiple safe operations in sequence */
OPTIMIZE_O2
static int test6(int p1, int p2, int p3) {
    int v1 = p1, v2 = p2, v3 = p3;
    volatile int acc = 0;
    
    /* Nested conditions leading to same label */
    if (v1 > 0) {
        if (v2 > 0) {
            if (v3 > 0) {
                goto safe_ops;
            }
        }
    }
    
    v1 = v2 = v3 = 0;
    return -1;
    
safe_ops:
    /* Multiple simple operations - compiler might split */
    int tmp = v1 ^ v2;      /* XOR operation */
    acc = tmp;
    tmp = tmp | v3;         /* OR operation */
    
    /* Complex enough to potentially split but safe */
    return (acc + tmp) & 0xFF;
}

/* Test 7: Avoid trapping operations - use shift instead of division */
OPTIMIZE_O2
static int test7(int a, int b) {
    int x = a, y = b;
    volatile int res = 0;
    
    /* Force jump */
    if (x == 0) {
        goto no_div_label;
    }
    
    if (y == 0) {
        goto no_div_label;
    }
    
    /* Use shift (non-trapping) instead of division */
    goto shift_label;
    
    return x / y;  /* This would trap but is unreachable */
    
no_div_label:
    x = 1;
    y = 1;
    
shift_label:
    /* Candidate: shift operation - doesn't trap */
    int shifted = x << 2;  /* Shift, not division */
    res = shifted + y;
    
    return res;
}

/* Main driver that executes all tests */
int main(void) {
    int total = 0;
    
    /* Run tests with various inputs to exercise different paths */
    total += test1(10, 5);    /* x > y case */
    total += test1(5, 10);    /* x <= y case */
    
    total += test2(0x1234, 0x5678);
    total += test2(0, 0xABCD);
    
    total += test3(100, 50, 25);
    total += test3(25, 50, 100);
    
    total += test4(10);
    total += test4(11);
    
    total += test5(42, 42);
    total += test5(100, 200);
    
    total += test6(1, 2, 3);
    total += test6(-1, -2, -3);
    
    total += test7(5, 3);
    total += test7(0, 5);
    
    printf("Total result: %d\n", total);
    printf("All tests executed - check for delay slot filling in assembly output\n");
    
    /* Also print assembly hint */
    printf("\nCompile with: gcc -O2 -march=mips32 -S -fverbose-asm test.c\n");
    printf("Then examine .s file for delay slot patterns\n");
    
    return total != 0 ? 0 : 1;
}
