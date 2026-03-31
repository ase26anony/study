/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int d = 5, e = 15;
    
    /* Create resource set for parent instruction */
    int parent_res = ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code to create basic block boundaries */
    c = ext_func2(b);
    
target_label1:
    /* Non-jump, non-sequence instruction at jump target */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    d = e + 25;  /* Simple arithmetic, non-trapping */
    
    /* More code to prevent tail merging */
    result += d + parent_res;
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int x = 100, y = 200, z = 0;
    volatile int p = 50, q = 75;
    
    /* Loop with nested conditional jumps */
    for (int i = 0; i < 3; i++) {
        int loop_res = ext_func2(x + i);
        
        /* Switch statement to create complex control flow */
        switch (i) {
            case 0:
                if (cond2) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            case 1:
                if (cond3) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            default:
                if (cond4) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
        }
        
        /* Alternative path */
        z = ext_func3(y);
        continue;
        
    target_label2:
        /* Safe arithmetic at target label */
        p = q - 10;  /* Subtraction, non-trapping */
        
        /* External call after label for resource separation */
        int post_label = ext_func1(p);
        result += post_label + loop_res;
    }
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    volatile int m = 1000, n = 2000;
    volatile int r = 300, s = 400;
    
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Create parent instruction resources */
    int parent_val = ext_func3(m);
    
    /* Conditional computed goto */
    if (cond1 && !cond2) {
        __asm__ volatile ("" : : : "memory");
        goto *labels[1];
    }
    
    /* Intermediate basic block */
    n = ext_func2(n);
    
label_a:
    s = r * 2;  /* Multiplication with safe operands */
    goto end;
    
label_b:
    /* Target label instruction - simple assignment */
    s = r + 100;  /* Different operation than parent */
    goto end;
    
label_c:
    s = r / 2;  /* Division but with constant divisor 2 (safe) */
    
end:
    result += s + parent_val;
}

/* Function with multiple jump-to-label patterns */
__attribute__((optimize("O2")))
void test_function4(void) {
    volatile int var1 = 1, var2 = 2, var3 = 3;
    volatile int var4 = 4, var5 = 5;
    
    /* Pattern 1 */
    int res1 = ext_func1(var1);
    if (cond3) {
        __asm__ volatile ("" : : : "memory");
        goto pattern1_target;
    }
    var2 = ext_func2(var2);
    
pattern1_target:
    var4 = var5 + 7;  /* Simple addition */
    result += var4 + res1;
    
    /* Pattern 2 - different operation */
    int res2 = ext_func3(var3);
    if (cond4) {
        __asm__ volatile ("" : : : "memory");
        goto pattern2_target;
    }
    var1 = ext_func1(var1);
    
pattern2_target:
    var5 = var4 - 3;  /* Simple subtraction */
    result += var5 + res2;
}

/* Function designed for MIPS-like delay slot exploration */
__attribute__((optimize("O2")))
void test_function_mips_like(void) {
    volatile int base = 100;
    volatile int offset = 50;
    volatile int temp = 0;
    
    /* Multiple conditional jumps in sequence */
    for (int j = 0; j < 4; j++) {
        int iteration_res = ext_func2(base + j);
        
        /* First conditional jump */
        if (j % 2 == 0) {
            if (cond1) {
                __asm__ volatile ("" : : : "memory");
                goto mips_target1;
            }
        }
        
        /* Second conditional jump */
        if (j % 3 == 0) {
            if (cond3) {
                __asm__ volatile ("" : : : "memory");
                goto mips_target2;
            }
        }
        
        /* Default path */
        temp = ext_func3(offset);
        continue;
        
    mips_target1:
        /* First target - bitwise operation */
        offset = base & 0xFF;  /* Safe bitwise AND */
        result += offset + iteration_res;
        continue;
        
    mips_target2:
        /* Second target - shift operation */
        offset = base << 2;  /* Safe left shift */
        result += offset + iteration_res;
    }
}

/* External function definitions (simulated) */
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
    printf("Starting reorg test patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Execute test functions with various patterns */
    test_function1();
    printf("After test_function1: result = %d\n", result);
    
    test_function2();
    printf("After test_function2: result = %d\n", result);
    
    test_function3();
    printf("After test_function3: result = %d\n", result);
    
    test_function4();
    printf("After test_function4: result = %d\n", result);
    
    test_function_mips_like();
    printf("After test_function_mips_like: result = %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return 0;
}
