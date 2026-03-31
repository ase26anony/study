/* ifcvt_test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 5;
static volatile int* global_ptr = NULL;
static int global_array[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_value(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x;
}

/* Function with complex condition that will be modified in the then-block header */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_if_conversion(int x, int y) {
    volatile int local_cond = x;
    int* ptr = &global_array[0];
    int result = 0;
    
    /* Complex condition using multiple expressions */
    if ((global_cond > global_threshold) && 
        (local_cond < y) && 
        (*ptr != 0) &&
        (global_ptr != NULL)) {
        
        /* HEADER START - This is the critical section */
        
        /* First, generate some notes/debug info */
        asm volatile("# DEBUG/NOTE: Entering then block");
        
        /* This is the key: modify the condition expression BEFORE any real instruction */
        /* We modify global_cond which is part of the complex condition */
        global_cond = x + 1;  /* MODIFIES CONDITION EXPRESSION */
        
        /* Add another note to ensure we're still in header */
        asm volatile("# Another note in header");
        
        /* HEADER END - Now real instructions begin */
        
        /* Real work after header */
        result = use_value(x) * 2;
        *ptr = result;
        
        /* More operations to ensure block has substance */
        for (int i = 0; i < 3; i++) {
            global_array[i] += result;
        }
        
        /* Call a function to create more instructions */
        result = use_value(result + y);
        
    } else {
        /* Else block with different computation */
        result = use_value(y) * 3;
        if (global_ptr) {
            *global_ptr = result;
        }
    }
    
    /* Ensure result is used */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Another test case with pointer-based condition */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_pointer_modification(int x) {
    static volatile int data = 0;
    volatile int* cond_ptr = &data;
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*cond_ptr > x && global_cond < 100) {
        
        /* Header with notes */
        asm volatile("# Note: pointer test header");
        
        /* Modify through the pointer - changes the condition! */
        *cond_ptr = x * 2;  /* MODIFIES DEREFERENCED CONDITION */
        
        asm volatile("# End of header notes");
        
        /* Real work */
        result = use_value(x) + *cond_ptr;
        global_array[0] = result;
        
        /* Complex computation to create more instructions */
        for (int i = 1; i < 5; i++) {
            global_array[i] = global_array[i-1] + i;
        }
        
    } else {
        result = use_value(x) - 10;
    }
    
    return result;
}

/* Test with function call in condition */
__attribute__((noinline, noclone))
static int get_condition_value(void) {
    return global_cond;
}

__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_function_condition(int x) {
    int local_var = x;
    int result = 0;
    
    /* Condition with function call */
    if (get_condition_value() > local_var && global_threshold < 20) {
        
        /* Header section */
        asm volatile("# Function condition header");
        
        /* Modify global_cond which affects get_condition_value() */
        global_cond = local_var - 1;  /* MODIFIES CONDITION */
        
        asm volatile("# More header notes");
        
        /* Real instructions */
        result = use_value(local_var) * 4;
        
        /* Create more complexity */
        for (int i = 0; i < 10; i += 2) {
            global_array[i % 10] = result + i;
        }
        
    } else {
        result = use_value(local_var) / 2;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 10;  /* Start with condition true */
    global_threshold = 3;
    global_array[0] = 1;  /* Make *ptr != 0 true */
    global_ptr = &global_array[0];
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Test multiple times to ensure if-conversion pass runs */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to exercise different paths */
        int x = i * 2;
        int y = i + 5;
        
        /* Call test functions with varying conditions */
        results[0] += test_if_conversion(x, y);
        results[1] += test_pointer_modification(x);
        results[2] += test_function_condition(y);
        
        /* Modify globals to change branch behavior */
        global_cond = (i % 3) * 5;
        global_threshold = (i % 2) ? 2 : 8;
        global_array[0] = (i % 4) ? 1 : 0;
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = results[0] + results[1] + results[2];
    printf("Final result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
