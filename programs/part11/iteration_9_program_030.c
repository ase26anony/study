/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern int ext_func4(int);

/* Volatile control variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int global_result = 0;

/* Test function 1: Basic jump-to-label pattern */
__attribute__((optimize("O2")))
void test_basic_jump_pattern(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int local_cond = cond1;
    
    /* Create resource usage before jump */
    int res1 = ext_func1(a);
    
    /* Conditional jump to label */
    if (local_cond) {
        goto target_label_1;
    }
    
    /* Some intermediate code */
    b = ext_func2(b);
    
target_label_1:
    /* Simple non-jump, non-trapping instruction at target */
    /* This is the candidate for delay slot filling */
    c = a + b;  /* Simple arithmetic, no resource conflict with jump */
    
    /* Use result to prevent elimination */
    global_result += c + res1;
}

/* Test function 2: Nested control flow with switch */
__attribute__((optimize("O3")))
void test_nested_switch_pattern(void) {
    volatile int x = 5, y = 15, z = 0;
    volatile int switch_val = cond2 ? 1 : 2;
    
    for (int i = 0; i < 3; i++) {
        switch (switch_val) {
            case 1:
                /* Call external function to create resource separation */
                ext_func3(x);
                
                /* Conditional jump */
                if (cond3) {
                    goto target_label_2;
                }
                break;
                
            case 2:
                y = ext_func4(y);
                break;
        }
        
        /* Some computation */
        x = x * 2;
    }
    
    /* This should be unreachable from the goto above */
    z = x - y;
    return;
    
target_label_2:
    /* Candidate instruction for delay slot */
    /* Uses different variables than those in the jump's basic block */
    volatile int temp1 = 100, temp2 = 200;
    volatile int sum = temp1 + temp2;  /* Safe, non-trapping operation */
    
    global_result += sum;
}

/* Test function 3: Multiple labels with different operations */
__attribute__((optimize("O2"), __noinline__))
void test_multiple_labels(void) {
    volatile int p = 30, q = 40, r = 0;
    volatile int local_flag = cond4;
    
    /* Inline assembly to create artificial resource constraints */
    asm volatile ("" : : : "memory");
    
    if (local_flag) {
        goto label_a;
    }
    
    p = ext_func1(p);
    
    if (!local_flag) {
        goto label_b;
    }
    
    /* Intermediate block */
    q = q * 2;
    goto end;
    
label_a:
    /* First candidate: simple assignment */
    r = p;  /* Just assignment, no complex operation */
    goto end;
    
label_b:
    /* Second candidate: subtraction */
    r = q - p;  /* Simple subtraction */
    
end:
    global_result += r;
    
    /* More inline assembly as barrier */
    asm volatile ("" : : : "memory");
}

/* Test function 4: Computed goto pattern */
__attribute__((optimize("O2")))
void test_computed_goto(void) {
    static void* labels[] = { &&label_x, &&label_y, &&label_z };
    volatile int idx = cond1 ? 0 : 1;
    volatile int m = 50, n = 60, o = 0;
    
    /* Resource usage before goto */
    ext_func2(m);
    
    /* Computed goto */
    goto *labels[idx];
    
    /* Unreachable code */
    m = m + n;
    return;
    
label_x:
    /* Candidate instruction: multiplication */
    o = m * 2;  /* Simple multiplication */
    goto finish;
    
label_y:
    /* Candidate instruction: bitwise operation */
    o = m & 0xFF;  /* Bitwise AND, non-trapping */
    goto finish;
    
label_z:
    /* Candidate instruction: shift */
    o = m << 2;  /* Shift operation */
    
finish:
    global_result += o;
    ext_func3(o);
}

/* Test function 5: Loop with conditional exit to label */
__attribute__((optimize("O3"), __noinline__))
void test_loop_exit_pattern(void) {
    volatile int counter = 0;
    volatile int accum = 0;
    volatile int data[4] = {1, 2, 3, 4};
    
    while (counter < 10) {
        /* External call creates resource separation */
        int val = ext_func4(counter);
        
        /* Conditional jump out of loop */
        if (val > 5 && cond3) {
            goto exit_label;
        }
        
        /* Normal loop processing */
        accum += data[counter % 4];
        counter++;
    }
    
    /* Normal loop exit */
    global_result += accum;
    return;
    
exit_label:
    /* Candidate at loop exit point */
    volatile int final = accum * 10;  /* Simple multiplication */
    global_result += final;
}

/* Test function 6: Try to avoid trapping instructions */
__attribute__((optimize("O2")))
void test_safe_operations(void) {
    volatile int u = 100, v = 200;
    volatile int w = 0;
    
    /* Use safe divisor to avoid trapping */
    volatile int safe_divisor = 2;  /* Not zero, so division is safe */
    
    /* External call for resource separation */
    ext_func1(u);
    
    /* Multiple conditional jumps */
    if (cond1) {
        if (cond2) {
            goto safe_label_1;
        } else {
            goto safe_label_2;
        }
    }
    
    v = ext_func2(v);
    goto end;
    
safe_label_1:
    /* Safe operations only */
    w = u + v;  /* Addition - safe */
    goto end;
    
safe_label_2:
    /* More safe operations */
    w = v - u;  /* Subtraction - safe */
    /* w = u / safe_divisor; */  /* Even division is safe with non-zero divisor */
    
end:
    global_result += w;
}

/* Test function 7: Complex pattern with inline assembly barriers */
__attribute__((optimize("O3"), __noinline__))
void test_with_asm_barriers(void) {
    volatile int arr[8] = {0};
    volatile int idx = 0;
    volatile int total = 0;
    
    /* Start with assembly barrier */
    asm volatile ("" : : : "memory");
    
    for (idx = 0; idx < 8; idx++) {
        arr[idx] = idx * 10;
        
        /* Conditional jump to label inside loop */
        if (cond1 && (idx == 4)) {
            goto process_midpoint;
        }
    }
    
    /* Post-loop processing */
    total = ext_func3(arr[7]);
    goto finalize;
    
process_midpoint:
    /* Candidate instruction in middle of loop */
    /* Simple array access and assignment */
    volatile int mid_val = arr[idx] + 5;  /* Safe operation */
    
    /* Continue loop */
    for (; idx < 8; idx++) {
        arr[idx] = ext_func4(arr[idx]);
    }
    
finalize:
    /* Calculate final total */
    for (int i = 0; i < 8; i++) {
        total += arr[i];
    }
    
    global_result += total;
    
    /* Final memory barrier */
    asm volatile ("" : : : "memory");
}

/* Dummy external function implementations */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
int ext_func4(int x) { return x / 2; }

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;  /* True */
    cond2 = 0;  /* False */
    cond3 = 1;  /* True */
    cond4 = 0;  /* False */
    
    /* Run all test patterns */
    test_basic_jump_pattern();
    test_nested_switch_pattern();
    test_multiple_labels();
    test_computed_goto();
    test_loop_exit_pattern();
    test_safe_operations();
    test_with_asm_barriers();
    
    printf("All tests completed. Global result: %d\n", global_result);
    printf("(Non-zero result confirms execution through all code paths)\n");
    
    return 0;
}
