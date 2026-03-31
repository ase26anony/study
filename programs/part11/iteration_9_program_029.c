/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int result = 0;

/* Function with attribute to force O2 optimization specifically */
__attribute__((optimize("O2"), noinline))
static int test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create artificial resource usage before jump */
    int res1 = ext_func1(a);
    
    /* Conditional jump to label - must remain as simple jump */
    if (cond1) {
        /* Inline assembly to create resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    res1 = ext_func2(b);
    z = x * y;
    
target_label1:
    /* This is next_trial - simple non-jump, non-sequence instruction */
    /* Must not reference/set resources used by parent insn */
    c = a + b;  /* Simple arithmetic with different variables */
    
    /* More code to prevent tail merging */
    result += c;
    return ext_func3(c);
}

/* Different pattern with switch statement */
__attribute__((optimize("O3"), noinline))
static int test_pattern2(int val) {
    volatile int m = 100, n = 200, p = 0;
    volatile int q = 50, r = 25;
    
    /* Call external function to create resource separation */
    int tmp = ext_func2(val);
    
    switch (val % 3) {
        case 0:
            if (cond2) {
                __asm__ volatile ("" : : : "memory");
                goto target_label2;
            }
            break;
        case 1:
            q = ext_func1(r);
            break;
        default:
            break;
    }
    
    /* Some computation */
    p = m - n;
    
    if (cond3) {
        __asm__ volatile ("# dummy" : : : "memory");
        goto skip_label;
    }
    
target_label2:
    /* Candidate for delay slot filling */
    /* Uses completely different variables than parent insn */
    r = q * 2;  /* Simple multiplication */
    
    /* Ensure this isn't optimized away */
    result += r;
    
skip_label:
    return ext_func3(p + r);
}

/* Pattern with computed goto */
__attribute__((optimize("O2"), noinline))
static int test_pattern3(void) {
    volatile int u = 7, v = 13, w = 0;
    static void* labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Resource usage before potential jump */
    int res = ext_func1(u);
    
    /* Conditional that may lead to computed goto */
    if (cond4) {
        goto *labels[res % 3];
    }
    
    /* Normal path */
    w = u + v;
    goto end;
    
label_a:
    /* Simple assignment - delay slot candidate */
    v = u * 2;
    result += v;
    goto end;
    
label_b:
    /* Another simple operation */
    u = v - 5;
    result += u;
    goto end;
    
label_c:
    /* Yet another candidate */
    w = u + v + 10;
    result += w;
    
end:
    return ext_func2(w);
}

/* Pattern with loop and nested conditionals */
__attribute__((optimize("O2"), noinline))
static int test_pattern4(int iterations) {
    volatile int sum = 0;
    volatile int i = 0;
    volatile int temp1 = 8, temp2 = 4;
    
    for (i = 0; i < iterations; i++) {
        /* Varying condition to prevent optimization */
        volatile int inner_cond = (i % 2 == 0);
        
        /* External call creates resource barrier */
        int foo = ext_func3(i);
        
        if (inner_cond) {
            if (cond1) {
                /* Memory barrier to prevent reordering */
                __asm__ volatile ("" : : : "memory");
                goto loop_target;
            }
            temp1 = foo + 1;
        } else {
            temp2 = foo - 1;
        }
        
        /* Some computation */
        sum += temp1 * temp2;
        continue;
        
    loop_target:
        /* Simple operation at jump target - delay slot candidate */
        temp2 = temp1 + 3;  /* Different resources than parent */
        sum += temp2;
    }
    
    result += sum;
    return sum;
}

/* Function with multiple basic blocks and labels */
__attribute__((optimize("O3"), noinline))
static int test_pattern5(int x) {
    volatile int a = x, b = x * 2, c = 0;
    volatile int d = 100, e = 200;
    
    /* Complex control flow */
    if (a > 10) {
        int res = ext_func1(a);
        if (res > 5) {
            __asm__ volatile ("# barrier" : : : "memory");
            goto target_mid;
        }
        c = b + a;
    } else if (a < -5) {
        goto target_end;
    } else {
        d = ext_func2(b);
    }
    
    /* Middle computation */
    e = d * 2;
    
    if (cond3) {
        return e;
    }
    
target_mid:
    /* Simple operation - potential delay slot */
    b = a + 10;  /* Uses different vars than surrounding code */
    result += b;
    
    /* More code to prevent optimization */
    c = ext_func3(b);
    
target_end:
    return c + e;
}

/* Main function that exercises all patterns */
int main(void) {
    int total = 0;
    
    printf("Testing delay slot patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    
    /* Execute all test patterns */
    total += test_pattern1();
    total += test_pattern2(7);
    total += test_pattern3();
    total += test_pattern4(3);
    total += test_pattern5(15);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (accumulated: %d)\n", result, total);
    
    /* Additional test with different conditions */
    cond1 = 0;
    cond2 = 1;
    result = 0;
    
    total += test_pattern1();
    total += test_pattern2(10);
    
    printf("Final result: %d\n", result);
    
    return total > 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
