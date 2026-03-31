/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int x = 0, y = 0, z = 0;
    volatile int local_cond = cond1;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* Parent instruction for delay slot */
    
    /* Call external function to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (local_cond) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = c * 2;
    ext_func2(y);
    
    /* This should not be executed if jump is taken */
    z = d / 2;
    
target_label1:
    /* Candidate for delay slot filling - simple arithmetic */
    /* Uses different variables than parent instruction */
    int temp = c + d;  /* Non-trapping, non-jump instruction */
    result += temp;
    
    /* More operations after label */
    ext_func3(temp);
    x = temp * 2;
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int p = 0, q = 0, r = 0;
    volatile int local_cond = cond2;
    
    /* Different parent instruction */
    p = a * b;
    ext_func1(p);
    
    /* Jump inside a loop */
    for (int i = 0; i < 3; i++) {
        if (local_cond) {
            goto target_label2;
        }
        q = p + i;
        ext_func2(q);
    }
    
    r = c - d;
    
target_label2:
    /* Another candidate - simple assignment */
    int temp = a - b;  /* Safe, non-trapping operation */
    result += temp;
    
    ext_func3(temp);
}

/* Function with switch statement */
__attribute__((optimize("O2"), noinline))
void test_pattern3(int mode) {
    volatile int u = 0, v = 0, w = 0;
    volatile int local_cond = cond3;
    
    /* Parent instruction with memory operation */
    u = a;
    v = b;
    ext_func1(u + v);
    
    switch (mode) {
        case 1:
            if (local_cond) {
                goto target_label3;
            }
            w = u * 2;
            break;
        case 2:
            w = v * 3;
            break;
        default:
            w = 0;
    }
    
    ext_func2(w);
    
target_label3:
    /* Candidate - bitwise operation */
    int temp = c & 0xFF;  /* Non-trapping */
    result += temp;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    ext_func3(temp);
}

/* Function using computed goto */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int m = 0, n = 0;
    volatile int local_cond = cond4;
    
    static void *labels[] = { &&label1, &&label2, &&target_label4 };
    
    /* Parent instruction */
    m = d - a;
    ext_func1(m);
    
    /* Computed goto */
    if (local_cond) {
        goto *labels[2];
    }
    
    n = m + 10;
    
label1:
    n += 5;
    goto end;
    
label2:
    n -= 5;
    goto end;
    
target_label4:
    /* Candidate - shift operation */
    int temp = b << 2;  /* Safe operation */
    result += temp;
    
    ext_func2(temp);
    
end:
    ext_func3(n);
}

/* Nested control flow pattern */
__attribute__((optimize("O3"), noinline))
void test_pattern5(void) {
    volatile int i, j, k;
    volatile int local_cond = cond1;
    
    /* Complex parent instruction sequence */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            k = a + b + i + j;
            ext_func1(k);
            
            if (local_cond && (i == 1)) {
                goto target_label5;
            }
            
            k = k * 2;
            ext_func2(k);
        }
    }
    
    k = c + d;
    
target_label5:
    /* Candidate - simple increment */
    int temp = a++;  /* Post-increment is safe */
    result += temp;
    
    ext_func3(temp);
}

/* Function that might create SEQUENCE pattern to avoid */
__attribute__((optimize("O2")))
void test_avoid_sequence(void) {
    volatile int x = cond1;
    
    /* Parent instruction */
    int parent = a * b;
    
    if (x) {
        goto regular_label;  /* This should work */
        /* Don't put inline asm here to avoid SEQUENCE */
    }
    
    parent += 10;
    
regular_label:
    /* Safe candidate instruction */
    int temp = c + d;
    result += temp;
}

/* External function implementations (simulated) */
int ext_func1(int x) {
    /* Memory clobber to affect resource analysis */
    __asm__ volatile ("" : : : "memory");
    return x + 1;
}

int ext_func2(int x) {
    __asm__ volatile ("" : : : "memory");
    return x * 2;
}

int ext_func3(int x) {
    __asm__ volatile ("" : : : "memory");
    return x - 1;
}

/* Main function to execute all patterns */
int main(void) {
    printf("Starting reorg delay slot test...\n");
    
    /* Initialize volatile variables with non-zero values */
    cond1 = 1;
    cond2 = 0;  /* Will take different path */
    cond3 = 1;
    cond4 = 0;
    a = 5; b = 10; c = 15; d = 20;
    result = 0;
    
    /* Execute all test patterns */
    test_pattern1();
    printf("Pattern 1 executed, result = %d\n", result);
    
    test_pattern2();
    printf("Pattern 2 executed, result = %d\n", result);
    
    test_pattern3(1);
    printf("Pattern 3 executed, result = %d\n", result);
    
    test_pattern4();
    printf("Pattern 4 executed, result = %d\n", result);
    
    test_pattern5();
    printf("Pattern 5 executed, result = %d\n", result);
    
    test_avoid_sequence();
    printf("Avoid sequence executed, result = %d\n", result);
    
    /* Test with different conditions */
    cond1 = 0;
    cond2 = 1;
    cond3 = 0;
    cond4 = 1;
    
    result = 0;
    test_pattern1();
    test_pattern2();
    test_pattern3(2);
    test_pattern4();
    test_pattern5();
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
