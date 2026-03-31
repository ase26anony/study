/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern int ext_func4(int);

/* Volatile control variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int cond5 = 1;

/* Distinct volatile variables for resource separation */
volatile int var_a = 1, var_b = 2, var_c = 3;
volatile int var_x = 10, var_y = 20, var_z = 30;
volatile int var_p = 100, var_q = 200, var_r = 300;
volatile int result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    int local1, local2, local3;
    
    /* Parent instruction computation - uses var_a, var_b */
    local1 = var_a + var_b;
    
    /* External call creates resource barrier */
    local1 = ext_func1(local1);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    local2 = var_c * 2;
    local2 = ext_func2(local2);
    
    /* Avoid falling through to target */
    if (cond2) {
        local3 = var_x - var_y;
    }
    
    return;  /* Early return to make jump necessary */
    
target_label1:
    /* Delay slot candidate: simple arithmetic with non-overlapping resources */
    /* Uses var_x, var_y which don't overlap with var_a, var_b used above */
    local2 = var_x + var_y;
    
    /* External call after target */
    result += ext_func3(local2);
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int local4, local5;
    
    /* Loop with nested control flow */
    for (int i = 0; i < 3; i++) {
        /* Parent instruction */
        local4 = var_p * var_q;
        local4 = ext_func1(local4);
        
        /* Switch statement inside loop */
        switch (i) {
            case 0:
                if (cond3) {
                    /* Jump to label */
                    goto target_label2;
                }
                local5 = var_r / 2;
                break;
            case 1:
                local5 = var_a + var_b;
                break;
            default:
                /* Another jump pattern */
                if (cond4) {
                    goto target_label3;
                }
                break;
        }
        
        /* Continue loop body */
        local5 = ext_func2(local5);
        continue;
        
    target_label2:
        /* Simple assignment - safe non-trapping operation */
        local5 = var_z;
        result += local5;
        break;  /* Exit loop */
        
    target_label3:
        /* Another simple operation */
        local5 = var_x - var_y;
        result += local5;
        continue;
    }
}

/* Function with computed goto */
__attribute__((optimize("O2"), __noinline__))
void test_pattern3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int selector = 1;
    
    /* Parent instruction with resource usage */
    int res1 = var_a * var_b + var_c;
    res1 = ext_func3(res1);
    
    /* Computed goto */
    goto *labels[selector];
    
    /* Unreachable code */
    res1 = var_x + var_y;
    
label_a:
    /* Simple arithmetic - delay slot candidate */
    res1 = var_p + var_q;
    result += res1;
    return;
    
label_b:
    /* Another simple operation */
    res1 = var_x * var_y;
    result += res1;
    return;
    
label_c:
    /* Safe division with non-zero divisor */
    if (var_y != 0) {
        res1 = var_z / var_y;
    } else {
        res1 = 1;
    }
    result += res1;
    return;
}

/* Function with multiple nested jumps */
__attribute__((optimize("O2"), __noinline__))
void test_pattern4(void) {
    volatile int counter = 0;
    
    while (counter++ < 2) {
        /* Parent instruction */
        int temp = var_a + var_b;
        temp = ext_func4(temp);
        
        /* Nested if with goto */
        if (cond5) {
            if (var_x > var_y) {
                __asm__ volatile ("" : : : "memory");
                goto target_label4;
            } else {
                temp = var_z;
            }
        }
        
        /* More code */
        temp = ext_func1(temp);
        continue;
        
    target_label4:
        /* Simple safe operation - addition */
        temp = var_p + var_q;
        result += temp;
        
        /* External call to prevent optimization */
        ext_func2(temp);
        break;
    }
}

/* Function that mimics delay slot patterns */
__attribute__((optimize("O3"), __noinline__))
void test_pattern5(void) {
    volatile int a = var_a, b = var_b, c = var_c;
    
    /* Multiple basic blocks */
    if (a > 0) {
        /* Parent instruction */
        int res = b * c;
        res = ext_func3(res);
        
        /* Jump to label */
        if (cond1 && cond3) {
            __asm__ volatile ("" : : : "memory");
            goto compute_point;
        }
        
        /* Alternative path */
        res = ext_func1(res);
        result += res;
        return;
    }
    
    /* Another block */
    if (b > 0) {
        int res = a + c;
        res = ext_func2(res);
        result += res;
        return;
    }
    
compute_point:
    /* Delay slot candidate: simple assignment */
    /* Uses different variables than parent instruction */
    int res = var_x + var_y;
    result += res;
    
    /* Prevent tail call optimization */
    __asm__ volatile ("" : : : "memory");
}

/* Dummy external functions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
int ext_func4(int x) { return x / 2; }

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile variables with non-zero values */
    cond1 = 1; cond2 = 0; cond3 = 1; cond4 = 0; cond5 = 1;
    var_a = 5; var_b = 7; var_c = 11;
    var_x = 13; var_y = 17; var_z = 19;
    var_p = 23; var_q = 29; var_r = 31;
    result = 0;
    
    /* Call test patterns multiple times */
    for (int i = 0; i < 2; i++) {
        test_pattern1();
        test_pattern2();
        test_pattern3();
        test_pattern4();
        test_pattern5();
        
        /* Modify conditions to explore different paths */
        cond1 = !cond1;
        cond3 = !cond3;
    }
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}
