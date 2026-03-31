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
volatile int cond5 = 1;

/* Distinct volatile variables for resource separation */
volatile int var_a = 1;
volatile int var_b = 2;
volatile int var_c = 3;
volatile int var_d = 4;
volatile int var_e = 5;
volatile int var_f = 6;
volatile int var_g = 7;
volatile int var_h = 8;

/* Result accumulator */
volatile int result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_function1(void) {
    int local1 = var_a;
    int local2 = var_b;
    
    /* Call external function before jump */
    local1 = ext_func1(local1);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    local2 = var_c + var_d;
    result += local2;
    
    /* This should never be reached if cond1 is true */
    return;
    
target_label1:
    /* Simple non-jump, non-trapping instruction */
    /* Using distinct variables to avoid resource conflicts */
    int local3 = var_e + var_f;
    result += local3;
    
    /* Call external function after label */
    ext_func2(local3);
}

/* Function with different pattern */
__attribute__((optimize("O2"), noinline))
void test_function2(int param) {
    volatile int local_var = param;
    
    /* Complex control flow with switch */
    switch (local_var % 3) {
        case 0:
            if (cond2) {
                __asm__ volatile ("" : : : "memory");
                goto target_label2;
            }
            break;
        case 1:
            local_var = ext_func3(local_var);
            break;
        default:
            break;
    }
    
    /* Loop with jump pattern */
    for (int i = 0; i < 3; i++) {
        if (cond3 && (i % 2 == 0)) {
            __asm__ volatile ("" : : : "memory");
            goto target_label2;
        }
        local_var += i;
    }
    
    /* Fallback return */
    result += local_var;
    return;
    
target_label2:
    /* Simple arithmetic with no resource conflicts */
    int temp = var_g - var_h;
    result += temp;
    
    /* Another external call */
    ext_func1(temp);
}

/* Function using computed goto */
__attribute__((optimize("O2"), noinline))
void test_function3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Use volatile to prevent optimization */
    volatile int selector = cond4 ? 0 : 1;
    
    /* External call before jump */
    int pre_val = ext_func2(var_a);
    
    /* Computed goto */
    goto *labels[selector];
    
    /* Unreachable code */
    result += 999;
    return;
    
label_a:
    /* Simple assignment - no trapping */
    {
        int x = var_b;
        int y = var_c;
        int z = x * y;
        result += z;
    }
    return;
    
label_b:
    /* Another simple operation */
    result += var_d / 2;  /* Division by constant 2 is safe */
    return;
    
label_c:
    /* Safe memory operation */
    {
        volatile int safe_var = 100;
        result += safe_var;
    }
    return;
}

/* Function with nested control flow */
__attribute__((optimize("O3"), noinline))
void test_function4(void) {
    /* Multiple volatile variables for resource separation */
    volatile int r1 = var_a;
    volatile int r2 = var_b;
    volatile int r3 = var_c;
    volatile int r4 = var_d;
    
    /* Complex pre-jump computation */
    r1 = ext_func1(r1);
    r2 = ext_func2(r2);
    
    /* Nested if conditions */
    if (cond5) {
        if (r1 > 0) {
            if (r2 < 100) {
                __asm__ volatile ("" : : : "memory");
                goto target_label4;
            }
        }
    }
    
    /* Alternative path */
    r3 = r3 * r4;
    result += r3;
    return;
    
target_label4:
    /* Simple, safe instruction for delay slot candidate */
    /* Using completely different variables */
    {
        int safe_calc = var_e * var_f;
        result += safe_calc;
    }
    
    /* Final external call */
    ext_func3(result);
}

/* Function specifically for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_function5(void) {
    /* Simulate delay slot parent instruction resources */
    volatile int parent_res1 = var_a;
    volatile int parent_res2 = var_b;
    
    /* Parent instruction computation */
    int parent_result = parent_res1 + parent_res2;
    
    /* External call to create resource barrier */
    ext_func1(parent_result);
    
    /* Jump to label */
    if (parent_result > 0) {
        /* Memory barrier */
        __asm__ volatile ("" : : : "memory");
        goto target_label5;
    }
    
    /* Dead code if jump taken */
    parent_result = ext_func2(parent_result);
    result += parent_result;
    return;
    
target_label5:
    /* Instruction that doesn't reference parent's resources */
    /* Using completely separate variables */
    int candidate = var_g + var_h;  /* Simple addition, no trapping */
    result += candidate;
    
    /* Verify no resource conflict */
    ext_func3(candidate);
}

/* Main function to execute all tests */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;  /* Force jump in test_function1 */
    cond2 = 0;  /* May or may not jump */
    cond3 = 1;  /* Force some jumps */
    cond4 = 1;  /* Control computed goto */
    cond5 = 1;  /* Force jump in test_function4 */
    
    /* Initialize resource variables */
    var_a = 10;
    var_b = 20;
    var_c = 30;
    var_d = 40;
    var_e = 50;
    var_f = 60;
    var_g = 70;
    var_h = 80;
    
    result = 0;
    
    /* Execute test functions multiple times */
    for (int i = 0; i < 3; i++) {
        test_function1();
        test_function2(i);
        test_function3();
        test_function4();
        test_function5();
        
        /* Modify conditions to explore different paths */
        cond1 = i % 2;
        cond4 = (i + 1) % 2;
    }
    
    /* Print result to ensure execution */
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 3; }
