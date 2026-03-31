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

/* Function with jump-to-label pattern for delay slot candidate */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource usage before jump */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        /* This goto creates a simplejump_p */
        goto target_label1;
    }
    
    /* Some intermediate code */
    z = x * y;
    ext_func2(z);
    
    /* Return early to avoid executing target without jump */
    if (cond2) {
        return;
    }
    
target_label1:
    /* This is next_trial - simple non-jump, non-sequence instruction */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    c = a + b;  /* Simple arithmetic, no traps */
    
    /* Use result to prevent optimization */
    result += c;
}

/* More complex pattern with nested control flow */
__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int i, j, k;
    volatile int arr[10] = {0};
    
    for (i = 0; i < 10; i++) {
        /* Multiple conditional jumps */
        if (cond3 && (i % 2 == 0)) {
            ext_func1(i);
            goto compute_label;
        }
        
        if (cond4) {
            arr[i] = i * 2;
            continue;
        }
        
        switch (i % 3) {
            case 0:
                j = i + 1;
                break;
            case 1:
                /* Another jump to label */
                if (cond1) {
                    goto compute_label;
                }
                j = i - 1;
                break;
            default:
                j = i * 2;
        }
        
        /* Continue loop */
        arr[i] = j;
        continue;
        
    compute_label:
        /* Target label instruction - safe, non-trapping */
        k = i * 3 + 1;  /* Multiplication is safe with integers */
        result += k;
        arr[i] = k;
    }
}

/* Function with computed goto */
__attribute__((optimize("O2"), __noinline__))
void test_pattern3(void) {
    volatile int m = 7, n = 13, p = 0;
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Resource usage */
    ext_func2(m);
    
    /* Computed goto */
    goto *labels[cond1 ? 0 : 1];
    
label_a:
    /* Intermediate code that won't be reached directly */
    p = m / 2;  /* Safe division, divisor is constant */
    goto end;
    
label_b:
    /* Target for jump - simple assignment */
    p = n - m;  /* Simple subtraction */
    result += p;
    goto end;
    
label_c:
    p = m + n;
    goto end;
    
end:
    ext_func3(p);
}

/* Function with multiple basic blocks and labels */
__attribute__((optimize("O2"), __noinline__))
int test_pattern4(int seed) {
    volatile int r1 = seed, r2 = seed * 2, r3 = 0;
    volatile int counter = 0;
    
    /* Inline assembly to create artificial resource constraints */
    asm volatile ("" : : : "memory");
    
    while (counter++ < 5) {
        /* Complex condition to prevent optimization */
        if ((r1 ^ r2) & 0xF) {
            ext_func1(r1);
            
            /* Jump to label */
            if (cond1 || cond3) {
                goto process_data;
            }
        }
        
        /* Alternative path */
        r3 = r1 | r2;
        ext_func2(r3);
        continue;
        
    process_data:
        /* Target instruction - memory operation */
        r3 = r1 & r2;  /* Bitwise operation, no traps */
        result += r3;
        
        /* External call after target */
        ext_func3(r3);
    }
    
    return r3;
}

/* Function specifically designed for MIPS delay slots */
__attribute__((optimize("O2"), __noinline__))
void test_mips_delay_slot(void) {
    volatile int val1 = 100, val2 = 200, val3 = 0;
    volatile int *ptr = &val3;
    
    /* Multiple jumps to create opportunities for delay slot filling */
    if (cond1) {
        ext_func1(val1);
        
        /* This should be a simple conditional jump */
        if (val1 > 50) {
            goto do_calc;
        }
    }
    
    /* Different path */
    val3 = val1 * 2;
    ext_func2(val3);
    return;
    
do_calc:
    /* Target: simple store operation */
    *ptr = val1 + val2;  /* Memory store, no traps */
    
    /* Use result */
    result += *ptr;
}

/* Main test driver */
int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = rand() & 1;
    cond2 = rand() & 1;
    cond3 = rand() & 1;
    cond4 = rand() & 1;
    
    /* Run all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4(42);
    test_mips_delay_slot();
    
    /* Additional iterations with different conditions */
    for (int i = 0; i < 3; i++) {
        cond1 = i & 1;
        cond3 = (i >> 1) & 1;
        test_pattern1();
        test_pattern4(i * 10);
    }
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x / 3; }
