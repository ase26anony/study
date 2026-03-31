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

/* Global volatile variables for distinct resource sets */
volatile int var_a = 1, var_b = 2, var_c = 3;
volatile int var_x = 10, var_y = 20, var_z = 30;
volatile int var_m = 100, var_n = 200, var_p = 300;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    /* Parent instruction resources (simulated delay slot) */
    volatile int parent_res1 = var_a + var_b;
    
    /* External call before jump */
    int tmp = ext_func1(parent_res1);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        asm volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    var_c = tmp + 1;
    
target_label1:
    /* Candidate for delay slot filling - simple arithmetic with different vars */
    /* Must not reference/set parent_res1 resources */
    var_x = var_y + var_z;  /* Uses distinct variable set */
    
    /* External call after target */
    ext_func2(var_x);
    
    result += var_x;
}

/* Another pattern with switch statement */
__attribute__((optimize("O2")))
void test_pattern2(void) {
    volatile int switch_var = 2;
    volatile int parent_res2 = var_m * var_n;
    
    ext_func2(parent_res2);
    
    switch (switch_var) {
        case 1:
            if (cond2) goto target_label2;
            break;
        case 2:
            /* Complex conditional jump pattern */
            for (int i = 0; i < 3; i++) {
                if (cond3 && !cond4) {
                    asm volatile ("" : : : "memory");
                    goto target_label2;
                }
                ext_func3(i);
            }
            break;
        default:
            break;
    }
    
    /* Some code that won't be executed due to the goto */
    var_p = 999;
    
target_label2:
    /* Safe non-trapping operation - subtraction */
    var_a = var_b - var_c;  /* Different resource set from parent_res2 */
    
    /* Prevent optimization merging */
    asm volatile ("" : : : "memory");
    
    result += var_a;
}

/* Function with computed goto */
__attribute__((optimize("O3")))
void test_pattern3(void) {
    static const void* labels[] = { &&label1, &&label2, &&label3 };
    
    volatile int idx = 1;
    volatile int parent_res3 = var_x * var_y;
    
    /* Resource-intensive parent operation */
    for (int i = 0; i < 10; i++) {
        parent_res3 += i;
        asm volatile ("" : : : "memory");
    }
    
    ext_func3(parent_res3);
    
    /* Conditional jump using computed goto */
    if (cond1 && cond3) {
        goto *labels[idx];
    }
    
    /* Fall-through code */
    var_z = 50;
    goto end;
    
label1:
    /* Not a jump target candidate (too simple) */
    return;
    
label2:
    /* Candidate instruction - simple assignment */
    var_m = var_n;  /* Different resource set from parent_res3 */
    
    /* External call to separate resources */
    ext_func1(var_m);
    
    result += var_m;
    goto end;
    
label3:
    var_p = 1000;
    goto end;
    
end:
    return;
}

/* Function with nested loops and multiple labels */
__attribute__((optimize("O2"), noinline))
void test_pattern4(void) {
    volatile int loop_cond = 5;
    volatile int parent_res4 = 0;
    
    /* Complex parent computation */
    for (int i = 0; i < loop_cond; i++) {
        parent_res4 += var_a + var_b + var_c;
        asm volatile ("" : : : "memory");
        
        for (int j = 0; j < 2; j++) {
            if (cond1 || cond2) {
                if (j == 1) {
                    ext_func2(parent_res4);
                    
                    /* Jump to label */
                    if (cond3) {
                        goto target_label4;
                    }
                }
            }
        }
    }
    
    /* Alternative path */
    var_x = 99;
    return;
    
target_label4:
    /* Simple arithmetic - addition with constants */
    var_y = 25 + 17;  /* No variable references that conflict with parent */
    
    result += var_y;
}

/* Function designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int a = 1, b = 2, c = 3;
    volatile int x = 10, y = 20;
    
    /* Parent instruction simulation */
    int parent_val = a * b + c;
    ext_func1(parent_val);
    
    /* Multiple conditional jumps */
    if (cond1) {
        if (cond2) {
            asm volatile ("" : : : "memory");
            goto mips_target;
        } else {
            ext_func2(0);
        }
    }
    
    /* Intermediate basic block */
    x = y * 2;
    
mips_target:
    /* Ideal delay slot candidate:
       - Simple operation (assignment)
       - Uses different registers/variables than parent
       - Non-trapping
       - Not a jump */
    y = x + 5;  /* x and y are different from a,b,c used in parent */
    
    result += y;
}

/* Main function to execute all patterns */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    
    /* Execute test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_mips_pattern();
    
    /* Additional executions with different conditions */
    cond2 = 1;
    cond4 = 1;
    test_pattern1();
    test_pattern2();
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
