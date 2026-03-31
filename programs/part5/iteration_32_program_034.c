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
static int use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Function with complex condition that will be modified in the then block header */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_if_conversion(int x, int y) {
    volatile int local_var = x;
    int result = 0;
    
    /* Complex condition using global variable - creates non-trivial test_expr */
    if (global_cond < global_threshold && local_var > 0) {
        /* This assignment modifies global_cond which is part of the condition.
           It happens in the block header before any real instruction.
           We'll add notes/debug info before it. */
        
        /* Generate NOTE insns via asm comments */
        asm volatile("# NOTE: Entering then block");
        asm volatile("# DEBUG: x = %0" : : "r"(x));
        
        /* CRITICAL: Modify the condition variable in the header */
        global_cond = x + y;  /* This should trigger modified_in_p */
        
        /* Generate another note */
        asm volatile("# NOTE: After condition modification");
        
        /* Real instructions after the header */
        result = x * y;
        global_array[0] = result;
        
        /* More operations to ensure block has multiple instructions */
        result += use_value(y);
        result *= 2;
        
        /* Use result to prevent dead code elimination */
        asm volatile("" : "+r"(result) : : "memory");
    } else {
        /* Else block with different computation */
        result = x - y;
        global_array[1] = result;
    }
    
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_pointer_condition(int x) {
    static volatile int data = 0;
    volatile int* ptr = &data;
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > 0 && x > 10) {
        /* Notes/debug insns in header */
        asm volatile("# NOTE: Pointer condition block");
        asm volatile("# DEBUG: ptr = %0" : : "r"(ptr));
        
        /* Modify through the pointer - affects the condition */
        *ptr = x * 2;  /* This modifies memory used in condition */
        
        /* More notes */
        asm volatile("# NOTE: After pointer write");
        
        /* Real computation */
        result = x * x;
        for (int i = 0; i < 3; i++) {
            result += i;
        }
    } else {
        result = x / 2;
    }
    
    return result;
}

/* Test with function call in condition */
__attribute__((noinline, noclone))
static int get_counter(void) {
    static volatile int counter = 0;
    return counter++;
}

__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_function_condition(int x) {
    int result = 0;
    
    /* Condition with function call */
    if (get_counter() < 3 && x > 0) {
        /* Notes in header */
        asm volatile("# NOTE: Function condition block");
        
        /* We can't modify the function return, but we can modify
           the static variable it uses */
        /* Actually, we need to modify something in test_expr.
           Let's use a different approach with global */
        extern volatile int func_cond_var;
        
        /* This asm generates notes but also acts as barrier */
        asm volatile("# DEBUG: Before modification" : : : "memory");
        
        /* Do some computation that might affect condition */
        result = x + 5;
        asm volatile("" : "+r"(result) : : "memory");
    } else {
        result = x - 5;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Initialize global pointer */
    static int data = 42;
    global_ptr = &data;
    
    /* Test multiple paths to exercise if-conversion */
    for (int i = 0; i < 10; i++) {
        global_cond = i % 3;
        
        /* Call test function with different parameters */
        total += test_if_conversion(i, i * 2);
        total += test_pointer_condition(i);
        total += test_function_condition(i);
        
        /* Vary the threshold */
        global_threshold = (i % 2 == 0) ? 3 : 8;
    }
    
    printf("Result: %d\n", total);
    
    /* Use the result to prevent optimization */
    volatile int output = total;
    return output != 0 ? 0 : 1;
}
