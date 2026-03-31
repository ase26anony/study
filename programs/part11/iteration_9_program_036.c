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

/* Global volatile variables for resource separation */
volatile int gvar1 = 10;
volatile int gvar2 = 20;
volatile int gvar3 = 30;
volatile int gvar4 = 40;
volatile int gvar5 = 50;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int local1 = 100;
    volatile int local2 = 200;
    volatile int local3 = 300;
    
    /* Create artificial resource constraints with inline assembly */
    asm volatile ("" : : : "memory");
    
    /* Pattern 1: Conditional jump to label with simple arithmetic at target */
    if (cond1) {
        /* Parent instruction for delay slot */
        int temp = ext_func1(local1) + gvar1;
        result += temp;
        
        /* Conditional jump to label */
        if (cond2) {
            goto target_label1;
        }
        
        /* Some intermediate code */
        local2 = ext_func2(local2);
        asm volatile ("" : : : "memory");
        
    target_label1:
        /* Simple non-jump, non-trapping instruction at target */
        /* This is the candidate for delay slot filling (next_trial) */
        local3 = gvar2 + gvar3;  /* Simple addition, no traps */
        result += local3;
    }
    
    /* More code to prevent tail merging */
    asm volatile ("" : : : "memory");
    ext_func3(result);
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int counter = 0;
    
    /* Loop with nested conditional jumps */
    for (int i = 0; i < 10; i++) {
        asm volatile ("" : : : "memory");
        
        /* Switch statement to create complex control flow */
        switch (i % 3) {
            case 0:
                if (cond3) {
                    /* Parent instruction using different resources */
                    int temp = ext_func1(a) * b;
                    result ^= temp;
                    
                    /* Jump to label */
                    if (cond4) {
                        goto case0_target;
                    }
                    
                    /* Intermediate computation */
                    c = ext_func2(c);
                    
                case0_target:
                    /* Simple assignment at target */
                    d = gvar4 - gvar5;  /* Subtraction, no traps */
                    result += d;
                }
                break;
                
            case 1:
                /* Another pattern */
                {
                    volatile int x = 5, y = 6;
                    asm volatile ("" : : : "memory");
                    
                    if (cond1) {
                        int temp = ext_func3(x) | y;
                        result |= temp;
                        
                        if (cond2) {
                            goto case1_target;
                        }
                        
                        ext_func1(y);
                        
                    case1_target:
                        /* Simple bitwise operation */
                        x = gvar1 & gvar2;  /* No traps */
                        result ^= x;
                    }
                }
                break;
        }
        
        counter++;
        asm volatile ("" : : : "memory");
    }
}

/* Function using computed goto for variety */
__attribute__((optimize("O2"), __noinline__))
void test_function3(void) {
    static void *labels[] = { &&label1, &&label2, &&label3 };
    volatile int selector = cond1 ? 0 : 1;
    
    /* Parent instruction with resource usage */
    volatile int res1 = ext_func1(gvar1);
    volatile int res2 = ext_func2(gvar2);
    int parent_result = res1 + res2;
    result += parent_result;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Computed goto */
    goto *labels[selector];
    
label1:
    /* Simple arithmetic at target */
    {
        volatile int t1 = gvar3;
        volatile int t2 = gvar4;
        int simple_op = t1 * t2;  /* Multiplication, but with safe operands */
        result += simple_op;
    }
    goto end;
    
label2:
    /* Another simple operation */
    {
        volatile int t = gvar5;
        int shift_op = t << 2;  /* Shift operation, no traps */
        result |= shift_op;
    }
    goto end;
    
label3:
    /* Assignment only */
    {
        volatile int assign = 999;
        result = assign;
    }
    
end:
    asm volatile ("" : : : "memory");
}

/* Function with multiple jump-to-label patterns in sequence */
__attribute__((optimize("O2"), __noinline__))
void test_function4(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    
    /* Pattern A */
    {
        ext_func1(v1);
        if (cond1) {
            /* Parent uses v1, v2 */
            int parent_op = (v1 + v2) * 3;
            result += parent_op;
            
            if (cond2) {
                goto target_a;
            }
            
            ext_func2(v3);
            
        target_a:
            /* Target uses v4 only - different resources */
            v4 = v4 + 5;  /* Simple increment */
            result += v4;
        }
    }
    
    asm volatile ("" : : : "memory");
    
    /* Pattern B with different variable sets */
    {
        volatile int w1 = 10, w2 = 20;
        ext_func3(w1);
        
        if (cond3) {
            /* Parent uses w1 */
            int parent_op = w1 << 1;
            result ^= parent_op;
            
            if (cond4) {
                goto target_b;
            }
            
            ext_func1(w2);
            
        target_b:
            /* Target uses only constants - no resource conflict */
            int safe_op = 100 + 200;  /* Constant expression */
            result += safe_op;
        }
    }
}

/* Function that avoids potentially trapping operations at target */
__attribute__((optimize("O2")))
void test_function5(void) {
    volatile int safe_divisor = 1;  /* Non-zero to avoid trap */
    volatile int array[4] = {1, 2, 3, 4};
    
    /* Multiple basic blocks */
    for (int i = 0; i < 4; i++) {
        asm volatile ("" : : : "memory");
        
        if (i % 2 == 0) {
            /* Parent instruction */
            int idx = ext_func1(i) % 4;
            int parent_val = array[idx];
            result += parent_val;
            
            /* Conditional jump */
            if (cond1 && (i < 3)) {
                goto safe_target;
            }
            
            ext_func2(i);
            continue;
            
        safe_target:
            /* Safe operation at target - no division by zero, no memory access with variable index */
            int safe_result = gvar1 + i;  /* Simple addition with loop counter */
            result ^= safe_result;
        } else {
            /* Different path */
            result -= i;
        }
    }
}

/* Main function that calls all test patterns */
int main(void) {
    printf("Starting reorg test patterns...\n");
    
    /* Initialize volatile conditions with non-deterministic values */
    cond1 = (rand() % 2);
    cond2 = (rand() % 2);
    cond3 = (rand() % 2);
    cond4 = (rand() % 2);
    
    /* Execute all test functions */
    test_function1();
    test_function2();
    test_function3();
    test_function4();
    test_function5();
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Conditions: %d %d %d %d\n", cond1, cond2, cond3, cond4);
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x ^ 0x55; }
