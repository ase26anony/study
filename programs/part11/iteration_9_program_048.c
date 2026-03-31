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

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int x = 0, y = 0, z = 0;
    
    /* Create resource usage before jump */
    x = a + b;
    ext_func1(x);
    
    /* Conditional jump to label */
    if (cond1) {
        /* This goto creates a simplejump_p */
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = c - d;
    ext_func2(y);
    
    /* This should not be reached when cond1 is true */
    z = x * y;
    
target_label1:
    /* This is next_trial - simple non-jump, non-sequence instruction */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    volatile int r1 = b + c;  /* Simple arithmetic, no traps */
    result += r1;
    
    /* More code after label */
    ext_func3(r1);
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int p = 0, q = 0, r = 0;
    volatile int counter = 0;
    
    /* Loop with nested control flow */
    for (counter = 0; counter < 3; counter++) {
        /* Switch statement to create complex control flow */
        switch (counter) {
            case 0:
                p = a * 2;
                break;
            case 1:
                q = b / 2;  /* Safe division by constant */
                break;
            default:
                r = c + d;
        }
        
        /* Multiple conditional jumps */
        if (cond2) {
            goto target_label2;
        }
        
        if (cond3) {
            /* Another jump opportunity */
            ext_func1(p);
            goto alternate_label;
        }
        
        continue;
        
    alternate_label:
        q = r + p;
        ext_func2(q);
        
        /* Jump to the target label */
        if (cond4) {
            goto target_label2;
        }
    }
    
    /* Some code that might be skipped */
    p = q * r;
    
target_label2:
    /* Candidate for delay slot filling */
    /* Simple assignment with no resource conflicts */
    volatile int r2 = d - a;  /* Safe subtraction */
    result += r2;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
}

/* Function with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int selector = 1;
    volatile int m = 0, n = 0;
    
    /* Resource usage in parent block */
    m = ext_func1(a);
    n = ext_func2(b);
    
    /* Computed goto */
    goto *labels[selector];
    
label_a:
    m = m + 1;
    goto end;
    
label_b:
    /* Target label with simple instruction */
    volatile int r3 = c * 2;  /* Simple multiplication */
    result += r3;
    
    /* Followed by external call */
    ext_func3(r3);
    goto end;
    
label_c:
    n = n - 1;
    goto end;
    
end:
    return;
}

/* Function with nested if-else chains */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int i = 0, j = 0, k = 0;
    
    /* Complex control flow */
    if (cond1) {
        if (cond2) {
            j = a + b;
            ext_func1(j);
        } else {
            k = c - d;
            if (cond3) {
                goto target_label4;
            }
            ext_func2(k);
        }
    } else {
        i = b * c;
    }
    
    /* More intermediate code */
    for (int idx = 0; idx < 2; idx++) {
        j = j + idx;
        if (cond4 && idx == 1) {
            goto target_label4;
        }
    }
    
    /* Unreachable when jumps are taken */
    k = i * j;
    
target_label4:
    /* Simple non-trapping instruction */
    volatile int r4 = d / 4;  /* Safe division by constant 4 */
    result += r4;
    
    /* Memory barrier */
    __asm__ volatile ("" : : : "memory");
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int reg1 = 0, reg2 = 0, reg3 = 0;
    
    /* Simulate register operations */
    reg1 = a;
    reg2 = b;
    
    /* Conditional branch to label */
    if (reg1 < reg2) {
        /* This creates a conditional jump instruction */
        goto mips_target;
    }
    
    /* Alternative path */
    reg3 = ext_func1(reg1);
    goto mips_end;
    
mips_target:
    /* This instruction could be placed in delay slot */
    /* Uses completely different resource set */
    volatile int r5 = c + 100;  /* Simple addition */
    result += r5;
    
    /* External call creates resource boundary */
    ext_func2(r5);
    
mips_end:
    /* Final computation */
    reg1 = reg2 + reg3;
}

/* External function definitions (simulated) */
int ext_func1(int x) {
    /* Memory clobber to affect resource analysis */
    __asm__ volatile ("" : : : "memory");
    return x + 1;
}

int ext_func2(int x) {
    __asm__ volatile ("" : : : "memory");
    return x * 2;
}

int ext_func3(int x) {
    __asm__ volatile ("" : : : "memory");
    return x - 1;
}

int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;  /* Will take the jump */
    cond2 = 0;  /* Will not take the jump */
    cond3 = 1;  /* Will take the jump */
    cond4 = 1;  /* Will take the jump */
    
    /* Run all test patterns */
    test_pattern1();
    printf("Pattern 1 executed, result = %d\n", result);
    
    test_pattern2();
    printf("Pattern 2 executed, result = %d\n", result);
    
    test_pattern3();
    printf("Pattern 3 executed, result = %d\n", result);
    
    test_pattern4();
    printf("Pattern 4 executed, result = %d\n", result);
    
    test_mips_pattern();
    printf("MIPS pattern executed, result = %d\n", result);
    
    printf("All tests completed. Final result = %d\n", result);
    
    return 0;
}
