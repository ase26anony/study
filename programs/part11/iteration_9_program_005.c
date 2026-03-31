/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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
volatile int result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource set for parent instruction */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        /* This goto creates a simplejump_p */
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    x = ext_func2(x);
    
    /* This should not be reached when cond1 is true */
    z = x * y;
    
target_label1:
    /* Non-jump, non-sequence instruction at jump target */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    c = b + 15;  /* Simple arithmetic, no trapping */
    
    /* External call to separate resources */
    ext_func3(c);
    
    result += c;
}

/* Another pattern with different operations */
__attribute__((optimize("O2")))
void test_pattern2(void) {
    volatile int m = 100, n = 200;
    volatile int p = 50, q = 25;
    volatile int r = 0;
    
    /* Different computation for parent instruction */
    ext_func2(m);
    
    /* Nested control flow */
    for (int i = 0; i < 3; i++) {
        if (cond2) {
            goto target_label2;
        }
        
        /* Vary the complexity */
        switch (i) {
            case 0: p += 1; break;
            case 1: p += 2; break;
            default: p += 3;
        }
        
        if (cond3 && i == 1) {
            /* Another jump opportunity */
            goto target_label2;
        }
    }
    
    /* Fall-through path */
    r = p * q;
    goto after_label;
    
target_label2:
    /* Safe non-trapping operation at target */
    r = n - 75;  /* Subtraction, no division */
    
after_label:
    ext_func1(r);
    result += r;
}

/* Pattern with computed goto */
__attribute__((optimize("O3")))
void test_pattern3(void) {
    volatile int u = 1000, v = 2000;
    volatile int w = 0;
    
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Parent instruction with inline assembly for resource constraints */
    asm volatile ("" : : : "memory");
    
    /* Conditional jump using computed goto */
    if (cond4) {
        goto *labels[0];
    }
    
    /* Intermediate computation */
    w = ext_func3(u);
    
    /* Jump to different labels based on condition */
    if (w > 500) {
        goto *labels[1];
    } else {
        goto *labels[2];
    }
    
label_a:
    /* Target instruction - simple assignment */
    v = 999;
    goto end;
    
label_b:
    /* Another target - safe operation */
    v = v + 1;
    goto end;
    
label_c:
    /* Yet another - multiplication is safe with non-zero */
    v = v * 2;
    
end:
    asm volatile ("" : : : "memory");
    result += v;
}

/* Pattern inside loop with multiple exit points */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int counter = 0;
    volatile int accum = 0;
    volatile int temp;
    
    while (counter < 10) {
        /* Parent instruction computation */
        temp = ext_func1(counter);
        
        /* Multiple conditional jumps to same label */
        if (cond1 && (counter % 2 == 0)) {
            goto loop_target;
        }
        
        if (cond3 && (counter % 3 == 0)) {
            goto loop_target;
        }
        
        /* Normal loop body */
        accum += counter * 2;
        counter++;
        continue;
        
    loop_target:
        /* Instruction at jump target - different resource set */
        accum += 100;  /* Simple addition */
        counter++;
    }
    
    ext_func2(accum);
    result += accum;
}

/* Function with more complex control flow */
__attribute__((optimize("O3"), __noinline__))
void test_pattern5(int mode) {
    volatile int val1 = 42, val2 = 84;
    volatile int output = 0;
    
    /* Create artificial resource pressure */
    asm volatile ("# Resource barrier" : : : "memory");
    
    switch (mode) {
        case 0:
            if (cond1) {
                goto switch_target;
            }
            output = val1 * 2;
            break;
            
        case 1:
            if (cond2) {
                goto switch_target;
            }
            output = val1 + val2;
            break;
            
        case 2:
            if (cond3) {
                goto switch_target;
            }
            output = val2 / 2;  /* Division by constant is safe */
            break;
            
        default:
            output = 0;
    }
    
    goto after_switch;
    
switch_target:
    /* Target instruction - uses completely different variable set */
    {
        volatile int safe_var1 = 1000, safe_var2 = 2000;
        output = safe_var1 + safe_var2;  /* Safe addition */
    }
    
after_switch:
    /* External call to clobber registers */
    ext_func3(output);
    
    asm volatile ("# End of resource section" : : : "memory");
    result += output;
}

/* Dummy external functions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }

int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;  /* Force first jump */
    cond2 = 0;  /* Skip some jumps */
    cond3 = 1;  /* Force other jumps */
    cond4 = 0;  /* Skip computed goto */
    
    printf("Testing delay slot patterns...\n");
    
    /* Execute all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    
    /* Test with different modes */
    for (int i = 0; i < 3; i++) {
        test_pattern5(i);
    }
    
    /* Additional runs with different conditions */
    cond2 = 1;
    cond4 = 1;
    test_pattern2();
    test_pattern3();
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
