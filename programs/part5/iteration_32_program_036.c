/* ifcvt_test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 5;
static volatile int* global_ptr = NULL;
static volatile int global_array[10] = {0};

/* Function to prevent optimizations */
int __attribute__((noinline, noclone)) 
external_func(int x) {
    return x * 2;
}

/* Test function with the targeted if-then-else structure */
int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int input) {
    volatile int local_var = input;
    volatile int result = 0;
    volatile int* ptr = &global_array[0];
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial test_expr */
    if (global_cond < global_threshold && *ptr == 0) {
        /* 
         * CRITICAL: Modify the condition variable BEFORE any real instruction
         * The block starts with a label, then we need to modify test_expr
         * in the header portion before then_last_head
         */
        
        /* First, generate some NOTE/DEBUG_INSN instructions */
        /* These won't count as "real" instructions for the header check */
        asm volatile("# DEBUG/NOTE: Entering then block" : : : "memory");
        asm volatile("# Another note" : : : "memory");
        
        /* 
         * MODIFICATION IN HEADER: 
         * This modifies global_cond which is part of the condition expression
         * This happens before any non-note, non-debug, non-label instruction
         */
        global_cond = 10;  /* This modifies test_expr! */
        
        /* Now add some real instructions after the modification */
        /* These will establish then_last_head */
        result = external_func(local_var);
        *ptr = result;
        global_array[1] = global_cond + 1;
        
        /* More instructions to ensure we have a proper block */
        for (int i = 0; i < 3; i++) {
            global_array[i] += i;
        }
    } else {
        /* Else block with different behavior */
        result = local_var / 2;
        global_cond = -1;
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (result) : : "memory");
    return result;
}

/* Another test with different condition structure */
int __attribute__((noinline, noclone, optimize("O2")))
test_pointer_modification(int input) {
    static volatile int static_var = 0;
    volatile int* volatile ptr = &static_var;
    volatile int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr < 100 && global_threshold > 0) {
        /* Notes/debug insns first */
        asm volatile("# Pointer test note 1" : : : "memory");
        asm volatile("# Pointer test note 2" : : : "memory");
        
        /* Modify through pointer - affects the condition expression */
        *ptr = 200;  /* Modifies test_expr! */
        
        /* Real instructions after modification */
        result = input * 3;
        global_threshold = result;
        
        /* More operations */
        for (int i = 0; i < 2; i++) {
            result += external_func(i);
        }
    } else {
        result = input - 5;
        *ptr = 50;
    }
    
    return result;
}

/* Test with function call in condition */
int __attribute__((noinline, noclone, optimize("O2")))
test_function_call_condition(int input) {
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Self-contained function for condition */
    int __attribute__((noinline)) get_value(void) {
        return counter;
    }
    
    /* Condition with function call */
    if (get_value() < 10 && global_cond > -5) {
        /* Notes first */
        asm volatile("# Function call test note" : : : "memory");
        
        /* Modify variable used in function call */
        counter = 20;  /* Affects test_expr if it's tracked */
        
        /* Real instructions */
        result = input + 100;
        global_cond = result % 7;
        
        /* Additional operations */
        result += external_func(input);
    } else {
        result = input * 2;
        counter = 5;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 0;
    global_threshold = 5;
    global_array[0] = 0;
    global_ptr = &global_array[0];
    
    printf("Testing if-conversion with header modification...\n");
    
    /* Call test functions multiple times to exercise different paths */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to take different branches */
        int input = i % 3;
        
        /* First test - should trigger the uncovered code when global_cond < 5 */
        results[0] += test_if_conversion(input);
        
        /* Reset condition for next test */
        if (i % 2 == 0) {
            global_cond = 0;  /* Will take then-block */
        } else {
            global_cond = 10; /* Will take else-block */
        }
        
        /* Second test */
        results[1] += test_pointer_modification(input);
        
        /* Third test */
        results[2] += test_function_call_condition(input);
    }
    
    printf("Results: %d, %d, %d\n", results[0], results[1], results[2]);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = results[0] + results[1] + results[2];
    asm volatile("" : "+r" (final_result) : : "memory");
    
    return final_result != 0 ? 0 : 1;
}
