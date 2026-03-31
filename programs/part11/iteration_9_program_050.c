/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables to prevent optimization */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 0;
volatile int cond4 = 1;
volatile int result = 0;
volatile int x = 5, y = 10, z = 15;
volatile int a = 1, b = 2, c = 3, d = 4;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int local_cond = cond1;
    volatile int temp1 = x;
    volatile int temp2 = y;
    
    /* Call external function to create resource barrier */
    ext_func1(temp1);
    
    /* Conditional jump to label */
    if (local_cond) {
        goto target_label1;
    }
    
    /* Some intermediate computation */
    temp1 = temp1 + temp2;
    ext_func2(temp1);
    
    /* The target label with simple non-jump instruction */
    target_label1:
    /* Simple arithmetic that doesn't conflict with parent instruction resources */
    z = a + b;  /* Uses different volatile variables than parent */
    
    /* More computation to prevent tail merging */
    result += z;
    ext_func3(result);
}

__attribute__((optimize("O2")))
void test_pattern2(void) {
    volatile int local_cond = cond2;
    volatile int m = 7, n = 8;
    volatile int p = 9, q = 10;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    /* Jump pattern inside a loop */
    for (int i = 0; i < 3; i++) {
        ext_func1(i);
        
        if (local_cond && (i % 2 == 0)) {
            goto target_label2;
        }
        
        m = m * 2;
        __asm__ volatile ("" : : : "memory");
        
        target_label2:
        /* Simple assignment with non-overlapping resources */
        n = p - q;  /* Different variables than used before jump */
        
        result += n;
    }
}

__attribute__((optimize("O3")))
void test_pattern3(void) {
    volatile int local_cond = cond3;
    volatile int r1 = 100, r2 = 200;
    volatile int s1 = 300, s2 = 400;
    
    /* Switch statement with jump-to-label pattern */
    switch (local_cond) {
        case 0:
            ext_func2(r1);
            if (r1 > 50) {
                goto target_label3;
            }
            r2 = r1 * 2;
            break;
        case 1:
            ext_func3(r2);
            if (r2 < 500) {
                goto target_label3;
            }
            r1 = r2 / 2;
            break;
        default:
            break;
    }
    
    /* Some intermediate code */
    __asm__ volatile ("" : : : "memory");
    
    target_label3:
    /* Safe, non-trapping operation */
    s1 = s2 + 5;  /* No division, uses different variables */
    
    result += s1;
}

/* Function with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern4(void) {
    static const void* labels[] = { &&label1, &&label2, &&label3 };
    volatile int idx = cond4 ? 0 : 1;
    volatile int u = 20, v = 30;
    volatile int w = 40;
    
    ext_func1(u);
    
    /* Computed goto */
    goto *labels[idx];
    
    label1:
    v = u + 10;
    goto end;
    
    label2:
    /* Target for delay slot candidate */
    w = v - 5;  /* Simple arithmetic */
    goto end;
    
    label3:
    v = w * 2;
    goto end;
    
    end:
    result += v + w;
    
    /* Memory barrier */
    __asm__ volatile ("" : : : "memory");
}

/* Nested control flow pattern */
__attribute__((optimize("O2")))
void test_pattern5(void) {
    volatile int local_cond1 = cond1;
    volatile int local_cond2 = cond2;
    volatile int t1 = 50, t2 = 60;
    volatile int u1 = 70, u2 = 80;
    
    if (local_cond1) {
        for (int j = 0; j < 2; j++) {
            ext_func2(j);
            
            while (local_cond2 && j < 1) {
                /* Complex nested pattern */
                if (t1 > 40) {
                    goto target_label5;
                }
                t2 = t1 + j;
                break;
            }
            
            /* Continue after loop */
            u1 = t2 * 3;
        }
    } else {
        ext_func3(t1);
    }
    
    /* Some code in between */
    __asm__ volatile ("" : : : "memory");
    
    target_label5:
    /* Simple, safe operation */
    u2 = u1 - 10;  /* No resource conflict with parent */
    
    result += u2;
}

/* Function that mimics delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_like_delay_slot(void) {
    volatile int flag = cond3;
    volatile int val1 = 1000, val2 = 2000;
    volatile int res1 = 0, res2 = 0;
    
    /* Multiple external calls to separate resources */
    val1 = ext_func1(val1);
    __asm__ volatile ("" : : : "memory");
    
    /* Conditional jump */
    if (flag) {
        goto mips_target;
    }
    
    /* Parent instruction computation */
    res1 = val1 * 2;
    ext_func2(res1);
    
    mips_target:
    /* Candidate for delay slot - uses completely different resources */
    res2 = val2 + 777;  /* val2 not used before, res2 not set before */
    
    /* Ensure result is used */
    result += res1 + res2;
    __asm__ volatile ("" : : : "memory");
}

/* Dummy external function definitions */
int ext_func1(int arg) {
    return arg + 1;
}

int ext_func2(int arg) {
    return arg * 2;
}

int ext_func3(int arg) {
    return arg - 1;
}

int main(void) {
    /* Initialize volatile control variables */
    cond1 = rand() % 2;
    cond2 = rand() % 2;
    cond3 = rand() % 2;
    cond4 = rand() % 2;
    
    printf("Starting reorg pattern tests...\n");
    
    /* Execute all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    test_mips_like_delay_slot();
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
