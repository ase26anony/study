/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern void ext_side_effect(void);

/* Volatile control variables */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int result = 0;

/* Test function 1: Basic jump-to-label pattern */
__attribute__((optimize("O2")))
void test_basic_jump(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int jump_cond = cond1;
    
    /* Resource used by parent instruction (simulating delay slot) */
    volatile int parent_res = 100;
    
    /* Call external function to create resource separation */
    ext_func1(parent_res);
    
    if (jump_cond) {
        /* This goto creates a simple jump to label */
        goto target_label_1;
    }
    
    /* Some intermediate code to avoid fall-through optimization */
    a = ext_func2(a);
    
target_label_1:
    /* This is the candidate for delay slot filling (next_trial) */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    c = b + 30;  /* Simple arithmetic, non-trapping */
    
    /* Another external call to prevent merging with following code */
    ext_side_effect();
    
    result += c;
}

/* Test function 2: Nested control flow with switch */
__attribute__((optimize("O3")))
void test_nested_switch(void) {
    volatile int x = 5, y = 15, z = 0;
    volatile int switch_val = cond2 ? 1 : 2;
    
    for (int i = 0; i < 3; i++) {
        switch (switch_val) {
            case 1:
                if (cond3) {
                    /* Jump to label after switch */
                    goto target_label_2;
                }
                x = ext_func1(x);
                break;
            case 2:
                y = ext_func2(y);
                break;
            default:
                break;
        }
        
        /* Some computation that uses different resources */
        volatile int temp = x * y;
        
        if (temp > 50) {
            /* Another potential jump point */
            goto target_label_2;
        }
    }
    
    /* Default path */
    z = x + y;
    goto end_func_2;
    
target_label_2:
    /* Delay slot candidate - simple assignment with no resource conflict */
    z = 42;  /* Magic number assignment */
    
end_func_2:
    result += z;
}

/* Test function 3: Computed goto pattern */
__attribute__((optimize("O2"), __noinline__))
void test_computed_goto(void) {
    volatile int p = 100, q = 200, r = 0;
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Create artificial resource usage */
    volatile int resource_a = p * 2;
    ext_func3(resource_a);
    
    /* Use inline assembly to create control flow barrier */
    __asm__ volatile ("" : : : "memory");
    
    /* Conditional computed goto */
    int idx = cond4 ? 0 : 1;
    goto *labels[idx];
    
label_a:
    /* Intermediate computation */
    p = ext_func1(p);
    goto label_target;
    
label_b:
    q = ext_func2(q);
    goto label_target;
    
label_c:
    /* Fall through to target */
    
label_target:
    /* This is our delay slot candidate */
    /* Simple operation with no trapping */
    r = q - p;  /* Subtraction is safe */
    
    /* Prevent tail merging */
    __asm__ volatile ("" : : : "memory");
    
    result += r;
}

/* Test function 4: Loop with multiple exit points */
__attribute__((optimize("O2"), __noinline__))
void test_loop_exits(void) {
    volatile int counter = 0;
    volatile int accum = 0;
    volatile int exit_flag = 0;
    
    while (counter < 10) {
        /* Vary the exit condition */
        exit_flag = ext_func1(counter) % 3;
        
        if (exit_flag == 1 && cond1) {
            /* Jump out of loop to target label */
            goto loop_exit_label;
        }
        
        if (exit_flag == 2 && cond3) {
            /* Alternative exit path */
            accum += counter * 2;
            counter++;
            continue;
        }
        
        /* Normal loop body */
        accum += counter;
        counter++;
        
        /* Insert memory barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Loop completed normally */
    accum += 1000;
    goto function_end;
    
loop_exit_label:
    /* Delay slot candidate - safe assignment */
    accum = 999;  /* Distinct value to mark early exit */
    
function_end:
    result += accum;
}

/* Test function 5: Multiple labels in sequence */
__attribute__((optimize("O3"), __noinline__))
void test_label_sequence(void) {
    volatile int val1 = 1, val2 = 2, val3 = 3;
    volatile int output = 0;
    
    /* First computation block */
    val1 = ext_func1(val1);
    
    if (cond2) {
        goto second_label;
    }
    
first_label:
    /* Intermediate computation */
    val2 = val1 * 10;
    goto third_label;
    
second_label:
    /* Another intermediate step */
    val3 = ext_func2(val2);
    
third_label:
    /* This is our target - simple non-trapping operation */
    output = val1 + val2 + val3;
    
    /* Final external call */
    ext_func3(output);
    
    result += output;
}

/* Test function 6: Minimal pattern with inline assembly constraints */
__attribute__((optimize("O2"), __noinline__))
void test_minimal_pattern(void) {
    volatile int x = 10, y = 20;
    
    /* Force jump instruction generation */
    if (cond1) {
        __asm__ volatile ("# Jump barrier" : : : "memory");
        goto minimal_target;
    }
    
    x = ext_func1(x);
    
minimal_target:
    /* Extremely simple delay slot candidate */
    y = x + 5;  /* No function calls, no complex operations */
    
    result += y;
}

/* Dummy external function implementations */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x / 2; }
void ext_side_effect(void) { result += 1; }

/* Main function to execute all tests */
int main(void) {
    printf("Starting reorg delay slot test...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Execute all test patterns */
    test_basic_jump();
    printf("After test_basic_jump: result = %d\n", result);
    
    test_nested_switch();
    printf("After test_nested_switch: result = %d\n", result);
    
    test_computed_goto();
    printf("After test_computed_goto: result = %d\n", result);
    
    test_loop_exits();
    printf("After test_loop_exits: result = %d\n", result);
    
    test_label_sequence();
    printf("After test_label_sequence: result = %d\n", result);
    
    test_minimal_pattern();
    printf("After test_minimal_pattern: result = %d\n", result);
    
    printf("Final result: %d\n", result);
    
    /* Verify execution */
    if (result != 0) {
        printf("Test completed successfully (non-zero result).\n");
        return 0;
    } else {
        printf("Warning: Result is zero - patterns may have been optimized away.\n");
        return 1;
    }
}
