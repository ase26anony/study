/* ifcvt_coverage.c - Target specific if-conversion coverage test */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 5;
static volatile int* volatile global_ptr = NULL;
static volatile int global_array[10] = {0};

/* Helper function to prevent optimizations */
static int __attribute__((noinline, noclone)) 
helper_func(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

/* Function with the targeted if-then-else structure */
static int __attribute__((noinline, noclone, optimize("O2")))
target_if_conversion(int input) {
    volatile int local_var = input;
    int result = 0;
    
    /* Complex condition using global variable - creates non-trivial test_expr */
    if (global_cond < global_threshold && 
        (global_ptr != NULL ? *global_ptr : 0) < 10 &&
        local_var > 0) {
        
        /* This is the critical part: modifying the condition variable 
           in the block header before any real instruction */
        
        /* First, generate some NOTE/DEBUG_INSN instructions */
        asm volatile("# DEBUG/NOTE: Entering then block" : : : "memory");
        asm volatile("# Another note instruction" : : : "memory");
        
        /* NOW modify the condition variable - this should be detected 
           by modified_in_p() in the uncovered lines */
        global_cond = 10;  /* Direct modification of condition variable */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional instructions to create a multi-instruction block */
        result = helper_func(local_var);
        global_array[0] = result;
        
        asm volatile("# More code in then block" : : : "memory");
        result += global_threshold;
        
    } else {
        /* Else block with different computation */
        result = helper_func(input * 2);
        global_array[1] = result;
    }
    
    return result;
}

/* Another test case with pointer-based condition */
static int __attribute__((noinline, noclone, optimize("O2")))
test_pointer_condition(void) {
    static volatile int data = 3;
    volatile int* ptr = &data;
    int result = 0;
    
    /* Condition involving pointer dereference */
    if (*ptr > 0 && global_threshold < 20) {
        /* Generate notes/debug insns */
        asm volatile("# Note for pointer test" : : : "memory");
        
        /* Modify through pointer - affects condition expression */
        *ptr = 0;  /* This modifies the dereferenced value in condition */
        
        asm volatile("" : : : "memory");
        
        /* More instructions */
        result = helper_func(data);
        global_array[2] = result;
        
    } else {
        result = helper_func(100);
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
static int __attribute__((noinline, noclone, optimize("O2")))
test_compound_condition(int a, int b) {
    volatile int x = a;
    volatile int y = b;
    int result = 0;
    
    /* Compound condition */
    if (x > y && global_cond < 15) {
        /* Header with notes */
        asm volatile("# Compound condition note 1" : : : "memory");
        asm volatile("# Compound condition note 2" : : : "memory");
        
        /* Modify part of the compound condition */
        x = y - 1;  /* x is used in x > y condition */
        
        asm volatile("" : : : "memory");
        
        /* Block body */
        result = x * y + global_cond;
        global_array[3] = result;
        
    } else {
        result = y - x;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Initialize globals */
    global_cond = 0;
    global_threshold = 5;
    global_ptr = (int*)&global_array[0];
    *global_ptr = 7;
    
    printf("Testing if-conversion coverage scenarios...\n");
    
    /* Test multiple times to exercise different paths */
    for (int i = 0; i < 10; i++) {
        global_cond = i % 3;  /* Vary the condition */
        
        /* Call target function with different inputs */
        total += target_if_conversion(i);
        total += test_pointer_condition();
        total += test_compound_condition(i, i * 2);
        
        /* Modify threshold to change condition outcomes */
        global_threshold = (i % 2 == 0) ? 3 : 8;
    }
    
    printf("Total result: %d\n", total);
    printf("Global array[0]: %d\n", global_array[0]);
    
    return total > 0 ? 0 : 1;
}
