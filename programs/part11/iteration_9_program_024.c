/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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
    volatile int local_a = a;
    volatile int local_b = b;
    volatile int local_res = 0;
    
    /* Call external function to create resource barrier */
    local_res = ext_func1(local_a);
    
    /* Conditional jump to label - essential for jump_to_label_p() */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    local_res += ext_func2(local_b);
    
target_label1:
    /* Non-jump, non-sequence instruction at target */
    /* Simple arithmetic that doesn't conflict with parent instruction resources */
    volatile int temp = local_a + local_b;
    
    /* Another external call */
    temp = ext_func3(temp);
    
    result += temp;
}

/* More complex control flow */
__attribute__((optimize("O2"), noinline))
void test_function2(int iterations) {
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple conditional jumps in loop */
        if (cond2) {
            goto loop_target;
        }
        
        /* Different computation path */
        x = ext_func1(x);
        
        if (cond3 && i % 2 == 0) {
            goto loop_target;
        }
        
        y = ext_func2(y);
        continue;
        
    loop_target:
        /* Target instruction: simple assignment */
        z = x + y;
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    result += z;
}

/* Switch statement with label jumps */
__attribute__((optimize("O3")))
void test_function3(int mode) {
    volatile int val1 = a;
    volatile int val2 = b;
    volatile int computed = 0;
    
    switch (mode) {
        case 1:
            if (cond1) {
                goto switch_target;
            }
            val1 *= 2;
            break;
            
        case 2:
            computed = ext_func1(val1);
            if (cond4) {
                goto switch_target;
            }
            break;
            
        case 3:
            /* Direct computation */
            computed = val1 + val2;
            break;
            
        default:
            goto switch_target;
    }
    
    /* Some common code */
    computed += 5;
    goto end;
    
switch_target:
    /* Target instruction: safe subtraction */
    computed = val2 - val1;
    
end:
    result += computed;
}

/* Function using computed goto (labels as values) */
__attribute__((optimize("O2"), noinline))
void test_function4(void) {
    volatile int p = a;
    volatile int q = b;
    volatile int r = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&target_label2 };
    
    /* External call for resource separation */
    r = ext_func2(p);
    
    /* Conditional jump */
    if (cond3) {
        goto *labels[3];  /* Jump to target_label2 */
    }
    
    r = ext_func1(q);
    goto end_func4;
    
label1:
    p += 10;
    goto end_func4;
    
label2:
    q -= 5;
    goto end_func4;
    
label3:
    r = p * q;
    goto end_func4;
    
target_label2:
    /* Simple non-trapping instruction */
    r = q + p;
    
end_func4:
    result += r;
}

/* Function with inline assembly to create artificial resource constraints */
__attribute__((optimize("O3")))
void test_function5(void) {
    volatile int m = a;
    volatile int n = b;
    volatile int res = 0;
    
    /* Inline assembly with memory clobber */
    asm volatile(
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (res)
        : "r" (m), "r" (n)
        : "%eax", "memory"
    );
    
    /* Conditional jump after assembly */
    if (cond1 && cond3) {
        goto asm_target;
    }
    
    res = ext_func3(res);
    goto finish;
    
asm_target:
    /* Safe instruction at target */
    volatile int temp = m - n;
    
    /* Another assembly barrier */
    asm volatile("" : : : "memory");
    
    res += temp;
    
finish:
    result += res;
}

/* Nested control flow with multiple labels */
__attribute__((optimize("O2")))
void test_function6(int depth) {
    volatile int counter = 0;
    volatile int accum = 0;
    
    while (counter < depth) {
        /* Multiple conditional jumps at different nesting levels */
        if (cond2) {
            goto inner_target;
        }
        
        for (int j = 0; j < 3; j++) {
            if (cond1 && j == 1) {
                goto middle_target;
            }
            
            accum += ext_func1(counter + j);
            
            if (cond4) {
                goto inner_target;
            }
        }
        
        counter++;
        continue;
        
    middle_target:
        /* Intermediate computation */
        accum += 5;
        continue;
        
    inner_target:
        /* Target instruction: simple increment */
        counter++;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    result += accum;
}

/* External function definitions (simulated) */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 3;
}

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    a = 5; b = 10; c = 15; d = 20;
    result = 0;
    
    /* Run test functions with different patterns */
    test_function1();
    printf("After test1: result = %d\n", result);
    
    test_function2(3);
    printf("After test2: result = %d\n", result);
    
    test_function3(2);
    printf("After test3: result = %d\n", result);
    
    test_function4();
    printf("After test4: result = %d\n", result);
    
    test_function5();
    printf("After test5: result = %d\n", result);
    
    test_function6(2);
    printf("After test6: result = %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return 0;
}
