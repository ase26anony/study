/* ifcvt-test.c - Test case for GCC if-conversion pass */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fdump-rtl-all ifcvt-test.c -o ifcvt-test */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 10;
static volatile int* global_ptr = NULL;
static volatile int global_data[10] = {0};

/* Function to prevent optimization */
int __attribute__((noinline, noclone)) 
use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Force note/debug instructions in the header */
#define FORCE_NOTE() asm volatile("# HEADER NOTE %0" : : "i" (__LINE__))
#define FORCE_DEBUG() asm volatile("/* DEBUG at line %0 */" : : "i" (__LINE__))

/* Target function with specific if-then-else structure */
int __attribute__((noinline, noclone))
test_if_conversion(int x, int y) {
    int result = 0;
    static int static_counter = 0;
    
    /* Complex condition using global variable */
    /* This should create a non-trivial test_expr */
    if (global_cond > global_threshold && 
        (x > y || global_ptr != NULL)) {
        
        /* Block header starts here (label will be generated) */
        FORCE_NOTE();    /* Generates NOTE insn */
        FORCE_DEBUG();   /* Generates DEBUG_INSN */
        FORCE_NOTE();    /* Another NOTE */
        
        /* CRITICAL: Modify condition variable BEFORE any real instruction */
        /* This should be in the header before then_last_head */
        global_cond = x - y;  /* Modifies test_expr component */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Real work after the modification */
        result = x * y + global_cond;
        
        /* More operations to ensure block has multiple instructions */
        global_data[0] = result;
        static_counter++;
        result += use_value(static_counter);
        
        FORCE_NOTE();  /* Another note after real instructions */
        
    } else {
        /* Else branch with different computation */
        result = y - x;
        if (global_ptr) {
            result += *global_ptr;
        }
    }
    
    return result;
}

/* Another test case with pointer-based condition */
int __attribute__((noinline, noclone))
test_pointer_condition(int* ptr1, int* ptr2) {
    int local_data[5] = {1, 2, 3, 4, 5};
    int result = 0;
    
    /* Condition involving pointer dereference */
    if (ptr1 && *ptr1 > 0 && ptr2 && global_cond < 100) {
        
        /* Header with notes/debug */
        FORCE_DEBUG();
        FORCE_NOTE();
        asm volatile("# Another note %0" : : "i" (__LINE__));
        
        /* Modify condition component - dereference of ptr1 */
        /* This might be seen as modifying test_expr */
        *ptr1 = global_cond;  /* Modifies memory referenced in condition */
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Real work */
        result = *ptr1 + *ptr2;
        global_threshold = result;
        result += use_value(local_data[0]);
        
    } else {
        result = -1;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
int __attribute__((noinline, noclone))
test_compound_condition(int a, int b, int c) {
    volatile int local_cond = a;
    int result = 0;
    
    /* Compound condition with local variable */
    if ((local_cond > b) && (c < global_threshold) && (global_cond != 0)) {
        
        /* Header section */
        FORCE_NOTE();
        FORCE_DEBUG();
        FORCE_NOTE();
        
        /* Modify part of the condition */
        local_cond = b;  /* Modifies test_expr component */
        
        /* Ensure instruction isn't optimized away */
        asm volatile("" : : "r"(local_cond) : "memory");
        
        /* Additional instructions */
        result = a + b + c;
        for (int i = 0; i < 3; i++) {
            result += i;
        }
        global_data[1] = result;
        
    } else {
        result = a - b - c;
    }
    
    return result;
}

int main(void) {
    int results[10] = {0};
    int data1 = 5, data2 = 15;
    int test_data[3] = {20, 30, 40};
    
    /* Initialize globals */
    global_cond = 20;  /* > threshold */
    global_threshold = 10;
    global_ptr = &data1;
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Multiple calls to trigger if-conversion */
    for (int i = 0; i < 5; i++) {
        /* Vary inputs to exercise different paths */
        global_cond = 5 + i * 5;
        data1 = 10 + i;
        data2 = 20 - i;
        
        /* Call test functions */
        results[0] = test_if_conversion(data1, data2);
        results[1] = test_pointer_condition(&data1, &data2);
        results[2] = test_compound_condition(data1, data2, i);
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: %d, %d, %d\n", 
               i, results[0], results[1], results[2]);
    }
    
    /* Additional test with NULL pointer */
    global_ptr = NULL;
    results[3] = test_if_conversion(100, 50);
    results[4] = test_pointer_condition(NULL, &data2);
    
    printf("Final results: %d, %d\n", results[3], results[4]);
    
    return 0;
}
