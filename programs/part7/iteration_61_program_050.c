/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

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
    volatile int a = 5, b = 3, c = 0;
    int result = 0;
    
    if (a > b) {
        goto arithmetic_label;
    }
    
    return 0;
    
arithmetic_label:
    /* Candidate for delay slot: simple arithmetic */
    c = a + b;  /* next_trial: add instruction */
    result = c * 2;
    
    return result;
}

/* Test 2: Bitwise operations after label */
OPTIMIZE_O2  
static int test2(void) {
    volatile int x = 0xABCD, y = 0x1234;
    int mask = 0xFF;
    int res = 0;
    
    if (x != 0) {
        goto bitwise_label;
    }
    
    return 0;
    
bitwise_label:
    /* Candidate: bitwise operation */
    res = (x & mask) | y;  /* next_trial: bitwise ops */
    return res ^ 0x5555;
}

/* Test 3: Register move/swap pattern */
OPTIMIZE_O2
static int test3(void) {
    volatile int p = 100, q = 200;
    int temp = 0;
    
    if (p < q) {
        goto move_label;
    }
    
    return 0;
    
move_label:
    /* Candidate: register move operation */
    temp = p;  /* next_trial: move instruction */
    p = q;
    q = temp;
    
    return p + q;
}

/* Test 4: Stack-based memory operation (safe load/store) */
OPTIMIZE_O2
static int test4(void) {
    volatile int arr[4] = {1, 2, 3, 4};
    int idx = 2;
    int sum = 0;
    
    if (arr[0] > 0) {
        goto memory_label;
    }
    
    return 0;
    
memory_label:
    /* Candidate: stack memory load (shouldn't trap) */
    sum = arr[idx];  /* next_trial: load from stack */
    arr[0] = sum;
    
    return sum + arr[0];
}

/* Test 5: Comparison operation */
OPTIMIZE_O2
static int test5(void) {
    volatile int m = 50, n = 60;
    int cmp_result = 0;
    
    if (m != n) {
        goto compare_label;
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison sets condition codes */
    cmp_result = (m < n);  /* next_trial: compare instruction */
    return cmp_result ? 100 : 200;
}

/* Test 6: Multiple operations in sequence with loop */
OPTIMIZE_O2
static int test6(void) {
    volatile int counter = 0;
    int accum = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        if (i & 1) {  /* Jump on odd numbers */
            goto loop_label;
        }
        accum += i;
        continue;
        
    loop_label:
        /* Candidate: simple increment */
        counter++;  /* next_trial: increment instruction */
        accum += counter;
    }
    
    return accum;
}

/* Test 7: Shift operations */
OPTIMIZE_O2
static int test7(void) {
    volatile int val = 0x100;
    int shifted = 0;
    
    if (val > 0) {
        goto shift_label;
    }
    
    return 0;
    
shift_label:
    /* Candidate: shift operation */
    shifted = val << 2;  /* next_trial: shift instruction */
    return shifted >> 1;
}

/* Test 8: Complex pattern with multiple labels */
OPTIMIZE_O2
static int test8(void) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    int r1, r2, r3;
    
    /* Multiple jumps to increase optimization opportunities */
    if (a) goto L1;
    if (b) goto L2;
    
    return 0;
    
L1:
    r1 = a + b;
    if (r1 > 2) goto L3;
    
L2:
    r2 = b * c;
    goto L4;
    
L3:
    /* Candidate: subtraction */
    r3 = d - a;  /* next_trial: subtract instruction */
    return r3;
    
L4:
    return r2;
}

/* Test 9: Avoid resource conflicts by using fresh variables */
OPTIMIZE_O2
static int test9(void) {
    /* Use completely separate variables for jump condition
       and delay slot candidate to avoid resource conflicts */
    volatile int jump_cond_var = 10;
    int delay_slot_var1, delay_slot_var2;
    
    if (jump_cond_var > 5) {
        goto no_conflict_label;
    }
    
    return 0;
    
no_conflict_label:
    /* These variables not used in jump condition - no resource conflict */
    delay_slot_var1 = 20;
    delay_slot_var2 = delay_slot_var1 + 5;  /* next_trial: add with fresh regs */
    
    return delay_slot_var2;
}

/* Test 10: Nested control flow */
OPTIMIZE_O2
static int test10(void) {
    volatile int x = 0;
    int y = 0;
    
    while (x < 5) {
        if (x == 2) {
            goto nested_label;
        }
        x++;
    }
    
    return y;
    
nested_label:
    y = x * 3;  /* next_trial: multiplication */
    return y + 1;
}

/* Main function to execute all tests and ensure code runs */
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
    total += test9();
    total += test10();
    
    printf("Total result: %d\n", total);
    printf("(Non-zero indicates all code paths executed)\n");
    
    return 0;
}
