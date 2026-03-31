/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

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

/* Function with simple jump-to-label pattern */
__attribute__((optimize("O2")))
void test_simple_jump(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource set for parent instruction */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code to avoid block merging */
    x = ext_func2(x);
    
target_label1:
    /* Non-jump, non-sequence instruction - delay slot candidate */
    z = y + x;  /* Simple arithmetic, no trapping */
    
    /* Use result to prevent optimization */
    result += z;
}

/* Function with nested control flow */
__attribute__((optimize("O3")))
void test_nested_control_flow(void) {
    volatile int i, j, k;
    volatile int arr[10] = {0};
    
    for (i = 0; i < 10; i++) {
        switch (i % 3) {
            case 0:
                if (cond2) {
                    goto target_label2;
                }
                arr[i] = ext_func1(i);
                break;
                
            case 1:
                /* Another jump pattern */
                if (cond3) {
                    target_label2:
                    /* Delay slot candidate - simple assignment */
                    k = i * 2;
                    result += k;
                }
                arr[i] = ext_func2(i);
                break;
                
            case 2:
                arr[i] = ext_func3(i);
                break;
        }
    }
}

/* Function with multiple jump patterns */
__attribute__((optimize("O2"), noinline))
void test_multiple_jumps(void) {
    volatile int p = 100, q = 200, r = 0;
    volatile int m = 50, n = 25;
    
    /* First pattern */
    ext_func2(p);
    if (cond1 && !cond4) {
        goto target_a;
    }
    
    m = ext_func1(m);
    
    /* Second pattern */
    if (cond3) {
        target_a:
        /* Simple arithmetic - delay slot candidate */
        r = p - q;
        result += r;
    }
    
    /* Third pattern with different resources */
    volatile int s = 0, t = 0;
    ext_func3(n);
    if (cond2 || cond3) {
        goto target_b;
    }
    
    s = ext_func2(s);
    
target_b:
    /* Another simple operation */
    t = m + n;
    result += t;
}

/* Function using computed goto */
__attribute__((optimize("O2"), noinline))
void test_computed_goto(void) {
    volatile int val = 0;
    static void *labels[] = { &&label1, &&label2, &&label3 };
    
    /* Create resource separation */
    ext_func1(10);
    
    /* Jump using computed goto */
    goto *labels[cond1 ? 0 : 1];
    
label1:
    /* Delay slot candidate - safe operation */
    val = 100 / (cond3 + 1);  /* Division but divisor never zero */
    result += val;
    return;
    
label2:
    val = 200;
    result += val;
    return;
    
label3:
    val = 300;
    result += val;
    return;
}

/* Function with loop and jump to label */
__attribute__((optimize("O3"), noinline))
void test_loop_jump(void) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    while (counter < 5) {
        /* Call external function to create resource barrier */
        ext_func2(counter);
        
        /* Conditional jump inside loop */
        if (counter == 2 && cond1) {
            goto loop_target;
        }
        
        sum += counter;
        counter++;
        continue;
        
    loop_target:
        /* Simple assignment - delay slot candidate */
        sum += 100;
        counter++;
    }
    
    result += sum;
}

/* Function with inline assembly to force specific RTL patterns */
__attribute__((optimize("O2"), noinline))
void test_with_asm(void) {
    volatile int a = 10, b = 20, c = 0;
    
    /* Inline assembly with memory clobber to create resource constraints */
    asm volatile (
        "nop\n\t"
        "nop\n\t"
        : 
        : 
        : "memory"
    );
    
    /* Jump to label */
    if (cond1) {
        goto asm_target;
    }
    
    /* More assembly to prevent optimization */
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    
asm_target:
    /* Simple operation - delay slot candidate */
    c = a + b;
    result += c;
    
    /* More assembly to create resource separation */
    asm volatile (
        "nop\n\t"
        : 
        : 
        : "memory"
    );
}

/* Main test driver */
int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Run all test patterns */
    test_simple_jump();
    test_nested_control_flow();
    test_multiple_jumps();
    test_computed_goto();
    test_loop_jump();
    test_with_asm();
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
