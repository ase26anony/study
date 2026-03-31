/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables for control flow */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource set for parent instruction */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        /* This goto creates a simplejump_p */
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    z = x * y;
    ext_func2(z);
    
    /* Dead code that won't be executed but prevents optimization */
    if (cond2) {
        a = b + c;
    }
    
target_label1:
    /* Non-jump, non-sequence instruction at jump target */
    /* This is next_trial - must not reference/set parent's resources */
    c = x + y;  /* Uses different variables than parent instruction */
    
    /* Continue execution */
    result += c;
    ext_func3(result);
}

/* More complex pattern with switch statement */
__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int i = 0, j = 0, k = 0;
    volatile int m = 7, n = 8, p = 0;
    
    for (i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                ext_func1(m);
                if (cond3) {
                    goto target_label2;
                }
                j = m * 2;
                break;
                
            case 1:
                /* Different computation */
                k = n + 5;
                ext_func2(k);
                if (cond4) {
                    goto target_label2;
                }
                break;
                
            case 2:
                /* Direct jump to label */
                goto target_label2;
                
            default:
                break;
        }
        
        /* Some intermediate computation */
        p = i + j;
        continue;
        
    target_label2:
        /* Target instruction - simple arithmetic with no traps */
        /* Uses completely different variables than any parent computation */
        volatile int temp = 15;
        temp = temp + 3;  /* Safe addition, no division */
        
        result += temp;
        break;  /* Exit loop after hitting label */
    }
}

/* Function with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    volatile int a = 100, b = 200;
    volatile int r1 = 0, r2 = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&label1, &&label2, &&target_label3 };
    
    /* Resource setup */
    ext_func1(a);
    r1 = a * 2;
    
    /* Conditional that may or may not jump */
    if (cond1 || cond3) {
        /* Use computed goto to reach target */
        goto *labels[2];
    }
    
    /* Alternative path */
    r2 = b / 2;  /* Division but with constant divisor - safe */
    goto *labels[1];
    
label1:
    r1 += 10;
    goto end;
    
label2:
    r2 += 20;
    goto end;
    
target_label3:
    /* Target instruction for delay slot filling */
    /* Simple assignment with no resource conflict */
    volatile int local = 42;
    local = local + 1;  /* Safe increment */
    
    result += local;
    
end:
    /* Memory clobber to prevent optimization */
    __asm__ volatile ("" : : : "memory");
}

/* Function with inline assembly to create artificial resource constraints */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int x = 1, y = 2, z = 3;
    volatile int res = 0;
    
    /* Inline assembly creates resource usage pattern */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (res)
        : "r" (x), "r" (y)
        : "%eax", "memory"
    );
    
    /* Conditional jump */
    if (res > 0) {
        ext_func2(res);
        goto target_label4;
    }
    
    /* Alternative computation */
    z = x * y * 10;
    
target_label4:
    /* Target instruction - uses completely separate resources */
    volatile int safe_var = 99;
    safe_var = safe_var - 1;  /* Simple subtraction, no traps */
    
    result += safe_var;
    
    /* Another memory barrier */
    __asm__ volatile ("" : : : "memory");
}

/* Nested loop pattern */
__attribute__((optimize("O3")))
void test_pattern5(void) {
    volatile int i, j, k;
    volatile int sum = 0;
    
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            /* Different conditions for jumping */
            if ((i + j) % 2 == 0) {
                ext_func3(i);
                if (cond1) {
                    goto target_label5;
                }
            }
            
            k = i * j + 10;
            sum += k;
            
            /* Continue inner loop */
            continue;
            
        target_label5:
            /* Target instruction in nested context */
            volatile int inner_temp = 50;
            inner_temp = inner_temp + i;  /* Simple addition */
            
            result += inner_temp;
            
            /* Break out of loops */
            goto outer_break;
        }
    }
outer_break:
    return;
}

/* Function with multiple potential target labels */
__attribute__((optimize("O2"), noinline))
void test_pattern6(void) {
    volatile int a = 1, b = 2, c = 3;
    volatile int choice = cond1 ? 1 : 2;
    
    /* Multiple jump possibilities */
    switch (choice) {
        case 1:
            ext_func1(a);
            if (b > a) {
                goto target_label6a;
            }
            c = a + b;
            break;
            
        case 2:
            ext_func2(b);
            if (a < b) {
                goto target_label6b;
            }
            c = b - a;
            break;
            
        default:
            c = 0;
            break;
    }
    
    /* Fall through */
    result += c;
    return;
    
target_label6a:
    /* First target */
    volatile int tmp1 = 77;
    tmp1 = tmp1 * 2;  /* Multiplication with no volatile divisor */
    result += tmp1;
    return;
    
target_label6b:
    /* Second target */
    volatile int tmp2 = 88;
    tmp2 = tmp2 / 4;  /* Division by constant - safe */
    result += tmp2;
    return;
}

/* Main function that runs all test patterns */
int main(void) {
    printf("Starting reorg delay slot test patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Run all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    test_pattern6();
    
    /* Print result to ensure execution */
    printf("Accumulated result: %d\n", result);
    printf("Test patterns completed.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
