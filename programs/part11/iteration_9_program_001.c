/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

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
volatile int var_i = 90;
volatile int var_j = 100;

/* Global volatile result accumulator */
volatile int global_result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local1 = var_a;
    volatile int local2 = var_b;
    volatile int local3 = var_c;
    
    /* Parent instruction for delay slot */
    int parent_result = local1 + local2;  /* This is 'insn' in the uncovered code */
    
    /* Resource barrier */
    ext_func1(local3);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    local3 = var_d + var_e;
    ext_func2(local3);
    
    /* This should never be reached due to the goto */
    local1 = 999;
    
target_label1:
    /* This is 'next_trial' - simple non-jump, non-sequence instruction */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    volatile int target_result = var_f + var_g;  /* Simple arithmetic */
    
    /* Accumulate result */
    global_result += target_result + parent_result;
    
    /* Another resource barrier */
    ext_func3(target_result);
}

/* Function with different control flow pattern */
__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int x = var_h;
    volatile int y = var_i;
    volatile int z = var_j;
    
    /* Loop with nested conditional jumps */
    for (int i = 0; i < 3; i++) {
        /* Parent instruction using different resources */
        int loop_parent = x * y;
        
        /* Switch statement to create complex control flow */
        switch (i) {
            case 0:
                if (cond2) {
                    /* Another resource barrier */
                    ext_func1(z);
                    goto target_label2;
                }
                break;
            case 1:
                if (cond3) {
                    /* Inline assembly with memory clobber */
                    __asm__ volatile ("" : : : "memory", "eax", "ebx");
                    goto target_label2;
                }
                break;
            case 2:
                /* Force jump with volatile condition */
                volatile int force_jump = cond4;
                if (force_jump) {
                    goto target_label2;
                }
                break;
        }
        
        /* Continue normal loop execution */
        z = ext_func2(z);
        continue;
        
    target_label2:
        /* Simple non-jump instruction at target */
        /* Uses completely different variable set */
        volatile int simple_op = var_a - var_b;  /* Safe subtraction */
        
        global_result += simple_op + loop_parent;
        
        /* Break to avoid infinite loop */
        break;
    }
}

/* Function using computed goto for variety */
__attribute__((optimize("O2"), noinline))
void test_function3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    volatile int selector = cond5 ? 0 : 1;
    
    /* Parent instruction */
    volatile int parent_val = var_c * var_d;
    
    /* Resource barrier before jump */
    ext_func3(parent_val);
    
    /* Computed goto */
    goto *labels[selector];
    
    /* Unreachable code */
    parent_val = 0;
    
label_a:
    /* Simple assignment - no trapping operations */
    volatile int safe_assign = var_e;
    
    global_result += safe_assign + parent_val;
    return;
    
label_b:
    /* Another simple operation */
    volatile int another_safe = var_f + 1;
    
    global_result += another_safe - parent_val;
    return;
    
label_c:
    /* Default case */
    volatile int default_op = var_g / 2;  /* Division by constant is safe */
    
    global_result += default_op * parent_val;
}

/* Function with multiple nested conditionals */
__attribute__((optimize("O2"), cold))
void test_function4(void) {
    volatile int a = var_h;
    volatile int b = var_i;
    volatile int c = var_j;
    
    /* Complex parent instruction */
    int complex_parent = (a << 2) | (b & 0xFF);
    
    /* Nested conditionals */
    if (cond1) {
        if (cond2) {
            ext_func1(a);
        } else {
            if (cond3) {
                /* Multiple resource barriers */
                __asm__ volatile ("" : : : "memory");
                ext_func2(b);
                __asm__ volatile ("" : : : "memory");
                goto target_label4;
            }
        }
    }
    
    /* Alternative path */
    c = ext_func3(c);
    return;
    
target_label4:
    /* Very simple target instruction */
    volatile int simple = var_a;  /* Just a load */
    
    global_result += simple + complex_parent;
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), target("arch=mips32")))
void test_function_mips_like(void) {
    volatile int mips_var1 = 1000;
    volatile int mips_var2 = 2000;
    volatile int mips_var3 = 3000;
    
    /* Instruction that might need a delay slot */
    int mips_parent = mips_var1 << 3;
    
    /* Conditional branch to label */
    if (mips_var2 > 1500) {
        /* Inline assembly to simulate MIPS-like constraints */
        __asm__ volatile (".set noreorder\n\t"
                         ".set nomacro\n\t"
                         : : : "memory");
        goto mips_target;
    }
    
    /* Fall-through path */
    mips_var3 = ext_func1(mips_var3);
    return;
    
mips_target:
    /* Simple instruction suitable for delay slot */
    volatile int mips_simple = mips_var2 + 5;
    
    global_result += mips_simple * mips_parent;
    
    /* Restore normal assembly mode */
    __asm__ volatile (".set reorder\n\t"
                     ".set macro\n\t"
                     : : : "memory");
}

/* Main function that exercises all test patterns */
int main(void) {
    printf("Starting reorg test patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;  /* Force first jump */
    cond2 = 0;  /* Skip some jumps */
    cond3 = 1;  /* Force other jumps */
    cond4 = 0;
    cond5 = 1;
    
    /* Run all test functions */
    test_function1();
    test_function2();
    test_function3();
    test_function4();
    
    /* Try MIPS-like pattern if compiling for appropriate target */
    #if defined(__mips__) || defined(__mips) || defined(__MIPS__)
    test_function_mips_like();
    #endif
    
    printf("Global result: %d\n", global_result);
    printf("Test completed.\n");
    
    return global_result != 0 ? 0 : 1;
}

/* Dummy external function implementations */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 1;
}
