/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

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

/* Global volatile variables for resource separation */
volatile int gvar1 = 100;
volatile int gvar2 = 200;
volatile int gvar3 = 300;
volatile int gvar4 = 400;
volatile int gvar5 = 500;

/* Function with jump-to-label pattern for delay slot candidate */
__attribute__((optimize("O2")))
int test_pattern1(int x) {
    volatile int local_cond = cond1;
    volatile int a = gvar1;
    volatile int b = gvar2;
    volatile int c = gvar3;
    
    /* Call external function to create resource barrier */
    int ext_result = ext_func1(x);
    
    /* Parent instruction computation - uses specific resources */
    int parent_val = a * b + ext_result;
    
    /* Conditional jump to label */
    if (local_cond) {
        /* Inline assembly to prevent optimization */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    c = ext_func2(c);
    
target_label1:
    /* Delay slot candidate: Simple arithmetic with non-overlapping resources */
    /* Uses different variables than parent instruction */
    volatile int d = gvar4;
    volatile int e = gvar5;
    int candidate_val = d + e;  /* Simple, non-trapping operation */
    
    /* Another external call after label */
    candidate_val = ext_func3(candidate_val);
    
    return parent_val + candidate_val;
}

/* Second pattern with different control flow */
__attribute__((optimize("O3")))
int test_pattern2(int x) {
    volatile int local_cond = cond2;
    volatile int m = gvar1 + 10;
    volatile int n = gvar2 + 20;
    
    /* Loop with nested jumps */
    for (int i = 0; i < 3; i++) {
        volatile int loop_cond = (i & 1);
        
        switch (i) {
            case 0:
                ext_func1(i);
                if (loop_cond) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            case 1:
                if (local_cond) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            default:
                break;
        }
        
        /* Continue normal execution */
        n = m + i;
    }
    
    /* Fallback return if no jump taken */
    return x + 1;

target_label2:
    /* Another delay slot candidate */
    volatile int p = gvar3;
    volatile int q = gvar4;
    int simple_op = p - q;  /* Subtraction is safe */
    
    /* Use result to prevent dead code elimination */
    result += simple_op;
    
    return x + simple_op;
}

/* Third pattern using computed goto */
__attribute__((optimize("O2")))
int test_pattern3(int x) {
    volatile int local_cond = cond3;
    volatile int r = gvar1;
    volatile int s = gvar2;
    
    /* Labels for computed goto */
    void* labels[] = { &&label_a, &&label_b, &&target_label3 };
    
    /* Resource-intensive parent computation */
    int parent_compute = 0;
    for (int j = 0; j < 10; j++) {
        parent_compute += r * j;
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Conditional computed goto */
    if (local_cond) {
        goto *labels[2];  /* Jump to target_label3 */
    }
    
    /* Alternative path */
    s = ext_func2(s);
    goto label_a;
    
label_a:
    r = r + 1;
    goto label_b;
    
label_b:
    s = s - 1;
    return parent_compute;

target_label3:
    /* Safe delay slot candidate - assignment only */
    volatile int t = gvar5;
    volatile int u = 42;
    int safe_val = u;  /* Simple assignment, no trapping */
    
    /* Multiple safe operations */
    safe_val += 10;
    safe_val *= 2;
    
    return parent_compute + safe_val;
}

/* Fourth pattern with more complex surrounding blocks */
__attribute__((optimize("O1")))
int test_pattern4(int x) {
    volatile int local_cond = cond4;
    volatile int v1 = gvar1;
    volatile int v2 = gvar2;
    volatile int v3 = gvar3;
    
    /* Multiple basic blocks before jump */
    if (x > 0) {
        v1 = ext_func1(v1);
        if (x > 100) {
            v2 = ext_func2(v2);
            if (local_cond) {
                __asm__ volatile ("" : : : "memory");
                goto target_label4;
            }
            v3 = v1 + v2;
        } else {
            v3 = v1 - v2;
        }
    } else {
        v1 = v2 * 2;
    }
    
    /* Intermediate computation */
    int mid_val = v1 + v3;
    mid_val = ext_func3(mid_val);
    
    /* Another conditional jump opportunity */
    if (mid_val > 0 && local_cond) {
        __asm__ volatile ("" : : : "memory");
        goto target_label4;
    }
    
    return mid_val;

target_label4:
    /* Very simple delay slot candidate */
    volatile int v4 = gvar4;
    volatile int v5 = gvar5;
    int final_op = v4;  /* Just use v4, no computation */
    
    /* Ensure no trapping - avoid division by volatile zero */
    if (v5 != 0) {
        final_op += 1;  /* Safe increment */
    }
    
    result += final_op;
    return final_op;
}

/* Fifth pattern specifically for MIPS-like delay slots */
__attribute__((optimize("O2"), noinline))
int test_pattern5(int x) {
    volatile int local_cond = cond1 | cond2;
    volatile int w1 = gvar1;
    volatile int w2 = gvar2;
    
    /* Create artificial register pressure */
    int r1 = w1 + 1;
    int r2 = w2 + 2;
    int r3 = r1 * r2;
    int r4 = r3 - x;
    int r5 = r4 / 3;  /* Safe division by constant */
    
    /* Multiple external calls to separate resources */
    ext_func1(r1);
    ext_func2(r2);
    
    /* The critical jump pattern */
    if (local_cond) {
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto target_label5;
    }
    
    /* Alternative path with different resource usage */
    r5 = ext_func3(r5);
    return r5;

target_label5:
    /* Ideal delay slot candidate:
       - Uses completely different variables (w3, w4 vs w1, w2)
       - Simple addition (non-trapping)
       - No overlap with parent instruction's resources */
    volatile int w3 = gvar3;
    volatile int w4 = gvar4;
    int delay_candidate = w3 + w4 + 5;  /* Constant offset is safe */
    
    /* Store result to prevent elimination */
    result = delay_candidate;
    
    return delay_candidate;
}

/* Main function to execute all patterns */
int main() {
    int total = 0;
    
    printf("Testing delay slot patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 1;
    
    /* Execute all test patterns */
    total += test_pattern1(10);
    total += test_pattern2(20);
    total += test_pattern3(30);
    total += test_pattern4(40);
    total += test_pattern5(50);
    
    /* Use results to prevent optimization */
    printf("Total result: %d\n", total);
    printf("Global result: %d\n", result);
    
    /* Additional volatile operations to maintain control flow */
    volatile int final_check = total + result;
    if (final_check > 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
