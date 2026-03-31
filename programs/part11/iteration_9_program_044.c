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

/* Distinct volatile variables for resource separation */
volatile int var_a = 10;
volatile int var_b = 20;
volatile int var_c = 30;
volatile int var_d = 40;
volatile int var_e = 50;
volatile int var_f = 60;
volatile int var_g = 70;
volatile int var_h = 80;

/* Global accumulator for results */
volatile int global_result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local1 = var_a;
    volatile int local2 = var_b;
    
    /* Call external function before jump */
    int res1 = ext_func1(local1);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    local2 = var_c + var_d;
    res1 = ext_func2(local2);
    
target_label1:
    /* Non-jump, non-sequence instruction at target */
    /* Simple arithmetic that doesn't conflict with parent instruction resources */
    volatile int result = var_e + var_f;  /* Uses different variables than parent */
    
    /* Call external function after label */
    int res2 = ext_func3(result);
    
    global_result += result + res1 + res2;
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int x = var_a;
    volatile int y = var_b;
    
    /* Loop with nested conditional jump */
    for (int i = 0; i < 3; i++) {
        x = ext_func1(x);
        
        if (cond2) {
            /* Different computation in parent */
            y = var_c * var_d;
            goto target_label2;
        }
        
        y = ext_func2(y);
    }
    
    /* Some more code */
    x = x + var_e;
    
target_label2:
    /* Safe operation at target - no division, no volatile zero */
    volatile int calc = var_f - var_g;  /* Subtraction is safe */
    
    global_result += calc + x + y;
}

__attribute__((optimize("O2"), noinline))
void test_function3(void) {
    volatile int a = var_a;
    volatile int b = var_b;
    
    /* Switch statement with jumps to labels */
    switch (a % 3) {
        case 0:
            a = ext_func1(a);
            if (cond3) {
                b = var_c + var_d;
                goto target_label3;
            }
            break;
        case 1:
            a = ext_func2(a);
            break;
        default:
            a = ext_func3(a);
            if (cond4) {
                goto target_label3;
            }
    }
    
    b = b * 2;
    
target_label3:
    /* Simple assignment with non-overlapping resources */
    volatile int temp = var_h;  /* Just loading a value */
    
    global_result += a + b + temp;
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function4(void) {
    volatile int p = var_a;
    volatile int q = var_b;
    
    /* Create label array for computed goto */
    static void* labels[] = { &&label1, &&label2, &&target_label4 };
    
    /* Call external function creating resource barrier */
    p = ext_func4(p);
    
    /* Conditional jump using computed goto */
    if (cond5) {
        goto *labels[2];  /* Jump directly to target label */
    }
    
label1:
    q = ext_func1(q);
    goto end;
    
label2:
    q = ext_func2(q);
    goto end;
    
target_label4:
    /* Safe memory operation at target */
    volatile int val = var_g;
    volatile int sum = val + 100;  /* Simple addition */
    
    global_result += sum;
    return;
    
end:
    global_result += p + q;
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_function5_mips_like(void) {
    volatile int reg1 = var_a;
    volatile int reg2 = var_b;
    volatile int reg3 = var_c;
    
    /* Multiple external calls to separate resources */
    reg1 = ext_func1(reg1);
    reg2 = ext_func2(reg2);
    
    /* Complex condition to force conditional jump generation */
    if ((reg1 > 0) && (reg2 < 100) && cond1) {
        /* Inline assembly with memory clobber */
        __asm__ volatile ("# Resource barrier" : : : "memory");
        
        /* Jump to label */
        goto mips_target_label;
    }
    
    /* Alternative path */
    reg3 = ext_func3(reg3);
    reg1 = reg1 * reg3;
    
mips_target_label:
    /* Very simple instruction at target - ideal delay slot candidate */
    /* Uses completely different register/var set than parent */
    volatile int delay_slot_candidate = var_f;  /* Just a load */
    
    /* Ensure no resource conflict with parent instruction */
    global_result += delay_slot_candidate;
}

/* Function with multiple potential delay slot candidates */
__attribute__((optimize("O3"), noinline))
void test_function6_multiple_patterns(void) {
    volatile int x1 = var_a, x2 = var_b, x3 = var_c;
    volatile int y1 = var_d, y2 = var_e, y3 = var_f;
    
    /* Pattern 1 */
    x1 = ext_func1(x1);
    if (x1 > 50) {
        y1 = x2 + x3;
        goto pattern1_target;
    }
    
    y1 = ext_func2(y1);
    
pattern1_target:
    volatile int r1 = y2 - y3;  /* Safe subtraction */
    global_result += r1;
    
    /* Pattern 2 - inside loop */
    for (int i = 0; i < 2; i++) {
        x2 = ext_func3(x2);
        if (cond2) {
            y2 = x1 * x3;
            goto pattern2_target;
        }
    }
    
    y2 = ext_func4(y2);
    
pattern2_target:
    volatile int r2 = y1;  /* Simple load */
    global_result += r2;
    
    /* Pattern 3 - nested condition */
    if (cond3) {
        x3 = ext_func1(x3);
        if (cond4) {
            y3 = x1 + x2;
            goto pattern3_target;
        }
    }
    
    y3 = ext_func2(y3);
    
pattern3_target:
    volatile int r3 = 42;  /* Constant assignment - very safe */
    global_result += r3;
}

/* Main function that runs all tests */
int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile variables with non-zero values */
    var_a = 100;
    var_b = 200;
    var_c = 300;
    var_d = 400;
    var_e = 500;
    var_f = 600;
    var_g = 700;
    var_h = 800;
    
    /* Set conditions to ensure some jumps are taken */
    cond1 = 1;  /* Will take jump in test_function1 */
    cond2 = 0;  /* Will NOT take jump in test_function2 */
    cond3 = 1;  /* Will take jump in test_function3 */
    cond4 = 1;  /* Will take jump in test_function3 default case */
    cond5 = 1;  /* Will take computed goto in test_function4 */
    
    /* Run all test functions */
    test_function1();
    printf("After test1: global_result = %d\n", global_result);
    
    test_function2();
    printf("After test2: global_result = %d\n", global_result);
    
    test_function3();
    printf("After test3: global_result = %d\n", global_result);
    
    test_function4();
    printf("After test4: global_result = %d\n", global_result);
    
    test_function5_mips_like();
    printf("After test5: global_result = %d\n", global_result);
    
    test_function6_multiple_patterns();
    printf("After test6: global_result = %d\n", global_result);
    
    printf("Final result: %d\n", global_result);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 3; }
int ext_func4(int x) { return x / 2; }
