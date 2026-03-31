/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables to prevent optimization */
volatile int g_cond = 0;
volatile int g_a = 1, g_b = 2, g_c = 3, g_d = 4;
volatile int g_result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int local_cond = g_cond;
    volatile int x = g_a, y = g_b;
    int temp;
    
    /* Call external function to create resource separation */
    temp = ext_func1(x);
    
    /* Conditional jump to label - must be simple jump */
    if (local_cond) {
        goto target_label1;
    }
    
    /* Some intermediate code to avoid fall-through optimization */
    x = y + temp;
    y = ext_func2(x);
    
    /* Target label with simple non-jump instruction */
    target_label1:
    /* Simple arithmetic - no jumps, no sequences, no trapping */
    g_result += g_c + g_d;  /* Candidate for delay slot */
    
    /* More code after label */
    x = g_a * g_b;
    g_result += x;
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int cond1 = g_cond;
    volatile int cond2 = g_b;
    volatile int a = g_a, b = g_b, c = g_c;
    
    /* Nested control flow */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                if (cond1) {
                    goto target_label2;
                }
                break;
            case 1:
                if (cond2) {
                    /* Another path to same label */
                    goto target_label2;
                }
                break;
            default:
                a = ext_func3(b);
        }
        
        /* Intermediate computation with different resources */
        b = a + c;
        ext_func1(b);
        
        /* Target label inside loop */
        target_label2:
        /* Simple assignment - delay slot candidate */
        c = a - b;  /* Uses different variables than parent insn */
        
        /* Continue loop */
        a = b + i;
    }
    
    g_result += c;
}

/* Function with computed goto for variety */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    static void* labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int selector = g_cond & 0x3;
    volatile int x = g_a, y = g_b, z = g_c;
    
    /* Resource separation */
    ext_func2(x);
    
    /* Computed goto - may create different jump pattern */
    goto *labels[selector];
    
    label_a:
    x = y + 1;
    goto end;
    
    label_b:
    /* Target with simple operation */
    z = x * 2;  /* Delay slot candidate */
    goto end;
    
    label_c:
    y = z - 1;
    goto end;
    
    end:
    g_result += x + y + z;
}

/* Function with inline assembly to create artificial resource constraints */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int cond = g_cond;
    volatile int a = g_a, b = g_b;
    int res;
    
    /* Inline assembly creates memory clobber - affects resource analysis */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (res)
        : "r" (a), "r" (b)
        : "%eax", "memory"
    );
    
    /* Conditional jump after assembly */
    if (cond) {
        goto target_label4;
    }
    
    /* Different computation path */
    a = ext_func1(b);
    
    target_label4:
    /* Simple operation with no resource conflict with parent */
    b = res + 1;  /* Delay slot candidate */
    
    g_result += b;
}

/* Function with multiple jump-to-label patterns */
__attribute__((optimize("O2"), noinline))
void test_pattern5(void) {
    volatile int cond1 = g_a;
    volatile int cond2 = g_b;
    volatile int x = 0, y = 0, z = 0;
    
    /* First pattern */
    if (cond1) {
        goto target1;
    }
    
    x = ext_func2(g_c);
    
    target1:
    y = g_d + 5;  /* Candidate 1 */
    
    /* Second pattern with different resources */
    if (cond2) {
        goto target2;
    }
    
    z = ext_func3(x);
    
    target2:
    x = y * 2;  /* Candidate 2 */
    
    /* Third pattern in loop */
    for (int i = 0; i < 2; i++) {
        volatile int loop_cond = g_cond + i;
        
        if (loop_cond) {
            goto target3;
        }
        
        y = ext_func1(z);
        
        target3:
        z = x + i;  /* Candidate 3 */
    }
    
    g_result += x + y + z;
}

/* Function that avoids trapping instructions at target */
__attribute__((optimize("O2")))
void test_safe_target(void) {
    volatile int cond = g_cond;
    volatile int safe_a = 10, safe_b = 20;  /* Non-zero, safe values */
    
    /* Call external function for resource separation */
    ext_func1(safe_a);
    
    if (cond) {
        goto safe_target;
    }
    
    safe_a = ext_func2(safe_b);
    
    safe_target:
    /* Safe operation: no division, no memory access that could trap */
    safe_b = safe_a + 5;  /* Addition cannot trap */
    
    g_result += safe_b;
}

/* Main function to execute all test patterns */
int main(void) {
    /* Initialize volatile control variables */
    g_cond = 1;  /* Force some branches taken */
    g_a = 10;
    g_b = 20;
    g_c = 30;
    g_d = 40;
    
    /* Execute all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    test_safe_target();
    
    /* Print result to ensure execution */
    printf("Result: %d\n", g_result);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
