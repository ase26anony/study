/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern int ext_func4(int);

/* Volatile control variables */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int cond5 = 1;

/* Distinct volatile variables for resource separation */
volatile int var_a = 10;
volatile int var_b = 20;
volatile int var_c = 30;
volatile int var_d = 40;
volatile int var_e = 50;
volatile int var_f = 60;
volatile int var_g = 70;
volatile int var_h = 80;
volatile int var_i = 90;
volatile int var_j = 100;

volatile int result = 0;

/* Test function 1: Basic jump-to-label pattern */
__attribute__((optimize("O2")))
void test_func1(void) {
    volatile int local1 = var_a;
    volatile int local2 = var_b;
    
    /* Create resource usage before jump */
    int temp1 = ext_func1(local1);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    temp1 = ext_func2(local2);
    
target_label1:
    /* Non-jump, non-sequence instruction at target */
    /* Simple arithmetic that doesn't conflict with parent instruction's resources */
    volatile int temp2 = var_c + var_d;  /* Uses different variables than parent */
    
    /* Use result to prevent optimization */
    result += temp2;
}

/* Test function 2: Jump inside loop */
__attribute__((optimize("O2")))
void test_func2(void) {
    volatile int local3 = var_e;
    volatile int local4 = var_f;
    
    for (int i = 0; i < 3; i++) {
        /* Resource usage in loop */
        int temp3 = ext_func3(local3 + i);
        
        /* Multiple conditional jumps */
        if (cond2) {
            goto target_label2;
        }
        
        if (cond3 && i > 1) {
            __asm__ volatile ("" : : : "memory");
            goto target_label2;
        }
        
        continue;
        
    target_label2:
        /* Safe arithmetic operation at target */
        volatile int temp4 = var_g - var_h;  /* Different variables */
        result += temp4;
        
        /* Break to avoid infinite loop in coverage run */
        if (i > 0) break;
    }
}

/* Test function 3: Switch statement with jumps to labels */
__attribute__((optimize("O3")))
void test_func3(void) {
    volatile int local5 = var_i;
    volatile int local6 = var_j;
    
    int choice = cond4 ? 1 : 2;
    
    switch (choice) {
        case 1: {
            /* Resource usage in case */
            int temp5 = ext_func4(local5);
            
            /* Jump to label */
            if (cond5) {
                __asm__ volatile ("" : : : "memory");
                goto target_label3;
            }
            
            /* Fall through */
        }
        case 2: {
            /* Different computation */
            volatile int temp6 = local5 * 2;
            
        target_label3:
            /* Simple assignment at target */
            volatile int temp7 = var_a * var_b;  /* Distinct variables */
            result += temp7;
            break;
        }
        default:
            break;
    }
}

/* Test function 4: Computed goto using && labels */
__attribute__((optimize("O2")))
void test_func4(void) {
    volatile int local7 = var_c;
    volatile int local8 = var_d;
    
    /* Create label array */
    static void* labels[] = { &&label1, &&label2, &&label3 };
    
    /* Use external call for resource separation */
    int temp8 = ext_func1(local7);
    
    /* Conditional computed goto */
    if (temp8 > 0) {
        __asm__ volatile ("" : : : "memory");
        goto *labels[1];
    }
    
    /* Intermediate code */
    temp8 = ext_func2(local8);
    
label1:
    /* This might be optimized away, so add complexity */
    volatile int temp9 = 0;
    for (int j = 0; j < 2; j++) {
        temp9 += j;
    }
    return;
    
label2:
    /* Target instruction: simple arithmetic */
    volatile int temp10 = var_e / 2;  /* Division by constant is safe */
    result += temp10;
    goto label3;
    
label3:
    /* Another simple operation */
    volatile int temp11 = var_f % 3;  /* Modulo with constant is safe */
    result += temp11;
}

/* Test function 5: Nested control flow with multiple labels */
__attribute__((optimize("O2")))
void test_func5(void) {
    volatile int local9 = var_g;
    volatile int local10 = var_h;
    
    /* Complex nested conditionals */
    if (cond1) {
        if (cond2) {
            /* Resource usage */
            int temp12 = ext_func3(local9);
            goto target_label5a;
        } else {
            int temp13 = ext_func4(local10);
            if (cond3) {
                __asm__ volatile ("" : : : "memory");
                goto target_label5b;
            }
        }
    }
    
    /* Default path */
    volatile int temp14 = local9 + local10;
    result += temp14;
    return;
    
target_label5a:
    /* First target - simple assignment */
    volatile int temp15 = var_i - var_j;  /* Different variables */
    result += temp15;
    return;
    
target_label5b:
    /* Second target - safe computation */
    volatile int temp16 = var_a ^ var_b;  /* Bitwise operation, safe */
    result += temp16;
    
    /* Add loop to increase basic block complexity */
    for (int k = 0; k < 2; k++) {
        volatile int temp17 = k * 10;
        result += temp17;
    }
}

/* Test function 6: Multiple jumps to same label from different locations */
__attribute__((optimize("O3")))
void test_func6(void) {
    volatile int local11 = var_c;
    volatile int local12 = var_e;
    
    /* Jump point 1 */
    if (cond1 && local11 > 0) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    /* Some intermediate computation */
    int temp18 = ext_func1(local11);
    
    /* Jump point 2 */
    if (cond4 || temp18 < 100) {
        __asm__ volatile ("" : : : "memory");
        goto common_target;
    }
    
    /* Jump point 3 - inside loop */
    for (int m = 0; m < 3; m++) {
        if (m == 1 && cond5) {
            __asm__ volatile ("" : : : "memory");
            goto common_target;
        }
    }
    
    /* Fall-through path */
    volatile int temp19 = local12 * 3;
    result += temp19;
    return;
    
common_target:
    /* Common target instruction - safe arithmetic */
    volatile int temp20 = var_g + var_h + 5;  /* Simple addition */
    result += temp20;
}

/* Dummy external functions to satisfy declarations */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
int ext_func4(int x) { return x / 2; }

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions with non-deterministic values */
    cond1 = rand() % 2;
    cond2 = rand() % 2;
    cond3 = rand() % 2;
    cond4 = rand() % 2;
    cond5 = rand() % 2;
    
    /* Call test functions multiple times with different conditions */
    for (int run = 0; run < 2; run++) {
        test_func1();
        test_func2();
        test_func3();
        test_func4();
        test_func5();
        test_func6();
        
        /* Modify conditions for next run */
        cond1 ^= 1;
        cond2 ^= 1;
        cond3 ^= 1;
    }
    
    printf("Result accumulated: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}
