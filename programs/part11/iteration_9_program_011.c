/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 0;
volatile int cond4 = 1;
volatile int global_result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource usage before jump */
    c = ext_func1(a + b);
    
    /* Conditional jump to label */
    if (cond1) {
        /* This goto creates a simplejump_p */
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    z = x * y;
    ext_func2(z);
    
    /* This should NOT be executed if cond1 is true */
    c = a - b;
    
target_label1:
    /* This is next_trial - simple non-jump, non-sequence instruction */
    /* Use different variables than those in potential delay slot parent */
    volatile int p = 100, q = 200, r = 0;
    r = p + q;  /* Simple addition, no trapping */
    
    /* Ensure result is used */
    global_result += r;
}

/* Function with different pattern */
__attribute__((optimize("O2")))
void test_function2(void) {
    volatile int m = 50, n = 30, o = 0;
    volatile int u = 15, v = 25, w = 0;
    
    /* Call external function to create resource set */
    o = ext_func2(m * n);
    
    /* Complex control flow with switch */
    switch (cond2) {
        case 0:
            w = u / 2;  /* Safe division */
            break;
        case 1:
            if (cond3) {
                goto target_label2;
            }
            w = v * 3;
            break;
        default:
            w = 0;
    }
    
    /* More code to separate blocks */
    ext_func3(w);
    
    /* Another potential jump */
    if (cond4) {
        /* This creates another trial */
        goto target_label3;
    }
    
target_label2:
    /* Candidate for next_trial */
    volatile int d1 = 1000, d2 = 2000, d3 = 0;
    d3 = d1 - d2;  /* Simple subtraction */
    global_result += d3;
    
    return;
    
target_label3:
    /* Another candidate */
    volatile int e1 = 77, e2 = 33, e3 = 0;
    e3 = e1 & e2;  /* Bitwise AND - non-trapping */
    global_result += e3;
}

/* Function with loop and label at beginning */
__attribute__((optimize("O3")))
void test_function3(void) {
    volatile int i, j, k;
    volatile int sum = 0;
    
    for (i = 0; i < 10; i++) {
        /* Jump to label inside loop */
        if (cond1 && cond2) {
            goto loop_target;
        }
        
        /* Normal loop body */
        j = ext_func1(i);
        sum += j;
        
        /* Continue to next iteration */
        continue;
        
    loop_target:
        /* Simple instruction after label */
        k = i * 2;
        sum += k;
        
        /* Inline assembly to affect resource analysis */
        __asm__ volatile ("" : : : "memory");
    }
    
    global_result += sum;
}

/* Function using computed goto */
__attribute__((optimize("O2")))
void test_function4(void) {
    volatile int val = cond3 ? 1 : 2;
    volatile int res = 0;
    
    /* Label array for computed goto */
    static void* labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* External call before jump */
    ext_func3(val);
    
    /* Computed goto - may create different RTL pattern */
    goto *labels[val % 3];
    
    /* Unreachable code */
    res = 100;
    
label_a:
    /* Simple assignment */
    volatile int tmp1 = 42;
    global_result += tmp1;
    return;
    
label_b:
    /* Another simple operation */
    volatile int tmp2 = 84;
    tmp2 = tmp2 / 2;  /* Safe division by constant */
    global_result += tmp2;
    return;
    
label_c:
    /* Memory operation */
    volatile int tmp3 = 0;
    volatile int* ptr = &tmp3;
    *ptr = 168;
    global_result += *ptr;
}

/* Function with nested conditionals */
__attribute__((optimize("O2"), noinline))
void test_function5(void) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int result = 0;
    
    /* Complex pre-jump computation using distinct resources */
    result = ext_func1(a) + ext_func2(b);
    
    /* Multiple nested conditions */
    if (cond1) {
        if (cond2) {
            if (cond3) {
                goto deep_label;
            } else {
                result += c;
            }
        } else {
            result += d;
        }
    }
    
    /* Intermediate basic block */
    ext_func3(result);
    
    /* Another jump opportunity */
    if (cond4) {
        goto shallow_label;
    }
    
    result = result * 2;
    
deep_label:
    /* Simple instruction with no resource conflict */
    volatile int x = 500, y = 300;
    volatile int diff = x - y;  /* Different resources than parent */
    global_result += diff;
    return;
    
shallow_label:
    /* Another simple instruction */
    volatile int prod = 6 * 7;  /* Constant expression */
    global_result += prod;
}

/* Main function that calls all test functions */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;  /* Make some jumps taken */
    cond2 = 1;
    cond3 = 0;
    cond4 = 1;
    
    /* Call test functions multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        test_function1();
        test_function2();
        test_function3();
        test_function4();
        test_function5();
        
        /* Vary conditions */
        cond1 = !cond1;
        cond3 = i & 1;
    }
    
    /* Print result to ensure execution */
    printf("Final result: %d\n", global_result);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
