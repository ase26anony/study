/* ifcvt_coverage.c
 * Target: Trigger uncovered lines 577-583 in ifcvt.cc
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -S ifcvt_coverage.c
 * For ARM: gcc -O2 -march=armv7-a -mtune=cortex-a9 -fdump-rtl-ifcvt -S ifcvt_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 10;
static volatile int* volatile global_ptr = NULL;
static volatile int global_a = 0, global_b = 0, global_c = 0, global_d = 0;

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
int test_function_ifcvt(int x, int y) {
    volatile int local_modifier = 0;
    int result = 0;
    
    /* Complex condition using global variables - creates non-trivial test_expr */
    /* This condition should generate RTL that modified_in_p can track */
    if ((global_cond < threshold) && 
        (global_ptr != NULL && *global_ptr > 0) &&
        (global_a > global_b || global_c < global_d)) {
        
        /* 
         * HEADER SECTION START (targeting lines 577-583)
         * The block starts with a label (implicit from GCC)
         * We need to modify the condition variable BEFORE the first
         * non-label, non-note, non-debug instruction
         */
        
        /* Generate NOTE/DEBUG_INSN instructions using asm comments */
        asm volatile("# DEBUG/NOTE: Start of then block header");
        asm volatile("# Another note instruction in header");
        
        /* CRITICAL: Modify condition variable in the header section
         * This modification happens before then_last_head
         * global_cond is part of the test_expr
         */
        global_cond = x + y;  /* This modifies test_expr! */
        
        /* More notes to ensure we're still in header */
        asm volatile("# DEBUG/NOTE: After condition modification");
        
        /*
         * HEADER SECTION END
         * The first "real" instruction after the header would be here
         * For if-conversion, then_last_head points to the last insn
         * in the header before the first real instruction
         */
        
        /* Additional statements to create a multi-instruction block */
        result = x * y;
        local_modifier = result + 100;
        
        /* Use the modified global in computation */
        result += global_cond;
        
        /* More operations to ensure block isn't optimized away */
        asm volatile("" : : : "memory");  /* Compiler barrier */
        
        /* Function call to prevent tail merging */
        result += rand() % 10;
        
    } else {
        /* Else branch with different computation */
        result = x - y;
        global_cond = threshold - 1;  /* Different modification */
    }
    
    /* Use result to prevent dead code elimination */
    return result + local_modifier;
}

/* Second test function with pointer-based condition */
__attribute__((noinline, noclone))
int test_function_pointer(int* ptr, int val) {
    int result = 0;
    static volatile int static_counter = 0;
    
    /* Condition with pointer dereference */
    if (ptr != NULL && *ptr > val && global_cond < 100) {
        
        /* Header with notes */
        asm volatile("# Header note 1 for pointer test");
        asm volatile("# Header note 2");
        
        /* Modify the dereferenced pointer value - part of condition! */
        *ptr = val * 2;  /* This modifies test_expr (*ptr > val) */
        
        asm volatile("# After pointer modification");
        
        /* Real work */
        result = *ptr + val;
        static_counter++;
        result += static_counter;
        
        /* Complex enough to not be optimized */
        for (int i = 0; i < 3; i++) {
            result += i;
        }
        
    } else {
        result = val * 3;
        if (ptr) *ptr = val / 2;
    }
    
    return result;
}

/* Third test: Function call in condition */
volatile int func_cond_result = 0;

__attribute__((noinline)) 
int condition_func(void) {
    return global_cond + func_cond_result;
}

__attribute__((noinline, noclone))
int test_function_call(int x) {
    int result = 0;
    
    /* Function call in condition */
    if (condition_func() > x && global_a < global_b) {
        
        /* Header section */
        asm volatile("# Function condition test header");
        
        /* Modify variable used in function call */
        func_cond_result = x * 2;  /* Affects condition_func() return */
        
        asm volatile("# After func condition mod");
        
        /* Block body */
        result = x * x;
        global_a = result;
        result += condition_func();
        
    } else {
        result = x + x;
        func_cond_result = x / 2;
    }
    
    return result;
}

/* Main function to exercise all paths */
int main(void) {
    int total = 0;
    int test_array[5] = {1, 2, 3, 4, 5};
    int* test_ptr = &test_array[2];
    
    /* Initialize globals */
    global_cond = 5;
    threshold = 20;
    global_ptr = &test_array[0];
    *global_ptr = 15;
    global_a = 10;
    global_b = 20;
    global_c = 5;
    global_d = 30;
    
    printf("Starting ifcvt coverage test...\n");
    
    /* Multiple calls to ensure if-conversion pass runs */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to exercise different paths */
        global_cond = i * 3;
        threshold = i * 2 + 5;
        
        /* Call test functions with varying conditions */
        total += test_function_ifcvt(i, i + 1);
        total += test_function_pointer(test_ptr, i);
        total += test_function_call(i);
        
        /* Modify pointer target */
        if (test_ptr) {
            *test_ptr = i * 10;
        }
        
        /* Toggle condition variables */
        global_a = (i % 2 == 0) ? 5 : 25;
        global_b = (i % 3 == 0) ? 30 : 10;
    }
    
    printf("Total result: %d\n", total);
    printf("Final global_cond: %d\n", global_cond);
    
    return total > 0 ? 0 : 1;
}
