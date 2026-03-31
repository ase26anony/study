/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2"), noinline))
static int test_pattern1(void) {
    volatile int a = 10, b = 20, c = 30;
    volatile int x = 0, y = 0, z = 0;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* This could be the parent instruction needing delay slot */
    
    /* Call external function to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Use inline assembly to prevent optimization */
        asm volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = b + c;
    ext_func2(y);
    
    /* This should not be reached when cond1 is true */
    return -1;
    
target_label1:
    /* Non-jump, non-sequence instruction - delay slot candidate */
    /* Use different variables to avoid resource conflicts */
    z = c + 5;  /* Simple arithmetic, no traps */
    
    /* Another external call */
    ext_func3(z);
    
    return z;
}

/* More complex pattern with switch statement */
__attribute__((optimize("O2"), noinline))
static int test_pattern2(void) {
    volatile int i = 2, j = 3, k = 4;
    volatile int sum = 0;
    
    /* Parent instruction */
    sum = i * j;
    
    /* Switch with multiple cases */
    switch (i) {
        case 1:
            ext_func1(sum);
            break;
        case 2:
            /* Conditional jump inside switch case */
            if (cond2) {
                asm volatile ("" : : : "memory");
                goto target_label2;
            }
            sum += 5;
            break;
        default:
            sum = 0;
    }
    
    /* More code that shouldn't be reached */
    k = k * 2;
    return -1;
    
target_label2:
    /* Safe instruction - addition with non-trapping operands */
    j = k + 10;
    
    return j;
}

/* Pattern with loop */
__attribute__((optimize("O3"), noinline))
static int test_pattern3(void) {
    volatile int counter = 3;
    volatile int acc = 0;
    volatile int temp;
    
    while (counter > 0) {
        /* Parent instruction in loop */
        temp = acc * 2;
        
        /* Conditional jump from inside loop */
        if (cond3 && counter == 2) {
            asm volatile ("" : : : "memory");
            goto target_label3;
        }
        
        acc += counter;
        counter--;
    }
    
    return acc;
    
target_label3:
    /* Simple assignment - delay slot candidate */
    temp = 100;
    
    /* Continue loop */
    acc += temp;
    counter--;
    
    /* Finish loop */
    while (counter > 0) {
        acc += counter;
        counter--;
    }
    
    return acc;
}

/* Pattern using computed goto (labels as values) */
__attribute__((optimize("O2"), noinline))
static int test_pattern4(void) {
    volatile int val = 42;
    volatile int res = 0;
    
    /* Define labels */
    void* labels[] = { &&label_a, &&label_b, &&target_label4 };
    
    /* Parent instruction */
    res = val / 2;  /* Division but with constant 2 - no trap */
    
    /* External call for resource separation */
    ext_func2(res);
    
    /* Computed goto */
    if (cond4) {
        goto *labels[2];  /* Jump to target_label4 */
    }
    
    /* Alternative paths */
    goto *labels[0];
    
label_a:
    res += 1;
    return res;
    
label_b:
    res += 2;
    return res;
    
target_label4:
    /* Safe memory operation */
    volatile int safe_var = 7;
    res = safe_var * 3;  /* Multiplication - no traps */
    
    return res;
}

/* Function with multiple nested conditionals */
__attribute__((optimize("O3"), noinline))
static int test_pattern5(void) {
    volatile int a = 5, b = 6, c = 7;
    volatile int r1 = 0, r2 = 0;
    
    /* Complex parent instruction sequence */
    r1 = (a << 2) | (b & 0xF);
    
    /* Nested conditionals */
    if (cond1) {
        if (cond2) {
            ext_func1(r1);
        } else {
            /* Jump to label from nested else */
            if (cond3) {
                asm volatile ("" : : : "memory");
                goto target_label5;
            }
            r1 += 10;
        }
    } else {
        r1 = ext_func2(r1);
    }
    
    /* Unreachable when conditions are right */
    r2 = c * 100;
    return r2;
    
target_label5:
    /* Very simple instruction - ideal delay slot */
    r2 = a + b;  /* Uses different vars than parent instruction */
    
    return r2;
}

/* Main function that runs all patterns */
int main(void) {
    int r;
    
    printf("Testing delay slot patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;  /* True */
    cond2 = 0;  /* False */
    cond3 = 1;  /* True */
    cond4 = 1;  /* True */
    
    /* Run all test patterns */
    r = test_pattern1();
    result += r;
    printf("Pattern1 result: %d\n", r);
    
    r = test_pattern2();
    result += r;
    printf("Pattern2 result: %d\n", r);
    
    r = test_pattern3();
    result += r;
    printf("Pattern3 result: %d\n", r);
    
    r = test_pattern4();
    result += r;
    printf("Pattern4 result: %d\n", r);
    
    r = test_pattern5();
    result += r;
    printf("Pattern5 result: %d\n", r);
    
    printf("Total result: %d\n", result);
    
    return 0;
}

/* Dummy external functions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
