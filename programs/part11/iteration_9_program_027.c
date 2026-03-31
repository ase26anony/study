/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern int ext_func4(int);

/* Volatile control variables */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 0;
volatile int cond4 = 1;
volatile int cond5 = 0;

/* Distinct volatile variables for resource separation */
volatile int var_a = 1;
volatile int var_b = 2;
volatile int var_c = 3;
volatile int var_d = 4;
volatile int var_e = 5;
volatile int var_f = 6;
volatile int var_g = 7;
volatile int var_h = 8;
volatile int var_i = 9;
volatile int var_j = 10;

/* Global volatile result accumulator */
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
    
    /* Some intermediate computation */
    local2 = var_c + var_d;
    ext_func2(local2);
    
    /* Target label with simple non-jump instruction */
    target_label1:
    /* Simple arithmetic - delay slot candidate */
    var_e = var_f + var_g;  /* Does not reference/set parent insn resources */
    
    /* More computation after label */
    local1 = ext_func3(var_h);
    global_result += local1;
}

/* Function with different pattern */
__attribute__((optimize("O2")))
void test_function2(void) {
    volatile int control = cond2;
    int temp1 = var_a;
    int temp2 = var_b;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    /* Jump to label pattern inside a loop */
    for (int i = 0; i < 3; i++) {
        temp1 = ext_func1(temp1);
        
        if (control) {
            goto target_label2;
        }
        
        temp2 = var_c * var_d;
        ext_func2(temp2);
        
        /* Different target label */
        target_label2:
        /* Safe assignment - no trapping */
        var_i = var_j - var_a;  /* Distinct variables from parent */
        
        /* External call after label */
        temp1 = ext_func3(var_b);
        control = !control;
    }
    
    global_result += temp1;
}

/* Function with switch statement and computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    static void* labels[] = { &&case0, &&case1, &&case2 };
    int selector = cond3 ? 1 : 2;
    
    /* Resource setup before jump */
    int res1 = var_a * var_b;
    ext_func1(res1);
    
    /* Computed goto */
    goto *labels[selector % 3];
    
    case0:
        /* This should not be reached with our selector */
        var_c = var_d;
        break;
        
    case1:
        /* Target label instruction - simple addition */
        var_e = var_f + 1;  /* Non-trapping operation */
        
        /* Continue with more operations */
        res1 = ext_func2(var_g);
        break;
        
    case2:
        var_h = var_i;
        break;
    
    /* Common code after switch-like structure */
    __asm__ volatile ("" : : : "memory");
    global_result += res1;
}

/* Function with nested control flow */
__attribute__((optimize("O3")))
void test_function4(void) {
    int x = var_a;
    int y = var_b;
    
    /* Outer loop */
    for (int outer = 0; outer < 2; outer++) {
        /* Inner conditional */
        if (cond4) {
            x = ext_func1(x);
            
            /* Jump to label inside nested block */
            if (cond5) {
                goto target_label4;
            }
            
            y = var_c + var_d;
            ext_func2(y);
            
            /* Multiple basic blocks */
            switch (outer) {
                case 0:
                    x = x * 2;
                    break;
                case 1:
                    x = x / 2;  /* Division but with non-zero divisor */
                    break;
            }
            
            /* Target label with safe operation */
            target_label4:
            /* Simple assignment - good delay slot candidate */
            var_j = var_a + var_b;  /* Uses different vars than parent */
            
            /* External call creates resource separation */
            x = ext_func3(var_e);
        } else {
            x = ext_func4(y);
        }
        
        /* Memory barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    global_result += x;
}

/* Function specifically designed for MIPS-like delay slots */
__attribute__((optimize("O2"), noinline))
void test_function5_mips_like(void) {
    volatile int mips_cond = 1;
    int reg1 = var_a;
    int reg2 = var_b;
    
    /* Simulate MIPS-like register usage pattern */
    reg1 = ext_func1(reg1);
    
    /* Multiple conditional jumps in sequence */
    if (mips_cond) {
        reg2 = ext_func2(reg2);
        goto mips_target1;
    }
    
    reg1 = reg1 * reg2;
    
    mips_target1:
    /* Ideal delay slot instruction:
       - Simple arithmetic
       - No resource conflict with parent
       - Non-trapping */
    var_c = var_d + var_e;  /* All distinct from reg1/reg2 */
    
    /* Follow with more operations */
    reg1 = ext_func3(reg1);
    
    /* Another jump-label pair */
    if (!mips_cond) {
        goto mips_target2;
    }
    
    reg2 = ext_func4(reg2);
    
    mips_target2:
    /* Another candidate instruction */
    var_f = var_g - var_h;
    
    global_result += reg1 + reg2;
}

/* Function with mixed jump types */
__attribute__((optimize("O2")))
void test_function6(void) {
    int val1 = var_a;
    int val2 = var_b;
    
    /* Loop with multiple exit points */
    for (int i = 0; i < 10; i++) {
        val1 = ext_func1(val1);
        
        /* Conditional return (not a simple jump) */
        if (val1 > 100) {
            global_result += val1;
            return;
        }
        
        /* Conditional break */
        if (cond1 && cond2) {
            val2 = ext_func2(val2);
            goto loop_target;
        }
        
        /* Continue with normal loop body */
        val2 = val2 * i;
        
        loop_target:
        /* Target instruction - safe bit operation */
        var_i = var_j & 0xFF;  /* Non-trapping */
        
        /* External call */
        val1 = ext_func3(val1);
        
        /* Conditional continue */
        if (i % 2 == 0) {
            continue;
        }
        
        val2 = ext_func4(val2);
    }
    
    global_result += val2;
}

/* Main function to execute all tests */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 1;
    cond5 = 0;
    
    /* Initialize variables with non-zero values */
    var_a = 42;
    var_b = 17;
    var_c = 89;
    var_d = 123;
    var_e = 256;
    var_f = 512;
    var_g = 1024;
    var_h = 2048;
    var_i = 4096;
    var_j = 8192;
    
    /* Execute test functions */
    test_function1();
    test_function2();
    test_function3();
    test_function4();
    test_function5_mips_like();
    test_function6();
    
    /* Print result to ensure execution */
    printf("Global result: %d\n", global_result);
    
    /* Also print variables to prevent dead code elimination */
    printf("Variables: %d %d %d %d %d %d %d %d %d %d\n",
           var_a, var_b, var_c, var_d, var_e,
           var_f, var_g, var_h, var_i, var_j);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
int ext_func4(int x) { return x / 2; }
