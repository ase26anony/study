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
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int result = 0;

/* Function with jump-to-label pattern for delay slot candidate */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int x = 0, y = 0, z = 0;
    volatile int local_cond = cond1;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* Parent instruction computation */
    
    /* Call external function to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (local_cond) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = c * 2;
    ext_func2(y);
    
    /* This should not be executed when cond1 != 0 */
    z = d / 3;
    
target_label1:
    /* Candidate for delay slot filling - simple arithmetic */
    /* Uses different variables than parent instruction */
    int temp = c + d;  /* Non-trapping, non-jump instruction */
    result += temp;
    
    /* More operations after label */
    ext_func3(temp);
    a = temp + 1;
}

/* Function with nested control flow */
__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int i, j, k;
    volatile int switch_var = 2;
    
    for (i = 0; i < 3; i++) {
        switch (switch_var) {
            case 1:
                if (cond2) {
                    goto target_label2;
                }
                j = b * i;
                break;
            case 2:
                if (cond3) {
                    /* Jump to label with simple instruction after */
                    goto target_label2;
                }
                k = c + i;
                break;
            default:
                ext_func1(i);
        }
        
        /* Some computation */
        a = a + i;
    }
    
    /* Unreachable without jump */
    d = d * 2;
    
target_label2:
    /* Another delay slot candidate */
    /* Safe operation with no resource conflict */
    int temp = b - a;
    result += temp;
    
    /* Loop continues after label */
    for (int n = 0; n < 2; n++) {
        ext_func2(temp + n);
    }
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    volatile int choice = cond4 ? 1 : 2;
    
    /* Create distinct variable sets */
    volatile int p = 10, q = 20, r = 30;
    
    /* Parent instruction with its own resources */
    p = p * q;
    ext_func3(p);
    
    static void *labels[] = { &&label_a, &&label_b, &&default_label };
    
    if (choice >= 0 && choice < 3) {
        goto *labels[choice];
    }
    
    goto default_label;
    
label_a:
    /* Simple assignment - good delay slot candidate */
    r = q + 5;
    result += r;
    goto end;
    
label_b:
    /* Another simple operation */
    r = p - q;
    result += r;
    goto end;
    
default_label:
    /* Default case with safe operation */
    r = 100;
    result += r;
    
end:
    /* Post-label operations */
    ext_func1(r);
}

/* Function with multiple jump patterns */
__attribute__((optimize("O2"), __noinline__))
void test_function4(int param) {
    volatile int local_a = 1, local_b = 2, local_c = 3;
    volatile int should_jump = (param % 2) == 0;
    
    /* Complex parent instruction */
    asm volatile ("" : : : "memory");  /* Memory clobber for resource separation */
    
    local_a = ext_func1(param);
    
    /* Multiple conditional jumps */
    if (should_jump && cond1) {
        goto target_multi1;
    }
    
    if (local_b > local_c) {
        local_c = ext_func2(local_b);
    }
    
    /* Intermediate block */
    asm volatile ("" : : : "memory");
    
target_multi1:
    /* Simple arithmetic with no trapping */
    int sum = local_b + local_c;
    result += sum;
    
    /* Followed by external call */
    ext_func3(sum);
    
    /* Another jump opportunity */
    if (local_a > 0) {
        goto target_multi2;
    }
    
    local_a = sum * 2;
    
target_multi2:
    /* Another candidate instruction */
    int diff = local_c - local_b;
    result += diff;
}

/* Function designed for MIPS-like delay slot exploration */
__attribute__((optimize("O2"), __noinline__))
void mips_style_function(void) {
    volatile int reg1 = 100, reg2 = 200, reg3 = 300;
    volatile int branch_cond = 1;
    
    /* Simulate MIPS-like computation */
    reg1 = reg1 + reg2;  /* Parent instruction */
    
    /* Inline asm to force specific register usage */
    asm volatile ("# Resource barrier" : : : "memory");
    
    /* Conditional branch to label */
    if (branch_cond) {
        goto mips_target;
    }
    
    /* Alternative path */
    reg3 = ext_func1(reg1);
    
mips_target:
    /* Ideal delay slot candidate:
       - Simple operation (addition)
       - Different registers than parent
       - Non-trapping
       - Not a jump */
    int temp_result = reg2 + 50;
    result += temp_result;
    
    /* Continue execution */
    reg1 = ext_func2(temp_result);
}

/* Main function that exercises all patterns */
int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile control variables */
    cond1 = 1;  /* Force first jump */
    cond2 = 0;  /* Don't force second jump */
    cond3 = 1;  /* Force third jump */
    cond4 = 1;  /* Force computed goto to label_b */
    
    /* Reset result */
    result = 0;
    
    /* Execute test functions multiple times with different parameters */
    for (int i = 0; i < 5; i++) {
        test_function1();
        test_function2();
        test_function3();
        test_function4(i);
        mips_style_function();
        
        /* Modify conditions to explore different paths */
        cond1 = i % 2;
        cond3 = (i + 1) % 2;
    }
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
