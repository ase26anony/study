/* ifcvt_test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 5;
static volatile int* global_ptr = NULL;
static int global_array[10] = {0};

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_value(int x) {
    volatile int sink = x;
    return sink;
}

/* Function with complex condition that will be modified in the then block header */
static int __attribute__((noinline, noclone))
test_if_conversion(int a, int b) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This creates a non-trivial test_expr for modified_in_p to track */
    if (global_cond < global_threshold && global_ptr != NULL && *global_ptr > 0) {
        /* 
         * This assignment modifies global_cond which is part of the condition.
         * We need this to happen in the block header, before any "real" instruction.
         * We'll use inline asm to generate NOTE/DEBUG_INSN instructions first.
         */
        
        /* Generate NOTE instructions in the header */
        asm volatile("# HEADER NOTE 1" : : : "memory");
        asm volatile("# HEADER NOTE 2" : : : "memory");
        asm volatile("# HEADER NOTE 3" : : : "memory");
        
        /* Generate DEBUG_INSN-like instructions */
        /* Use __attribute__((used)) to ensure they're not optimized away */
        volatile int __attribute__((used)) debug_var = 0;
        (void)debug_var;
        
        /* 
         * CRITICAL: Modify the condition variable BEFORE any other real instruction.
         * This should be in the header portion of the block.
         */
        global_cond = a + b;  /* Modifies variable used in condition */
        
        /* Now add real instructions after the modification */
        result = use_value(a * b);
        
        /* More operations to ensure block has sufficient instructions */
        global_array[0] = result;
        if (global_ptr) {
            *global_ptr = result;
        }
        
        /* Additional operations to prevent block merging */
        for (int i = 0; i < 3; i++) {
            result += i;
        }
    } else {
        /* Else branch with different operations */
        result = use_value(a - b);
        global_cond = b - a;
    }
    
    return result;
}

/* Another test case with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int x) {
    static volatile int local_static = 0;
    volatile int* ptr = &local_static;
    int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr < x && global_cond > 0) {
        /* Header with notes/debug instructions */
        asm volatile("# POINTER TEST HEADER" : : : "memory");
        asm volatile("# ANOTHER NOTE" : : : "memory");
        
        /* Modify through pointer - affects condition */
        *ptr = x * 2;  /* Modifies memory location checked in condition */
        
        /* Real instructions after modification */
        result = use_value(x * 3);
        global_array[1] = result;
        
        /* Complex enough to not be optimized away */
        for (int i = 0; i < 4; i++) {
            result += global_array[i];
        }
    } else {
        result = use_value(x / 2);
        local_static = x;
    }
    
    return result;
}

/* Test with function call in condition */
static int __attribute__((noinline, noclone))
helper_func(void) {
    return global_threshold;
}

static int __attribute__((noinline, noclone))
test_function_call_cond(int val) {
    int result = 0;
    
    /* Condition with function call */
    if (val > helper_func() && global_cond < 10) {
        /* Header section */
        asm volatile("# FUNCTION CALL HEADER" : : : "memory");
        
        /* Modify condition variable */
        global_cond = val;  /* Affects second part of condition */
        
        /* Real work */
        result = use_value(val * val);
        for (int i = 0; i < 5; i++) {
            global_array[i] = result + i;
        }
    } else {
        result = use_value(val + val);
        global_cond = val / 2;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    int test_data = 10;
    
    /* Initialize global pointer */
    static volatile int ptr_target = 20;
    global_ptr = (int*)&ptr_target;
    
    /* First call - should take then branch */
    global_cond = 3;  /* Less than threshold */
    results[0] = test_if_conversion(5, 3);
    
    /* Second call - should take else branch */
    global_cond = 10; /* Greater than threshold */
    results[1] = test_if_conversion(2, 8);
    
    /* Third call - test pointer modification */
    results[2] = test_pointer_modification(test_data);
    
    /* Fourth call - test function call condition */
    int result4 = test_function_call_cond(test_data);
    
    /* Fifth call - vary inputs to exercise different paths */
    for (int i = 0; i < 5; i++) {
        global_cond = i;
        test_if_conversion(i, i * 2);
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 3; i++) {
        final_result += results[i];
    }
    final_result += result4;
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
