/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 0;
volatile int cond4 = 1;
volatile int result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int d = 5, e = 15;
    
    /* Call external function to create resource separation */
    ext_func1(a);
    
    /* Conditional jump to label - essential for jump_to_label_p(trial) */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate computation */
    c = a + b;
    ext_func2(c);
    
    /* Another conditional jump */
    if (cond2) {
        goto target_label2;
    }
    
    /* More computations */
    d = d * 2;
    
target_label1:
    /* This is next_trial - must be non-jump, non-sequence, non-trapping */
    /* Simple arithmetic that doesn't conflict with parent instruction resources */
    e = e + 3;  /* Safe addition */
    
    /* Continue execution */
    result += e;
    
target_label2:
    /* Another candidate for next_trial */
    a = a - 2;  /* Safe subtraction */
    result += a;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
}

/* Function with switch statement and nested control flow */
__attribute__((optimize("O2"), noinline))
void test_pattern2(int mode) {
    volatile int x = 100, y = 200, z = 0;
    volatile int temp;
    
    /* Complex control flow with switch */
    switch (mode) {
        case 0:
            if (cond3) {
                goto case0_target;
            }
            z = x * y;
            break;
            
        case 1:
            for (int i = 0; i < 3; i++) {
                if (i == 1 && cond4) {
                    goto loop_target;
                }
                temp = ext_func3(i);
            }
            break;
            
        default:
            break;
    }
    
    /* Return early to avoid executing target labels from wrong paths */
    if (mode != 0 && mode != 1) {
        return;
    }
    
case0_target:
    /* Candidate instruction for delay slot filling */
    /* Uses distinct variables to avoid resource conflicts */
    y = y + 10;  /* Simple, non-trapping operation */
    result += y;
    return;
    
loop_target:
    /* Another candidate in loop context */
    x = x - 5;   /* Simple, non-trapping operation */
    result += x;
    
    /* Inline assembly barrier */
    __asm__ volatile ("" : : : "memory");
}

/* Function with computed goto (labels as values) */
__attribute__((optimize("O3")))
void test_pattern3(void) {
    volatile int p = 50, q = 60, r = 0;
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* External call for resource separation */
    ext_func2(p);
    
    /* Conditional jump using computed goto */
    if (cond1) {
        goto *labels[0];
    }
    
    /* Some computation */
    r = p * q;
    ext_func1(r);
    
    /* Another conditional */
    if (cond2) {
        goto *labels[1];
    }
    
    /* More code to avoid fall-through */
    p = p + 100;
    
label_a:
    /* Delay slot candidate - simple assignment */
    q = 99;  /* Constant assignment, no trapping */
    result += q;
    goto end;
    
label_b:
    /* Another candidate - safe arithmetic */
    r = p + q;  /* Addition with no volatile divisor */
    result += r;
    goto end;
    
label_c:
    /* Not used in this path */
    return;
    
end:
    /* Memory clobber to prevent optimization */
    __asm__ volatile ("" : : : "memory");
}

/* Function specifically designed for MIPS delay slots */
__attribute__((optimize("O2"), noinline))
void test_mips_delay_slot(void) {
    volatile int m = 1000, n = 2000;
    volatile int counter = 0;
    
    /* Multiple conditional jumps in sequence */
    for (int i = 0; i < 4; i++) {
        /* Vary the condition */
        volatile int local_cond = (i & 1);
        
        /* Call external function to separate resources */
        ext_func3(i);
        
        if (local_cond) {
            goto mips_target;
        }
        
        /* Some computation that uses different resources */
        counter += m * i;
        
        /* Continue loop */
        continue;
        
    mips_target:
        /* Instruction at jump target - must be eligible for delay slot */
        /* Simple operation with no resource conflicts */
        n = n + i;  /* Safe: no division, no memory access that could trap */
        result += n;
        
        /* Inline assembly to mark instruction boundary */
        __asm__ volatile ("# MIPS delay slot candidate" : : : "memory");
    }
}

/* Function with mixed operations and multiple labels */
__attribute__((optimize("O2")))
void test_mixed_pattern(void) {
    volatile int var1 = 1, var2 = 2, var3 = 3;
    volatile int var4 = 4, var5 = 5;
    
    /* Use all external functions to create complex resource picture */
    var1 = ext_func1(var1);
    
    /* Nested if statements with goto */
    if (cond1) {
        if (cond2) {
            ext_func2(var2);
            goto mixed_target1;
        } else {
            var3 = ext_func3(var3);
        }
    }
    
    /* Switch with goto labels */
    switch (var1) {
        case 1:
            goto mixed_target2;
        case 2:
            var4 = var4 * 2;
            break;
        default:
            goto mixed_target3;
    }
    
    /* This should not be reached in some paths */
    var5 = 999;
    
mixed_target1:
    /* Simple assignment - good delay slot candidate */
    var2 = 42;  /* Constant assignment */
    result += var2;
    return;
    
mixed_target2:
    /* Simple arithmetic */
    var3 = var3 + 1;  /* Increment */
    result += var3;
    return;
    
mixed_target3:
    /* Another simple operation */
    var4 = var4 - 1;  /* Decrement */
    result += var4;
    
    /* Memory barrier */
    __asm__ volatile ("" : : : "memory");
}

/* Main function that calls all test patterns */
int main(void) {
    /* Initialize volatile conditions with non-constant values */
    cond1 = 1;  /* Will take some jumps */
    cond2 = 0;  /* Will not take others */
    cond3 = 1;
    cond4 = 0;
    
    /* Call test functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        test_pattern1();
        test_pattern2(i % 3);  /* Vary the mode */
        test_pattern3();
        test_mips_delay_slot();
        test_mixed_pattern();
        
        /* Change conditions to exercise different paths */
        cond1 = !cond1;
        cond2 = !cond2;
        cond3 = (i == 1);
        cond4 = (i == 2);
    }
    
    /* Print result to ensure execution */
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
