/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 2;
volatile int result = 0;

/* Function with simple jump-to-label pattern */
__attribute__((optimize("O2")))
void test_simple_jump(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource separation */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate computation */
    z = x * y;
    ext_func2(z);
    
    /* Avoid falling through to label */
    if (cond2) {
        x = y + 1;
    }
    
    /* Target label with simple non-jump instruction */
    target_label1:
    /* Simple arithmetic - delay slot candidate */
    c = a + b;  /* Should not reference/set parent insn resources */
    
    /* Use result to prevent optimization */
    result += c;
}

/* Function with nested control flow */
__attribute__((optimize("O3")))
void test_nested_control_flow(void) {
    volatile int i, j, k;
    volatile int arr[10] = {0};
    
    for (i = 0; i < 10; i++) {
        switch (i % 3) {
            case 0:
                if (cond1) {
                    ext_func1(i);
                    goto case_label;
                }
                j = i * 2;
                break;
                
            case 1:
                if (cond2) {
                    /* Inline assembly to create resource constraints */
                    __asm__ volatile ("" : : : "memory");
                    goto case_label;
                }
                k = i + 5;
                break;
                
            case 2:
            case_label:
                /* Simple assignment - delay slot candidate */
                arr[i] = i * 3;  /* Non-trapping, non-jump instruction */
                break;
        }
        
        /* External call for resource separation */
        ext_func2(arr[i]);
    }
    
    /* Accumulate results */
    for (i = 0; i < 10; i++) {
        result += arr[i];
    }
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_computed_goto(void) {
    volatile int val = 0;
    static void *labels[] = { &&label0, &&label1, &&label2 };
    
    /* Resource separation */
    ext_func3(cond3);
    
    /* Conditional jump */
    if (val < 5) {
        goto *labels[val % 3];
    }
    
    /* Some computation */
    val = ext_func1(val);
    
    label0:
    /* Simple subtraction - delay slot candidate */
    val = 100 - 50;  /* Safe, non-trapping operation */
    goto end;
    
    label1:
    /* Another simple operation */
    val = 200 / 2;   /* Division by constant - safe */
    goto end;
    
    label2:
    /* Memory operation */
    volatile int temp = 42;
    val = temp;
    
    end:
    result += val;
}

/* Function with loop and multiple labels */
__attribute__((optimize("O2"), __noinline__))
void test_loop_with_labels(int iterations) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    while (counter < iterations) {
        /* Create varying conditions */
        volatile int local_cond = counter % 4;
        
        /* Multiple jump patterns */
        if (local_cond == 0) {
            ext_func1(counter);
            goto loop_label1;
        } else if (local_cond == 1) {
            __asm__ volatile ("" : : : "memory");
            goto loop_label2;
        } else if (local_cond == 2) {
            if (cond3) {
                goto loop_label3;
            }
        }
        
        /* Default computation */
        sum += counter * 2;
        counter++;
        continue;
        
        loop_label1:
        /* Simple increment - delay slot candidate */
        sum += 1;
        counter++;
        continue;
        
        loop_label2:
        /* Simple assignment */
        sum = sum + counter;
        counter++;
        continue;
        
        loop_label3:
        /* Safe arithmetic */
        sum += counter - 1;
        counter++;
        continue;
    }
    
    result += sum;
}

/* Function with mixed operations around target label */
__attribute__((optimize("O3"), __noinline__))
void test_mixed_operations(void) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int *ptr = &a;
    
    /* Multiple external calls for resource separation */
    ext_func1(a);
    ext_func2(b);
    
    /* Complex condition */
    if ((a < b) && (c > d) || cond1) {
        __asm__ volatile ("" : : : "memory");
        goto mixed_label;
    }
    
    /* Intermediate computations using different variables */
    for (int i = 0; i < 3; i++) {
        *ptr = ext_func3(i);
        ptr = &b;
    }
    
    mixed_label:
    /* Simple, safe operation at target - delay slot candidate */
    d = a + b + c;  /* Uses different vars than parent insn might use */
    
    /* More operations to prevent tail merging */
    ext_func1(d);
    ext_func2(result);
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;  /* Make some jumps taken */
    cond2 = 0;  /* Make some not taken */
    cond3 = 2;
    
    /* Run test functions */
    test_simple_jump();
    test_nested_control_flow();
    test_computed_goto();
    test_loop_with_labels(5);
    test_mixed_operations();
    
    /* Print result to ensure execution */
    volatile int print_me = result;
    
    /* Use result to prevent dead code elimination */
    return print_me > 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
