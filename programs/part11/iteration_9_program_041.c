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

/* Function with O2 optimization and jump-to-label pattern */
__attribute__((optimize("O2")))
void test_function1(void) {
    volatile int x = a, y = b;
    volatile int local_cond = cond1;
    
    /* Call external function to create resource barrier */
    int tmp = ext_func1(x);
    
    /* Parent instruction for delay slot */
    int parent_result = x * y + tmp;
    
    if (local_cond) {
        /* Jump to label - this is the 'trial' jump */
        goto target_label1;
    }
    
    /* Some intermediate code */
    x = x + 1;
    y = y - 1;
    
target_label1:
    /* This is 'next_trial' - simple non-jump, non-sequence instruction */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    volatile int z = c + d;
    result += z;
    
    /* More code to prevent tail merging */
    x = x * 2;
    result += x;
}

/* Function with switch statement and nested jumps */
__attribute__((optimize("O2")))
void test_function2(int mode) {
    volatile int x = a, y = b;
    volatile int local_cond = cond2;
    
    switch (mode) {
        case 1:
            ext_func2(x);
            if (local_cond) {
                goto target_label2;
            }
            x = x * 3;
            break;
            
        case 2:
            ext_func1(y);
            if (!local_cond) {
                goto target_label2;
            }
            y = y / 2;
            break;
            
        default:
            break;
    }
    
    /* Some computation */
    int compute = x + y;
    
target_label2:
    /* Candidate for delay slot filling */
    /* Simple assignment with no resource conflict */
    volatile int w = d - c;
    result += w + compute;
    
    /* Loop to add complexity */
    for (int i = 0; i < 3; i++) {
        result += i;
    }
}

/* Function with computed goto (&& labels) */
__attribute__((optimize("O3")))
void test_function3(void) {
    volatile int x = b, y = c;
    static void *labels[] = { &&label1, &&label2, &&label3 };
    
    /* Resource-intensive parent instruction */
    int parent_val = (x << 2) | (y & 0xF);
    ext_func3(parent_val);
    
    /* Conditional computed goto */
    volatile int selector = cond3 ? 0 : 1;
    
    goto *labels[selector];
    
label1:
    x = x + 10;
    goto end;
    
label2:
    /* Target label with simple instruction */
    volatile int simple_op = a * 2;
    result += simple_op;
    goto end;
    
label3:
    y = y - 5;
    goto end;
    
end:
    /* Final computation */
    result += x + y;
}

/* Function with inline assembly to force specific RTL patterns */
__attribute__((optimize("O2"), noinline))
void test_function4(void) {
    volatile int x = a, y = b;
    volatile int local_cond = cond4;
    
    /* Inline assembly with memory clobber to create resource set */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        : 
        : "r" (x), "r" (y)
        : "%eax", "memory"
    );
    
    /* Complex parent instruction */
    int parent_result;
    asm volatile (
        "imull %1, %0\n\t"
        : "=r" (parent_result)
        : "r" (x), "0" (y)
        : "cc"
    );
    
    /* Conditional jump to label */
    if (local_cond != 0) {
        goto target_label4;
    }
    
    /* Intermediate basic block */
    ext_func2(x);
    
target_label4:
    /* Simple non-trapping instruction */
    /* Uses completely different volatile variables */
    volatile int m = 100, n = 50;
    volatile int diff = m - n;
    result += diff;
    
    /* Prevent optimization */
    asm volatile ("" : : : "memory");
}

/* Function with loop and multiple jump targets */
__attribute__((optimize("O2")))
void test_function5(int iterations) {
    volatile int x = c, y = d;
    
    for (int i = 0; i < iterations; i++) {
        volatile int loop_cond = (i % 2) == 0;
        
        /* Parent instruction in loop */
        int loop_parent = x * i + y;
        ext_func1(loop_parent);
        
        if (loop_cond) {
            goto loop_target;
        }
        
        /* Alternate path */
        x = x + i;
        continue;
        
    loop_target:
        /* Simple instruction at jump target */
        volatile int increment = i * 2;
        result += increment;
        
        y = y - i;
    }
}

/* Function specifically designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), target("arch=mips32")))
void test_function_mips(void) {
    volatile int x = a, y = b;
    volatile int mips_cond = cond1;
    
    /* Parent instruction using different registers/resources */
    int mips_parent = (x << 3) | (y & 0xFF);
    
    /* External call as barrier */
    ext_func3(mips_parent);
    
    /* Conditional jump */
    if (mips_cond) {
        goto mips_target;
    }
    
    /* Fall-through path */
    x = x ^ y;
    
mips_target:
    /* Very simple instruction - good delay slot candidate */
    volatile int simple = 42;
    result += simple;
    
    /* Ensure the instruction isn't optimized away */
    asm volatile ("" : "+r" (result));
}

/* External function implementations (simulated) */
int ext_func1(int val) {
    return val + 1;
}

int ext_func2(int val) {
    return val * 2;
}

int ext_func3(int val) {
    return val - 3;
}

int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile control variables */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    a = 5; b = 10; c = 15; d = 20;
    result = 0;
    
    /* Execute test functions with different patterns */
    test_function1();
    printf("After test_function1: result = %d\n", result);
    
    test_function2(1);
    printf("After test_function2: result = %d\n", result);
    
    test_function3();
    printf("After test_function3: result = %d\n", result);
    
    test_function4();
    printf("After test_function4: result = %d\n", result);
    
    test_function5(3);
    printf("After test_function5: result = %d\n", result);
    
    /* Only compile this for MIPS targets */
#ifdef __mips__
    test_function_mips();
    printf("After test_function_mips: result = %d\n", result);
#endif
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
