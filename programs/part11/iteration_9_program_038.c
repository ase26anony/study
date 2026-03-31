/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

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

/* Distinct volatile variables for different operations */
volatile int var_a = 10;
volatile int var_b = 20;
volatile int var_c = 30;
volatile int var_d = 40;
volatile int var_e = 50;
volatile int var_f = 60;
volatile int var_g = 70;
volatile int var_h = 80;

/* Result accumulator */
volatile int total_result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local1 = var_a;
    volatile int local2 = var_b;
    volatile int result = 0;
    
    /* Call external function to create resource barrier */
    int ext_result = ext_func1(local1);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    local2 = ext_result + 5;
    
target_label1:
    /* Simple arithmetic operation - candidate for delay slot */
    result = var_c + var_d;
    
    /* Another external call */
    ext_result = ext_func2(result);
    
    total_result += result + ext_result;
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int x = var_e;
    volatile int y = var_f;
    volatile int temp;
    
    /* Loop with nested conditional */
    for (int i = 0; i < 3; i++) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        
        if (cond2) {
            goto target_label2;
        }
        
        x = ext_func3(x);
        
        /* Switch statement for control flow complexity */
        switch (i) {
            case 0:
                temp = x + 1;
                break;
            case 1:
                temp = x * 2;
                break;
            default:
                temp = x - 1;
        }
        
        if (cond3 && i == 1) {
            goto target_label2;
        }
        
        continue;
        
    target_label2:
        /* Safe non-trapping operation */
        y = var_g - var_h;
        
        /* Another memory barrier */
        __asm__ volatile ("" : : : "memory");
        
        total_result += y;
        
        if (i == 2) break;
    }
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int val1 = 100;
    volatile int val2 = 200;
    
    /* Multiple external calls to separate resources */
    val1 = ext_func1(val1);
    val2 = ext_func2(val2);
    
    /* Computed goto */
    goto *labels[cond4 ? 0 : 1];
    
label_a:
    /* This should not be reached in normal flow */
    val1 = val1 * 2;
    goto end;
    
label_b:
    /* Simple assignment - delay slot candidate */
    val2 = var_a + var_b;
    
    /* External call after label */
    ext_func3(val2);
    goto end;
    
label_c:
    val1 = val1 / 2;  /* Avoid division by volatile zero */
    goto end;
    
end:
    total_result += val1 + val2;
}

/* Function with multiple jump targets */
__attribute__((optimize("O2"), noinline))
void test_function4(int mode) {
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    
    /* Complex condition to force jump generation */
    if (cond5 || (mode > 0 && a < b)) {
        if (a + b > c) {
            goto target_a;
        } else {
            goto target_b;
        }
    }
    
    /* Intermediate computation */
    a = ext_func4(a);
    
    return;
    
target_a:
    /* Simple arithmetic with distinct variables */
    b = var_c * var_d;
    
    /* Ensure no resource conflict with parent instruction */
    __asm__ volatile ("" : : : "memory");
    
    total_result += b;
    return;
    
target_b:
    /* Another simple operation */
    c = var_e - var_f;
    total_result += c;
    return;
}

/* Function with nested loops and labels */
__attribute__((optimize("O3")))
void test_function5(void) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            /* Varying conditions */
            if ((i + j) % 2 == 0) {
                if (cond1) {
                    goto inner_label;
                }
            }
            
            counter = ext_func1(counter);
            continue;
            
        inner_label:
            /* Safe operation - addition only */
            sum += var_g + var_h;
            
            /* External call to separate resources */
            ext_func2(sum);
            
            if (j == 2) break;
        }
        
        /* Another conditional jump */
        if (i == 3 && cond3) {
            goto outer_label;
        }
    }
    
    return;
    
outer_label:
    /* Final simple operation */
    sum = sum * 2;
    total_result += sum;
}

/* Main test driver */
int main(void) {
    printf("Starting delay slot filling tests...\n");
    
    /* Initialize volatile variables with non-zero values */
    var_a = 100;
    var_b = 200;
    var_c = 300;
    var_d = 400;
    var_e = 500;
    var_f = 600;
    var_g = 700;
    var_h = 800;
    
    /* Run test functions multiple times with different conditions */
    for (int i = 0; i < 2; i++) {
        cond1 = i % 2;
        cond2 = (i + 1) % 2;
        cond3 = i % 3 != 0;
        cond4 = i < 1;
        cond5 = i > 0;
        
        test_function1();
        test_function2();
        test_function3();
        test_function4(i);
        test_function5();
    }
    
    printf("Total result: %d\n", total_result);
    printf("Tests completed.\n");
    
    return total_result != 0 ? 0 : 1;
}

/* Dummy external function implementations */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
int ext_func4(int x) { return x / 2; }
