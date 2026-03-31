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
volatile int cond5 = 1;

/* Volatile computation variables - separate sets to avoid resource conflicts */
volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
volatile int x = 10, y = 20, z = 30;
volatile int p = 100, q = 200, r = 300;
volatile int result = 0;

/* Function with O2 optimization and jump-to-label pattern */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local_cond = cond1;
    volatile int local_a = a;
    volatile int local_b = b;
    
    /* Call external function to create resource barrier */
    int tmp = ext_func1(local_a);
    
    /* Conditional jump to label */
    if (local_cond) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    local_b = local_b * 2;
    
target_label1:
    /* Simple non-jump, non-trapping instruction at target */
    /* This is the candidate for delay slot filling (next_trial) */
    volatile int local_c = c;
    local_c = local_a + local_b;  /* Simple arithmetic, no traps */
    
    /* Another external call */
    tmp = ext_func2(local_c);
    
    result += local_c;
}

/* Function with switch statement and multiple labels */
__attribute__((optimize("O2")))
void test_function2(int mode) {
    volatile int local_cond = cond2;
    volatile int local_x = x;
    volatile int local_y = y;
    
    switch (mode) {
        case 1:
            if (local_cond) {
                __asm__ volatile ("" : : : "memory");
                goto target_label2;
            }
            local_x += 5;
            break;
            
        case 2:
            local_y *= 2;
            break;
            
        default:
            break;
    }
    
    /* Some computation */
    int tmp = ext_func3(local_x);
    
    if (tmp > 0) {
        __asm__ volatile ("" : : : "memory");
        goto target_label2;
    }
    
    return;
    
target_label2:
    /* Another simple instruction at target */
    volatile int local_z = z;
    local_z = local_x - local_y;  /* Simple subtraction, no traps */
    
    result += local_z;
}

/* Function with loop and computed goto */
__attribute__((optimize("O3")))
void test_function3(void) {
    volatile int local_cond = cond3;
    volatile int local_p = p;
    volatile int local_q = q;
    
    /* Labels for computed goto */
    static void *labels[] = { &&label0, &&label1, &&target_label3 };
    
    /* Loop with conditional jumps */
    for (int i = 0; i < 3; i++) {
        if (local_cond) {
            __asm__ volatile ("" : : : "memory");
            goto *labels[i];
        }
        
        local_p += i;
        
        if (i == 1) {
            /* Another conditional jump */
            if (cond4) {
                goto target_label3;
            }
        }
        
    label0:
        local_q += 1;
        continue;
        
    label1:
        local_q += 2;
        continue;
        
    target_label3:
        /* Simple assignment at target */
        volatile int local_r = r;
        local_r = local_p * local_q;  /* Multiplication, no division */
        
        result += local_r;
        break;
    }
}

/* Function with nested conditionals */
__attribute__((optimize("O2"), noinline))
void test_function4(void) {
    volatile int local_cond = cond5;
    volatile int local_d = d;
    volatile int local_e = e;
    
    /* Complex nested conditionals */
    if (local_cond) {
        int tmp1 = ext_func1(local_d);
        
        if (tmp1 > 0) {
            __asm__ volatile ("" : : : "memory");
            goto target_label4;
        } else {
            local_d += 10;
        }
    } else {
        local_e += 20;
    }
    
    /* More code */
    local_d = ext_func2(local_d);
    
    if (local_d < 100) {
        return;
    }
    
target_label4:
    /* Simple bitwise operation at target */
    volatile int local_result = result;
    local_result = local_d & local_e;  /* Bitwise AND, no traps */
    
    result = local_result;
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_like(void) {
    volatile int local_a = a;
    volatile int local_b = b;
    volatile int local_c = c;
    
    /* Pattern that might look like a delay slot candidate on MIPS */
    if (cond1) {
        /* External call to separate resources */
        int tmp = ext_func3(local_a);
        
        /* Conditional jump */
        if (tmp) {
            __asm__ volatile ("" : : : "memory");
            goto mips_target;
        }
        
        local_b = local_b * 3;
    }
    
    local_c = ext_func1(local_b);
    
mips_target:
    /* Very simple instruction - ideal delay slot candidate */
    volatile int local_x = x;
    local_x = local_a + 1;  /* Simple increment */
    
    result += local_x;
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting reorg test...\n");
    
    /* Initialize volatile variables with different values */
    cond1 = 1; cond2 = 0; cond3 = 1; cond4 = 0; cond5 = 1;
    a = 5; b = 7; c = 11; d = 13; e = 17;
    x = 23; y = 29; z = 31;
    p = 37; q = 41; r = 43;
    result = 0;
    
    /* Call test functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        test_function1();
        test_function2(i % 3);
        test_function3();
        test_function4();
        test_mips_like();
        
        /* Change conditions to exercise different paths */
        cond1 = !cond1;
        cond2 = !cond2;
        cond3 = (i % 2);
        cond4 = (i > 1);
        cond5 = (i < 2);
    }
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
