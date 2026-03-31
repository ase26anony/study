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
volatile int counter = 0;
volatile int result = 0;

/* Global volatile variables for resource separation */
volatile int gvar1 = 100;
volatile int gvar2 = 200;
volatile int gvar3 = 300;
volatile int gvar4 = 400;
volatile int gvar5 = 500;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local1 = 10;
    volatile int local2 = 20;
    volatile int local3 = 30;
    
    /* Parent instruction computation - uses distinct resources */
    int parent_compute = gvar1 + gvar2;
    
    /* External call before jump */
    ext_func1(parent_compute);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    local1 = ext_func2(local1);
    
target_label1:
    /* Target instruction - simple arithmetic with non-overlapping resources */
    /* This is the candidate for delay slot filling (next_trial) */
    int target_compute = gvar3 * gvar4;  /* Different variables than parent */
    
    /* External call after target label */
    ext_func3(target_compute);
    
    /* Use result to prevent optimization */
    result += target_compute + parent_compute;
}

/* Function with switch statement and nested control flow */
__attribute__((optimize("O2")))
void test_function2(void) {
    volatile int state = counter % 3;
    volatile int a = 1000;
    volatile int b = 2000;
    volatile int c = 3000;
    
    for (int i = 0; i < 5; i++) {
        switch (state) {
            case 0: {
                /* Parent instruction */
                int parent_val = a * b;
                
                /* Jump to label pattern inside switch case */
                if (cond2) {
                    goto target_label2;
                }
                
                /* Some computation */
                c = ext_func1(c);
                break;
            }
            case 1: {
                /* Another parent instruction */
                int parent_val2 = b - a;
                
                /* Different jump pattern */
                if (cond3) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                
                a = ext_func2(a);
                break;
            }
            default: {
                /* Default case with computation */
                b = ext_func3(b);
                break;
            }
        }
        
        /* Continue loop if not jumping */
        continue;
        
    target_label2:
        /* Target instruction - safe, non-trapping operation */
        /* Uses completely different variables than any parent */
        int target_val = gvar5 + 50;  /* gvar5 not used elsewhere in this function */
        
        /* External call */
        ext_func1(target_val);
        
        result += target_val;
        break;  /* Exit switch/loop after taking jump */
    }
}

/* Function with computed goto */
__attribute__((optimize("O3")))
void test_function3(void) {
    volatile int choice = counter % 2;
    
    /* Label array for computed goto */
    static void* labels[] = { &&label_a, &&label_b, &&normal_flow };
    
    /* Parent instruction with resource usage */
    volatile int parent_res = gvar1 * 2;
    ext_func2(parent_res);
    
    /* Computed goto */
    goto *labels[choice];
    
label_a:
    /* Intermediate computation */
    parent_res = ext_func3(parent_res);
    /* Fall through to target label */
    
label_b:
    /* Target instruction - simple assignment */
    /* Uses volatile to prevent optimization */
    volatile int target_res = gvar2 + gvar3;  /* Different resources */
    
    /* Memory barrier */
    __asm__ volatile ("" : : : "memory");
    
    result += target_res;
    return;
    
normal_flow:
    /* Normal path without jump to label */
    result += parent_res;
}

/* Function with loop and multiple jump targets */
__attribute__((optimize("O2"), noinline))
void test_function4(int iterations) {
    volatile int x = 0;
    volatile int y = 0;
    volatile int z = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Parent instruction in loop */
        int loop_parent = x + y;
        
        /* Conditional jump inside loop */
        if (cond4 || (i % 3 == 0)) {
            /* Inline assembly for resource separation */
            __asm__ volatile ("" : : : "memory");
            goto loop_target;
        }
        
        /* Normal loop body */
        x = ext_func1(x);
        y = ext_func2(y);
        continue;
        
    loop_target:
        /* Target instruction - safe arithmetic */
        /* Uses 'z' which isn't used in parent computation */
        z = z + gvar4;  /* gvar4 not used in parent */
        
        /* External call */
        ext_func3(z);
        
        result += z + loop_parent;
    }
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int temp;
    
    /* Pattern 1: if-goto with simple target */
    if (cond1) {
        temp = a + b;
        ext_func1(temp);
        goto mips_target1;
    }
    
    c = ext_func2(c);
    
mips_target1:
    /* Simple target instruction - addition with safe operands */
    d = d + 10;  /* No division, no trapping */
    result += d;
    
    /* Pattern 2: nested conditionals */
    if (cond2) {
        if (cond3) {
            temp = b * c;
            goto mips_target2;
        }
        a = ext_func3(a);
    }
    
    b = ext_func1(b);
    return;
    
mips_target2:
    /* Another simple target */
    c = c - 5;  /* Safe subtraction */
    result += c;
}

/* Main function that drives all test patterns */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 1;
    
    /* Run test functions multiple times with different conditions */
    for (counter = 0; counter < 10; counter++) {
        test_function1();
        
        /* Vary conditions */
        cond2 = counter % 2;
        cond4 = counter % 3;
        
        test_function2();
        test_function3();
        test_function4(3);
        test_mips_pattern();
    }
    
    /* Print result to ensure execution */
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
