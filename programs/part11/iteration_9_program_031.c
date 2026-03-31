/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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
volatile int result = 0;
volatile int x = 5, y = 10, z = 15;
volatile int a = 20, b = 25, c = 30;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local_cond = cond1;
    volatile int temp1 = x;
    volatile int temp2 = y;
    
    /* Call external function before jump */
    int ext_result = ext_func1(temp1);
    
    /* Conditional jump to label */
    if (local_cond) {
        /* Inline assembly to create resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    temp1 = temp1 + ext_result;
    
target_label1:
    /* Simple non-jump, non-trapping instruction at target */
    /* Using distinct variables to avoid resource conflicts */
    volatile int r1 = a;
    volatile int r2 = b;
    r1 = r1 + r2;  /* Simple arithmetic - delay slot candidate */
    
    /* Call external function after target */
    ext_func2(r1);
    
    result += r1;
}

__attribute__((optimize("O2")))
void test_function2(void) {
    volatile int local_cond = cond2;
    volatile int temp3 = z;
    volatile int temp4 = c;
    
    /* Different pattern with switch statement */
    switch (local_cond) {
        case 0:
            ext_func1(temp3);
            if (cond3) {
                __asm__ volatile ("" : : : "memory");
                goto target_label2;
            }
            break;
        default:
            temp3 = temp3 * 2;
            break;
    }
    
    /* Some loop to create more complex CFG */
    for (int i = 0; i < 3; i++) {
        if (i == 1 && cond4) {
            __asm__ volatile ("" : : : "memory");
            goto target_label2;
        }
        temp4 += i;
    }
    
target_label2:
    /* Another simple instruction at target */
    volatile int r3 = x;
    volatile int r4 = y;
    r3 = r3 - r4;  /* Subtraction - safe, non-trapping */
    
    ext_func3(r3);
    result += r3;
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    static void* labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int idx = cond1 ? 0 : 1;
    
    /* Use distinct variables for parent instruction */
    volatile int p1 = a;
    volatile int p2 = b;
    int parent_result = p1 * p2;  /* Parent instruction computation */
    
    /* Resource separation with external call */
    ext_func2(parent_result);
    
    /* Jump via computed goto */
    goto *labels[idx];
    
label_a:
    /* Intermediate block */
    p1 += 5;
    goto label_target;
    
label_b:
    p1 -= 3;
    goto label_target;
    
label_c:
    p1 *= 2;
    /* Fall through */
    
label_target:
    /* Target instruction with distinct variables */
    volatile int t1 = c;
    volatile int t2 = z;
    t1 = t2 + 7;  /* Simple assignment - delay slot candidate */
    
    result += t1 + parent_result;
}

/* Function with nested control flow */
__attribute__((optimize("O3")))
void test_function4(void) {
    volatile int local_x = x;
    volatile int local_y = y;
    
    /* Complex control flow with multiple labels */
    if (cond1) {
        ext_func1(local_x);
        
        for (int i = 0; i < 2; i++) {
            if (cond2) {
                __asm__ volatile ("" : : : "memory");
                goto deep_label;
            }
            local_x += i;
        }
        
        if (cond3) {
            switch (local_y) {
                case 10:
                    __asm__ volatile ("" : : : "memory");
                    goto deep_label;
                default:
                    break;
            }
        }
    }
    
    local_y = local_x * 2;
    return;
    
deep_label:
    /* Very simple target instruction */
    volatile int simple = 42;
    simple = simple + 1;  /* Extremely simple arithmetic */
    
    result += simple;
}

/* Function specifically for MIPS-like delay slot patterns */
__attribute__((optimize("O2")))
void test_mips_pattern(void) {
    volatile int m1 = a;
    volatile int m2 = b;
    volatile int m3 = c;
    
    /* Pattern resembling MIPS conditional branch */
    if (m1 > m2) {
        /* External call for resource separation */
        ext_func3(m1);
        
        /* Conditional jump to immediate label */
        if (cond1) {
            __asm__ volatile ("" : : : "memory");
            goto mips_target;
        }
        
        m2 = m1 + m2;
    }
    
    m3 = m2 * 2;
    return;
    
mips_target:
    /* Safe, non-conflicting operation */
    volatile int r = m3;
    r = r & 0xFF;  /* Bitwise AND - safe operation */
    
    result += r;
}

/* Function with multiple jump-to-label patterns */
__attribute__((optimize("O2")))
void test_multiple_patterns(void) {
    volatile int v1 = 100;
    volatile int v2 = 200;
    volatile int v3 = 300;
    
    /* Pattern 1 */
    if (v1 < v2) {
        ext_func1(v1);
        if (cond1) goto pat1_target;
    }
    
    v2 = v1 + 50;
    
pat1_target:
    volatile int res1 = v3;
    res1 = res1 / 2;  /* Division but with constant 2 - safe */
    result += res1;
    
    /* Pattern 2 */
    if (v2 > 150) {
        ext_func2(v2);
        if (cond2) goto pat2_target;
    }
    
    v3 = v2 - 75;
    
pat2_target:
    volatile int res2 = v1;
    res2 = res2 | 0x0F;  /* Bitwise OR - safe */
    result += res2;
    
    /* Pattern 3 */
    while (v1 < 105) {
        if (v1 == 103 && cond3) {
            ext_func3(v1);
            __asm__ volatile ("" : : : "memory");
            goto pat3_target;
        }
        v1++;
    }
    
pat3_target:
    volatile int res3 = v2;
    res3 = res3 ^ 0xAA;  /* Bitwise XOR - safe */
    result += res3;
}

/* Dummy external functions to satisfy declarations */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 3; }

int main(void) {
    printf("Starting reorg test patterns...\n");
    
    /* Initialize volatile control variables */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    x = 5; y = 10; z = 15;
    a = 20; b = 25; c = 30;
    
    /* Execute all test functions */
    test_function1();
    test_function2();
    test_function3();
    test_function4();
    test_mips_pattern();
    test_multiple_patterns();
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
