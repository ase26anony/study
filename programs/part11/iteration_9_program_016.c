/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int x = 0, y = 0, z = 0;
    volatile int local_cond = cond1;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* Parent instruction for delay slot */
    
    /* Call external function to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (local_cond) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = c * 2;
    ext_func2(y);
    
    /* This should be the delay slot candidate */
    target_label1:
    /* Simple non-jump, non-trapping instruction */
    z = d - 3;  /* next_trial candidate */
    
    /* Use result to prevent dead code elimination */
    result += z;
    
    /* More code after label */
    ext_func3(z);
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int p = 0, q = 0, r = 0;
    volatile int local_cond = cond2;
    
    /* Different computation for parent instruction */
    p = b * a;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    /* Nested control flow */
    for (int i = 0; i < 3; i++) {
        if (local_cond) {
            goto target_label2;
        }
        
        /* Alternate path */
        q = ext_func1(i);
        
        if (i == 1) {
            target_label2:
            /* Another delay slot candidate */
            r = c + d;  /* Simple addition, non-trapping */
            result += r;
            
            /* External call after target */
            ext_func2(r);
        }
    }
}

__attribute__((optimize("O2"), noinline))
void test_function3(void) {
    volatile int m = 0, n = 0;
    volatile int local_cond = cond3;
    
    /* Switch statement for varied control flow */
    switch (a) {
        case 5:
            m = b + 10;
            ext_func1(m);
            
            /* Jump to label from within switch */
            if (local_cond) {
                goto target_label3;
            }
            break;
        case 10:
            m = c - 5;
            break;
        default:
            m = 1;
    }
    
    /* Some intermediate computation */
    n = ext_func2(m);
    
    /* This should not be reached if jump is taken */
    m = m * 2;
    
    target_label3:
    /* Delay slot candidate - simple assignment */
    n = d;  /* Just assignment, no computation */
    result += n;
    
    /* Prevent tail call optimization */
    ext_func3(n);
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function4(void) {
    volatile int u = 0, v = 0, w = 0;
    volatile int local_cond = cond4;
    
    /* Parent instruction */
    u = a * b + c;
    
    /* Labels for computed goto */
    void* labels[] = { &&label1, &&label2, &&target_label4 };
    
    /* External call for resource separation */
    ext_func1(u);
    
    /* Conditional jump using computed goto */
    if (local_cond) {
        goto *labels[2];  /* Jump to target_label4 */
    }
    
    /* Alternative paths */
    label1:
    v = ext_func2(u);
    goto end;
    
    label2:
    w = ext_func3(v);
    goto end;
    
    target_label4:
    /* Delay slot candidate - subtraction */
    v = c - a;  /* Simple, non-trapping operation */
    result += v;
    
    /* More operations after */
    w = ext_func1(v);
    
    end:
    return;
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int reg1 = 0, reg2 = 0, reg3 = 0;
    volatile int branch_cond = cond1;
    
    /* Load operations simulating MIPS-like code */
    reg1 = a;
    reg2 = b;
    
    /* Arithmetic operation that could be in delay slot */
    reg3 = reg1 + reg2;
    
    /* External call to separate resources */
    ext_func1(reg3);
    
    /* Conditional branch to label */
    if (branch_cond) {
        goto mips_target;
    }
    
    /* Alternative path with different operations */
    reg1 = ext_func2(reg3);
    reg2 = reg1 * 2;
    
    mips_target:
    /* Instruction immediately after label - good delay slot candidate */
    reg3 = d;  /* Simple register load */
    result += reg3;
    
    /* Following instruction that uses the result */
    ext_func3(reg3);
}

/* Function with loop and multiple jump targets */
__attribute__((optimize("O3")))
void test_loop_pattern(void) {
    volatile int sum = 0;
    volatile int i;
    
    for (i = 0; i < 10; i++) {
        volatile int temp = cond1;
        
        /* Different parent instruction each iteration */
        sum += a + i;
        
        /* Conditional jump inside loop */
        if (temp) {
            goto loop_target;
        }
        
        /* Some computation on the non-jump path */
        ext_func1(i);
        continue;
        
        loop_target:
        /* Delay slot candidate inside loop */
        sum += b;  /* Simple addition */
        result += sum;
        
        /* Continue loop */
        ext_func2(sum);
    }
}

/* Dummy external functions */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 1;
}

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile control variables */
    cond1 = 1;  /* Force first jump to be taken */
    cond2 = 0;  /* Force second jump not taken */
    cond3 = rand() % 2;  /* Random condition */
    cond4 = 1;  /* Force computed goto jump */
    
    /* Run all test functions */
    test_function1();
    test_function2();
    test_function3();
    test_function4();
    test_mips_pattern();
    test_loop_pattern();
    
    printf("Result accumulated: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}
