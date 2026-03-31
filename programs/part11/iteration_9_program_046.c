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

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int x = a, y = b, z = c;
    volatile int temp;
    
    /* Create resource set for parent instruction */
    temp = x * y;  /* Parent instruction for delay slot */
    
    /* External call to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    z = ext_func2(z);
    
target_label1:
    /* Non-jump, non-sequence instruction at target */
    /* Use different variables than parent instruction */
    d = a + b;  /* Simple arithmetic, non-trapping */
    
    /* Another external call */
    ext_func3(d);
    
    result += d;
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int p = 100, q = 200, r = 300;
    volatile int local_result = 0;
    
    /* Loop with nested control flow */
    for (int i = 0; i < 3; i++) {
        /* Parent instruction with its own resource set */
        local_result = p * q + r;
        
        /* Switch statement to create complex CFG */
        switch (i) {
            case 0:
                if (cond2) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            case 1:
                if (cond3) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            default:
                break;
        }
        
        /* Continue loop if no jump */
        p = ext_func1(p);
        continue;
        
    target_label2:
        /* Target instruction - simple assignment */
        q = p - 50;  /* Safe, non-trapping operation */
        
        /* Break from loop after target */
        break;
    }
    
    result += local_result;
}

/* Function using computed goto for variety */
__attribute__((optimize("O2"), __noinline__))
void test_pattern3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int m = 1000, n = 2000;
    volatile int selector = 1;
    
    /* Parent instruction */
    int computation = m * 2;
    
    /* External call between parent and jump */
    ext_func2(computation);
    
    /* Conditional computed goto */
    if (cond4) {
        goto *labels[selector];
    }
    
    /* Fall-through path */
    n = ext_func3(n);
    goto end;
    
label_a:
    /* Simple arithmetic at target */
    m = n + 100;
    goto end;
    
label_b:
    /* Another simple operation */
    m = n - 100;
    goto end;
    
label_c:
    /* Yet another simple operation */
    m = n * 2;
    /* Fall through */
    
end:
    result += m;
}

/* Function with multiple potential delay slot candidates */
__attribute__((optimize("O2"), __noinline__))
void test_pattern4(void) {
    volatile int var1 = 1, var2 = 2, var3 = 3, var4 = 4;
    volatile int tmp;
    
    /* Multiple parent instructions in sequence */
    for (int j = 0; j < 2; j++) {
        /* First parent */
        tmp = var1 * var2;
        
        /* Jump to label after external call */
        ext_func1(tmp);
        
        if (j == 0 && cond1) {
            __asm__ volatile ("" : : : "memory");
            goto target_a;
        }
        
        /* Second parent */
        tmp = var3 + var4;
        
        if (j == 1 && cond3) {
            __asm__ volatile ("" : : : "memory");
            goto target_b;
        }
        
        continue;
        
    target_a:
        /* First target - simple assignment */
        var4 = var1 + var2;
        continue;
        
    target_b:
        /* Second target - different simple operation */
        var3 = var2 - var1;
        break;
    }
    
    result += var3 + var4;
}

/* Function specifically designed for MIPS-like delay slots */
__attribute__((optimize("O2"), __noinline__))
void test_mips_pattern(void) {
    volatile int reg1 = 10, reg2 = 20, reg3 = 30;
    volatile int delay_candidate;
    
    /* Simulate load/store pattern common in MIPS */
    reg1 = reg2 + reg3;  /* Parent instruction */
    
    /* Force conditional branch */
    if (reg1 > 0) {
        /* Inline asm to prevent optimization */
        __asm__ volatile (
            "# MIPS-like barrier"
            : 
            : 
            : "memory"
        );
        goto mips_target;
    }
    
    /* Alternative path */
    reg3 = ext_func2(reg3);
    return;
    
mips_target:
    /* Candidate for delay slot - uses different register */
    delay_candidate = reg2 * 2;  /* Simple, non-trapping */
    
    result += delay_candidate;
}

/* Main function that exercises all patterns */
int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    
    /* Run test patterns multiple times */
    for (int run = 0; run < 2; run++) {
        test_pattern1();
        test_pattern2();
        test_pattern3();
        test_pattern4();
        test_mips_pattern();
        
        /* Toggle conditions for different paths */
        cond1 = !cond1;
        cond3 = !cond3;
    }
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 1;
}
