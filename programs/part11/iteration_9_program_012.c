/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Call external function to create resource barrier */
    int r1 = ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    c = a + b;
    ext_func2(c);
    
target_label1:
    /* Simple arithmetic operation - candidate for delay slot */
    z = x + y;
    
    /* Use result to prevent optimization */
    result += z + r1;
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int m = 100, n = 200;
    volatile int p = 50, q = 25, r = 0;
    
    /* Loop with nested conditional */
    for (volatile int i = 0; i < 3; i++) {
        /* Resource separation with external call */
        int r2 = ext_func2(m);
        
        /* Switch statement to create complex control flow */
        switch (i) {
            case 0:
                if (cond2) {
                    goto target_label2;
                }
                break;
            case 1:
                if (cond3) {
                    goto target_label2;
                }
                break;
            default:
                /* Force jump with volatile condition */
                volatile int local_cond = 1;
                if (local_cond) {
                    goto target_label2;
                }
        }
        
        /* Some computation that shouldn't interfere */
        m = m + n;
        continue;
        
    target_label2:
        /* Safe non-trapping operation */
        r = p - q;
        
        /* Accumulate result */
        result += r + r2;
        
        /* Break to avoid infinite loop in this path */
        break;
    }
}

/* Function with inline assembly to create artificial resource constraints */
__attribute__((optimize("O2"), noinline))
void test_function3(void) {
    volatile int u = 1000, v = 2000, w = 0;
    volatile int s = 500, t = 300, u_result = 0;
    
    /* Inline assembly with memory clobber */
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        : 
        : 
        : "memory"
    );
    
    /* Computed goto for variety */
    void* labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Use volatile to prevent optimization */
    volatile int index = 0;
    
    if (cond4) {
        goto *labels[index];
    }
    
    /* Different path */
    u = ext_func3(u);
    goto end;
    
label_a:
    /* Simple assignment - good delay slot candidate */
    u_result = s * t;
    result += u_result;
    goto end;
    
label_b:
    /* Another simple operation */
    w = v - u;
    result += w;
    goto end;
    
label_c:
    /* Safe bit operation */
    u_result = s & t;
    result += u_result;
    
end:
    return;
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int reg1 = 10, reg2 = 20, reg3 = 0;
    volatile int reg4 = 30, reg5 = 40, reg6 = 0;
    
    /* Multiple external calls to separate resources */
    int temp1 = ext_func1(reg1);
    int temp2 = ext_func2(reg2);
    
    /* Series of conditional jumps */
    if (temp1 > 0) {
        goto mips_target1;
    }
    
    /* Intermediate computation with different registers */
    reg3 = reg1 * reg2;
    ext_func3(reg3);
    
    if (temp2 < 0) {
        goto mips_target2;
    }
    
mips_target1:
    /* Very simple operation - ideal delay slot candidate */
    reg6 = reg4 + reg5;
    result += reg6;
    return;
    
mips_target2:
    /* Another simple operation */
    reg6 = reg4 - reg5;
    result += reg6;
}

/* Function with switch and multiple labels */
__attribute__((optimize("O2")))
void test_switch_pattern(void) {
    volatile int val = 2;
    volatile int a = 100, b = 200, c = 0;
    
    switch (val) {
        case 1: {
            int r = ext_func1(a);
            if (r > 0) {
                goto switch_target;
            }
            c = a + b;
            break;
        }
        case 2: {
            int r = ext_func2(b);
            if (r < 0) {
                goto switch_target;
            }
            c = a - b;
            break;
        }
        case 3: {
            volatile int local_cond = 1;
            if (local_cond) {
                goto switch_target;
            }
            c = a * b;
            break;
        }
        default:
            c = b / 2;
    }
    
    /* Default path */
    result += c;
    return;
    
switch_target:
    /* Safe operation - no division by volatile zero */
    c = b + 100;
    result += c;
}

/* Main function that calls all test patterns */
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
        test_mips_pattern();
        test_switch_pattern();
        
        /* Change conditions to exercise different paths */
        cond1 = !cond1;
        cond2 = !cond2;
        cond3 = !cond3;
        cond4 = !cond4;
    }
    
    /* Print result to ensure execution */
    volatile int print_me = result;
    
    /* Use result to prevent dead code elimination */
    return print_me > 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
