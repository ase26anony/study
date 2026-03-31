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

/* Global result accumulator */
volatile int global_result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_function1(void) {
    int local1 = var_a;
    int local2 = var_b;
    
    /* Call external function before jump */
    local1 = ext_func1(local1);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    local2 = var_c + var_d;
    
target_label1:
    /* Simple non-jump, non-trapping instruction at target */
    /* Uses different variables than those used before jump */
    int result = var_e + var_f;
    global_result += result;
    
    /* Call external function after label */
    ext_func2(result);
}

/* Function with different optimization level */
__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int local_var1 = var_a;
    volatile int local_var2 = var_b;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    /* Multiple conditional jumps in loop */
    for (int i = 0; i < 3; i++) {
        if (cond2) {
            goto target_label2;
        }
        
        /* Some computation with different resources */
        local_var1 = var_c * 2;
        ext_func3(local_var1);
        
        if (cond3 && i == 1) {
            goto target_label2;
        }
    }
    
    /* Fall-through path */
    local_var2 = var_d / 2;
    goto after_target;
    
target_label2:
    /* Safe arithmetic operation - no division by volatile zero */
    int safe_result = var_g - var_h;
    global_result += safe_result;
    
after_target:
    /* More operations */
    ext_func4(safe_result);
}

/* Function with switch statement and computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    static void* labels[] = { &&case0, &&case1, &&case2, &&default_case };
    
    volatile int selector = cond4 ? 1 : 2;
    
    /* Resource separation: use different variables before jump */
    int pre_jump_var = var_a * var_b;
    ext_func1(pre_jump_var);
    
    /* Computed goto */
    goto *labels[selector];
    
case0:
    /* This should not be executed with current selector values */
    var_c = 100;
    return;
    
case1:
    /* Target label with simple non-jump instruction */
    {
        int temp = var_e;
        var_f = temp + 5;
        global_result += var_f;
    }
    return;
    
case2:
    /* Another target with safe operation */
    var_g = var_h - 10;
    global_result += var_g;
    return;
    
default_case:
    /* Default case with external call */
    ext_func2(var_d);
    return;
}

/* Function with nested control flow */
__attribute__((noinline))
void test_function4(int depth) {
    volatile int local1 = var_a;
    volatile int local2 = var_b;
    
    if (depth > 2) {
        return;
    }
    
    /* Multiple external calls to create resource barriers */
    local1 = ext_func3(local1);
    
    /* Conditional jump inside nested if */
    if (cond5) {
        if (depth % 2 == 0) {
            goto nested_target;
        }
    }
    
    /* Alternative path */
    local2 = ext_func4(local2);
    test_function4(depth + 1);
    return;
    
nested_target:
    /* Simple assignment at target - no complex operations */
    int simple_calc = var_c;
    var_d = simple_calc;
    global_result += simple_calc;
    
    /* Recursive call */
    test_function4(depth + 1);
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2")))
void test_mips_pattern(void) {
    volatile int reg1 = var_a;
    volatile int reg2 = var_b;
    volatile int reg3 = var_c;
    
    /* Simulate MIPS-like register usage pattern */
    __asm__ volatile ("# MIPS-like pattern start" : : : "memory");
    
    /* Multiple conditional jumps */
    if (reg1 > 0) {
        ext_func1(reg1);
        goto mips_target1;
    }
    
    if (reg2 < 100) {
        ext_func2(reg2);
        goto mips_target2;
    }
    
    /* Fall through */
    reg3 = ext_func3(reg3);
    goto mips_end;
    
mips_target1:
    /* Simple arithmetic - addition is safe */
    int sum = reg2 + 15;
    global_result += sum;
    goto mips_end;
    
mips_target2:
    /* Another safe operation - subtraction */
    int diff = reg3 - 25;
    global_result += diff;
    /* Fall through */
    
mips_end:
    __asm__ volatile ("# MIPS-like pattern end" : : : "memory");
}

/* Main function that calls all test patterns */
int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize more volatile variables for variety */
    volatile int init_cond = 1;
    volatile int counter = 0;
    
    /* Call test functions multiple times with different conditions */
    for (int i = 0; i < 5; i++) {
        cond1 = (i % 2 == 0);
        cond2 = (i % 3 == 0);
        cond3 = (i % 4 == 0);
        cond4 = (i % 5 == 0);
        cond5 = (i % 2 == 1);
        
        test_function1();
        test_function2();
        test_function3();
        test_function4(0);
        test_mips_pattern();
        
        counter++;
    }
    
    printf("Global result: %d\n", global_result);
    printf("Tests completed.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
int ext_func4(int x) { return x / 2; }
