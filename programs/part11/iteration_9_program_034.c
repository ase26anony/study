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
volatile int result = 0;

/* Test function 1: Basic jump-to-label pattern */
__attribute__((optimize("O2")))
void test_basic_jump(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int local_cond = cond1;
    
    /* Call external function before jump */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (local_cond) {
        /* Inline assembly to create resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    c = ext_func2(b);
    
target_label1:
    /* Simple non-jump, non-trapping instruction */
    /* This is the candidate for delay slot filling */
    a = b + 5;  /* Simple arithmetic, no resource conflict */
    
    /* Call external function after label */
    ext_func3(a);
    result += a;
}

/* Test function 2: Jump inside loop */
__attribute__((optimize("O3")))
void test_loop_jump(void) {
    volatile int x = 0, y = 100, z = 0;
    volatile int loop_cond = cond2;
    
    for (int i = 0; i < 10; i++) {
        ext_func1(i);
        
        if (loop_cond && (i % 2 == 0)) {
            /* Use different volatile for jump condition */
            volatile int jump_flag = cond3;
            if (jump_flag) {
                __asm__ volatile ("" : : : "memory");
                goto target_label2;
            }
        }
        
        x = ext_func2(y);
        continue;
        
    target_label2:
        /* Safe assignment without division */
        z = x * 2;  /* Multiplication is safe with integers */
        ext_func3(z);
        result += z;
        
        /* Break to avoid infinite loop in label */
        break;
    }
}

/* Test function 3: Switch statement with jumps */
__attribute__((optimize("O2")))
void test_switch_jump(void) {
    volatile int val = 3;
    volatile int switch_cond = cond1;
    
    ext_func1(val);
    
    switch (val) {
        case 1:
            if (switch_cond) {
                goto target_label3;
            }
            break;
        case 2:
            val = ext_func2(val);
            break;
        case 3:
            /* This case will execute */
            if (switch_cond) {
                __asm__ volatile ("" : : : "memory");
                goto target_label3;
            }
            break;
        default:
            val = 0;
    }
    
    /* Some code that won't be reached if jump is taken */
    val = ext_func3(val);
    return;
    
target_label3:
    /* Simple memory operation */
    volatile int temp = 42;
    result = temp - 10;  /* Simple subtraction */
    ext_func4(result);
}

/* Test function 4: Computed goto with address-of-label */
__attribute__((optimize("O2")))
void test_computed_goto(void) {
    volatile int mode = cond4 ? 1 : 2;
    void* label_ptr = NULL;
    
    static void* labels[] = { &&normal, &&target_label4 };
    
    ext_func1(mode);
    
    if (mode == 1) {
        label_ptr = labels[1];
    } else {
        label_ptr = labels[0];
    }
    
    /* Use inline assembly to prevent optimization of computed goto */
    __asm__ volatile ("" : : "r"(label_ptr) : "memory");
    
    goto *label_ptr;
    
normal:
    result += 100;
    return;
    
target_label4:
    /* Candidate instruction for delay slot */
    volatile int p = 50, q = 25;
    int r = p - q;  /* Simple subtraction, no traps */
    result += r;
    ext_func2(result);
}

/* Test function 5: Nested jumps with different operations */
__attribute__((optimize("O3")))
void test_nested_jumps(void) {
    volatile int outer_cond = cond1;
    volatile int inner_cond = cond3;
    volatile int a = 100, b = 200;
    
    ext_func1(a);
    
    if (outer_cond) {
        ext_func2(b);
        
        for (int i = 0; i < 5; i++) {
            if (inner_cond && (i > 2)) {
                /* Multiple conditions to create complex CFG */
                volatile int final_cond = cond4;
                if (final_cond) {
                    goto target_label5;
                }
            }
            a = ext_func3(a + i);
        }
        
        /* Another possible jump point */
        if (outer_cond) {
            __asm__ volatile ("" : : : "memory");
            goto alternate_label;
        }
    }
    
    result += a;
    return;
    
alternate_label:
    b = ext_func4(b);
    /* Fall through to target label */
    
target_label5:
    /* Very simple assignment - good delay slot candidate */
    volatile int simple = 999;
    result = simple;
}

/* Test function 6: Multiple labels in sequence */
__attribute__((optimize("O2")))
void test_multiple_labels(void) {
    volatile int step = 0;
    volatile int control = cond1;
    
    ext_func1(step);
    
    if (control) {
        step = 1;
        goto stage1;
    }
    
    step = ext_func2(step);
    return;
    
stage1:
    /* Intermediate label */
    ext_func3(step);
    if (step < 5) {
        step++;
        goto stage2;
    }
    
stage2:
    /* Final target label with simple operation */
    volatile int final_val = 1234;
    result = final_val >> 2;  /* Bit shift - safe operation */
}

/* Test function 7: Minimal pattern focusing on resource separation */
__attribute__((optimize("O2")))
void test_minimal_pattern(void) {
    /* Use completely separate variables for jump and target */
    volatile int jump_var = cond1;      /* Only for jump decision */
    volatile int compute_var = 100;     /* Only for parent instruction */
    volatile int target_var = 200;      /* Only for target instruction */
    
    /* Parent instruction computation */
    compute_var = ext_func1(compute_var);
    
    /* Conditional jump */
    if (jump_var) {
        __asm__ volatile ("" : : : "memory");
        goto minimal_target;
    }
    
    compute_var = ext_func2(compute_var);
    return;
    
minimal_target:
    /* Target instruction with no resource overlap */
    target_var = target_var + 1;  /* Uses different variable */
    result = target_var;
}

/* Main function to execute all tests */
int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;  /* Will take jumps */
    cond2 = 0;  /* Won't take jumps in loop */
    cond3 = 1;  /* Will take jumps */
    cond4 = 1;  /* For computed goto */
    
    /* Execute test functions */
    test_basic_jump();
    printf("After test_basic_jump: result = %d\n", result);
    
    test_loop_jump();
    printf("After test_loop_jump: result = %d\n", result);
    
    test_switch_jump();
    printf("After test_switch_jump: result = %d\n", result);
    
    test_computed_goto();
    printf("After test_computed_goto: result = %d\n", result);
    
    test_nested_jumps();
    printf("After test_nested_jumps: result = %d\n", result);
    
    test_multiple_labels();
    printf("After test_multiple_labels: result = %d\n", result);
    
    test_minimal_pattern();
    printf("After test_minimal_pattern: result = %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 3; }
int ext_func4(int x) { return x / 4; }
