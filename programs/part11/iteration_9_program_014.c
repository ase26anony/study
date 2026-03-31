/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

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
void test_function1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Call external function to create resource barrier */
    ext_func1(a);
    
    /* Conditional jump to label - must be simple jump */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    x = x * 2;
    y = y + 1;
    
target_label1:
    /* This is next_trial - must be non-jump, non-sequence, non-trapping */
    /* Use distinct variables to avoid resource conflicts with insn */
    z = x + y;  /* Simple arithmetic, no trapping */
    
    /* More code to prevent optimization */
    result += z;
    ext_func2(z);
}

/* Function with different pattern */
__attribute__((optimize("O2")))
void test_function2(void) {
    volatile int p = 100, q = 200, r = 0;
    volatile int m = 50, n = 25, o = 0;
    
    /* Loop to create more complex control flow */
    for (volatile int i = 0; i < 3; i++) {
        /* External call before jump */
        ext_func3(i);
        
        /* Conditional jump */
        if (cond2 || (i % 2 == 0)) {
            goto target_label2;
        }
        
        /* Alternative path */
        m = m - 1;
        continue;
        
    target_label2:
        /* Candidate for delay slot filling */
        /* Simple assignment with no trapping */
        o = m - n;  /* Subtraction is safe */
        
        /* Accumulate result */
        result += o;
    }
}

/* Function with switch statement */
__attribute__((optimize("O2")))
void test_function3(void) {
    volatile int val = 2;
    volatile int a = 10, b = 20, res = 0;
    
    switch (val) {
        case 1:
            ext_func1(1);
            if (cond3) goto target_label3;
            break;
        case 2:
            /* This is the path we'll take */
            ext_func2(2);
            if (cond1) goto target_label3;
            /* Fall through */
        case 3:
            a = a * 2;
            break;
        default:
            b = b / 2;  /* Avoid division by zero */
            break;
    }
    
    /* Some intermediate code */
    res = a + 5;
    
target_label3:
    /* Delay slot candidate - must not conflict with parent insn resources */
    /* Use completely different variables */
    volatile int temp1 = 15, temp2 = 30, temp3;
    temp3 = temp1 * temp2;  /* Multiplication, safe with these values */
    
    result += temp3;
}

/* Function with computed goto (&& labels) */
__attribute__((optimize("O2")))
void test_function4(void) {
    static void* labels[] = { &&label1, &&label2, &&label3 };
    volatile int idx = 1;
    volatile int u = 100, v = 200, w = 0;
    
    /* External call to create resource separation */
    ext_func3(u);
    
    /* Computed goto */
    goto *labels[idx];
    
label1:
    u = u + 10;
    goto end;
    
label2:
    /* This is our target - simple non-jump instruction */
    w = v - u;  /* Safe subtraction */
    goto end;
    
label3:
    v = v * 2;
    goto end;
    
end:
    result += w;
}

/* Function with inline assembly to create artificial resource constraints */
__attribute__((optimize("O2")))
void test_function5(void) {
    volatile int reg_var1 = 100, reg_var2 = 200, reg_var3 = 0;
    
    /* Inline assembly with memory clobber to affect resource analysis */
    __asm__ volatile (
        "nop\n\t"
        : 
        : 
        : "memory"
    );
    
    /* Conditional jump */
    if (cond4) {
        goto asm_target_label;
    }
    
    /* Different computation path */
    reg_var1 = reg_var1 + 50;
    
asm_target_label:
    /* Simple instruction that doesn't conflict with resources */
    /* Use fresh variables to avoid any overlap */
    volatile int fresh1 = 300, fresh2 = 400, fresh3;
    fresh3 = fresh1 + fresh2;  /* Safe addition */
    
    /* Another asm barrier */
    __asm__ volatile (
        "nop\n\t"
        : 
        : 
        : "memory"
    );
    
    result += fresh3;
}

/* Function with nested control flow */
__attribute__((optimize("O2")))
void test_function6(void) {
    volatile int outer = 0, inner = 0;
    volatile int a = 1, b = 2, c = 0;
    
    for (outer = 0; outer < 2; outer++) {
        ext_func1(outer);
        
        for (inner = 0; inner < 2; inner++) {
            /* Multiple conditional jumps */
            if (cond1 && (inner == 1)) {
                goto nested_target_label;
            }
            
            if (cond3 && (outer == 0)) {
                a = a * 3;
                continue;
            }
            
            b = b + 1;
        }
        
        /* Continue outer loop */
        continue;
        
    nested_target_label:
        /* Simple instruction at target */
        c = a + b;  /* Safe addition */
        result += c;
        
        /* Break to prevent infinite loops in test */
        break;
    }
}

/* Main function that calls all test functions */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Call test functions multiple times with different conditions */
    for (int i = 0; i < 2; i++) {
        test_function1();
        test_function2();
        test_function3();
        test_function4();
        test_function5();
        test_function6();
        
        /* Change conditions for second iteration */
        if (i == 0) {
            cond2 = 1;
            cond4 = 1;
        }
    }
    
    /* Print result to ensure execution */
    volatile int print_me = result;
    
    /* Use result to prevent dead code elimination */
    if (print_me > 0) {
        return 0;
    } else {
        return 1;
    }
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
