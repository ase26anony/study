/* Test program for triggering delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdint.h>

/* Force optimization on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2")))
#else
#define OPTIMIZE_O2
#endif

/* Test 1: Simple arithmetic after label */
OPTIMIZE_O2
static int test1(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    if (a < b) {
        goto target_label1;
    }
    
    /* This should be dead code */
    result = -1;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c;
    
    /* Prevent tail call optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 2: Bitwise operations after label */
OPTIMIZE_O2
static int test2(void) {
    volatile int x = 0x55, y = 0xAA;
    int z = 0;
    int result = 0;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 2; i++) {
        if (x != y) {
            goto target_label2;
        }
        /* Never reached */
        result = -1;
    }
    
    return result;
    
target_label2:
    /* Candidate: bitwise operation */
    z = x & y;  /* next_trial: and instruction */
    result = z ^ 0xFF;
    
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 3: Register move pattern */
OPTIMIZE_O2
static int test3(void) {
    volatile int src = 100;
    int dst1 = 0, dst2 = 0;
    
    /* Multiple basic blocks to create jump opportunities */
    if (src > 50) {
        if (src < 200) {
            goto target_label3;
        }
    }
    
    dst1 = -1;
    return dst1;
    
target_label3:
    /* Candidate: move-like operation */
    dst2 = src;  /* next_trial: move instruction */
    
    /* Use result to prevent elimination */
    int result = dst2 * 2;
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 4: Stack-based memory operation (safe load) */
OPTIMIZE_O2
static int test4(void) {
    volatile int array[4] = {1, 2, 3, 4};
    volatile int index = 2;
    int temp = 0;
    
    /* Create jump with simple condition */
    if (index >= 0 && index < 4) {
        goto target_label4;
    }
    
    return -1;
    
target_label4:
    /* Candidate: safe memory load from stack */
    temp = array[index];  /* next_trial: load instruction */
    
    int result = temp + 10;
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 5: Comparison operation */
OPTIMIZE_O2
static int test5(void) {
    volatile int p = 5, q = 7;
    int cmp_result = 0;
    
    /* Nested conditions to create jump */
    if (p != 0) {
        if (q != 0) {
            goto target_label5;
        }
    }
    
    return -1;
    
target_label5:
    /* Candidate: comparison operation */
    cmp_result = (p < q);  /* next_trial: compare instruction */
    
    int result = cmp_result ? 100 : 200;
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 6: Shift operation */
OPTIMIZE_O2
static int test6(void) {
    volatile int val = 0x1234;
    int shifted = 0;
    
    /* Simple unconditional jump pattern */
    if (1) {  /* Always true */
        goto target_label6;
    }
    
    /* Unreachable */
    return -1;
    
    /* Another basic block to prevent fall-through */
    shifted = -1;
    return shifted;
    
target_label6:
    /* Candidate: shift operation */
    shifted = val << 2;  /* next_trial: shift instruction */
    
    int result = shifted | 0x1;
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 7: Multiple operations in sequence */
OPTIMIZE_O2
static int test7(void) {
    volatile int a = 3, b = 4, c = 5;
    int r1 = 0, r2 = 0;
    
    /* Switch-like pattern with goto */
    switch (a) {
        case 3:
            goto target_label7;
        default:
            return -1;
    }
    
target_label7:
    /* First simple operation - candidate for delay slot */
    r1 = b + c;  /* next_trial: add instruction */
    
    /* Follow with another operation to create basic block */
    r2 = r1 * 2;
    
    asm volatile("" : "+r"(r2));
    return r2;
}

/* Test 8: Avoid resource conflicts with jump condition */
OPTIMIZE_O2
static int test8(void) {
    /* Use distinct variables for jump condition and delay candidate */
    volatile int cond_var = 1;      /* Used in jump condition only */
    volatile int data1 = 10;        /* Used in delay candidate only */
    volatile int data2 = 20;        /* Used in delay candidate only */
    int compute = 0;
    
    /* Jump condition uses only cond_var */
    if (cond_var > 0) {
        goto target_label8;
    }
    
    return -1;
    
target_label8:
    /* Delay candidate uses different variables to avoid conflicts */
    compute = data1 - data2;  /* next_trial: subtract instruction */
    
    int result = compute + 100;
    asm volatile("" : "+r"(result));
    return result;
}

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test1();
    total += test2();
    total += test3();
    total += test4();
    total += test5();
    total += test6();
    total += test7();
    total += test8();
    
    /* Print result to ensure code isn't optimized away */
    printf("Total: %d\n", total);
    
    /* Also use volatile to force computation */
    volatile int check = total;
    if (check > 0) {
        return 0;
    }
    return 1;
}
