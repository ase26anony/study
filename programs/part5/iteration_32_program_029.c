/* ifcvt_test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int global_modifier = 1;
static volatile int* global_ptr = NULL;
static volatile int global_array[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_value(int x) {
    volatile int sink = x;
    return sink;
}

/* Function with complex condition that will be modified in the then block header */
__attribute__((noinline, noclone))
int test_if_conversion(int x, int y) {
    volatile int local_var = x;
    volatile int* local_ptr = &local_var;
    int result = 0;
    
    /* Complex condition using multiple expressions */
    /* This should generate a non-trivial test_expr in RTL */
    if ((global_cond > 0 && *local_ptr < 100) || 
        (global_array[2] != 0 && y > 50)) {
        
        /* This is the critical part: modify a variable used in the condition
           BEFORE any real instruction in the then block */
        
        /* First, generate some NOTE/DEBUG_INSN instructions */
        /* These won't count as "real" instructions for then_last_head */
        asm volatile("# DEBUG/NOTE: Start of then block");
        asm volatile("# Another note instruction");
        
        /* NOW modify a condition variable - this should be in the header
           and detected by modified_in_p */
        global_cond = 10;  /* This modifies test_expr! */
        
        /* Generate more notes/debug to ensure we're still in header */
        asm volatile("# More notes before real work");
        
        /* Now do real work - this ends the header */
        result = x + y + global_modifier;
        
        /* More code to ensure block has substance */
        global_array[0] = result;
        result = use_value(result * 2);
        
        /* Additional modification to ensure block isn't optimized away */
        global_ptr = &global_array[0];
        *global_ptr = result % 100;
        
    } else {
        /* Else block with different computation */
        result = x - y;
        global_cond = -1;
        global_array[1] = result;
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (result) : : "memory");
    return result;
}

/* Another test case with pointer-based condition */
__attribute__((noinline, noclone))
int test_pointer_condition(int threshold) {
    static volatile int data[5] = {1, 2, 3, 4, 5};
    volatile int* ptr = &data[2];
    int sum = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > threshold && global_modifier == 1) {
        /* Notes/debug instructions first */
        asm volatile("# Pointer condition then block");
        asm volatile("# Note instruction 1");
        asm volatile("# Note instruction 2");
        
        /* Modify the pointer dereference source */
        *ptr = threshold - 1;  /* This affects test_expr! */
        
        /* More notes */
        asm volatile("# After modification note");
        
        /* Real work begins here */
        for (int i = 0; i < 5; i++) {
            sum += data[i];
        }
        
        /* Additional computation */
        sum = sum * global_modifier;
        global_cond = sum;
        
    } else {
        sum = threshold * 2;
        data[0] = sum;
    }
    
    return sum;
}

/* Test with function call in condition */
__attribute__((noinline, noclone))
static int get_value(void) {
    return global_modifier * 2;
}

__attribute__((noinline, noclone))
int test_function_condition(int x) {
    int val = x;
    
    /* Condition with function call */
    if (get_value() > 0 && global_cond < 100) {
        /* Header with notes */
        asm volatile("# Function condition header");
        
        /* Modify global_cond used in condition */
        global_cond = 200;  /* Affects test_expr */
        
        /* More notes */
        asm volatile("# After global_cond modification");
        
        /* Real work */
        val = val * 3;
        global_array[3] = val;
        
        /* Complex computation to prevent optimization */
        for (int i = 0; i < 3; i++) {
            val += global_array[i];
        }
        
    } else {
        val = x / 2;
        global_cond = 50;
    }
    
    return val;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 5;
    global_modifier = 2;
    global_array[2] = 3;
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and ensure if-conversion runs */
    
    /* Test 1: Take the then branch */
    results[0] = test_if_conversion(10, 60);
    
    /* Test 2: Take the else branch */
    global_cond = -5;
    results[1] = test_if_conversion(10, 40);
    
    /* Test 3: Pointer-based condition */
    results[2] = test_pointer_condition(2);
    
    /* Test 4: Function call condition */
    global_cond = 30;
    int result4 = test_function_condition(20);
    
    /* Use results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += results[i];
    }
    total += result4;
    
    printf("Total: %d\n", total);  /* Prevent dead code elimination */
    
    return total > 0 ? 0 : 1;
}
