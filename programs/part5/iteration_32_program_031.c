/* ifcvt-test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;

/* Barrier function to prevent optimization */
static void barrier(void) {
    asm volatile("" : : : "memory");
}

/* Function with attribute to prevent inlining and cloning */
__attribute__((noinline, noclone, optimize("O2")))
int test_if_conversion(int x, int y) {
    int result = 0;
    int local_var = x;
    
    /* Use a global variable in condition - creates non-trivial test_expr */
    if (global_cond < threshold) {
        /* This is the 'then' block (then_bb) */
        
        /* Generate NOTE/DEBUG_INSN instructions via asm comments */
        asm volatile("# HEADER: Beginning of then block");
        asm volatile("# DEBUG: x = %0" : : "r"(x));
        asm volatile("# NOTE: Testing if-conversion");
        
        /* CRITICAL: Modify the condition variable BEFORE any real instruction
           This should be in the header portion before then_last_head */
        global_cond = 50;  /* Modifies test_expr in header */
        
        /* Additional asm to ensure we have more NOTE/DEBUG_INSN */
        asm volatile("# MODIFIED: global_cond = 50");
        
        /* Real computation after the modification */
        result = x * y + 42;
        
        /* More code to ensure block has sufficient instructions */
        if (global_ptr != NULL) {
            *global_ptr = result;
        }
        
        barrier();
        result += global_cond;
        
    } else {
        /* Else block */
        result = x - y;
        global_cond += 10;
    }
    
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_pointer_condition(int* ptr1, int* ptr2) {
    int temp = 0;
    
    /* Complex condition with pointer dereference */
    if (ptr1 != NULL && *ptr1 > *ptr2 && global_cond < 200) {
        /* Header with notes/debug */
        asm volatile("# POINTER TEST HEADER");
        asm volatile("# DEBUG: ptr1 = %0, ptr2 = %1" : : "r"(ptr1), "r"(ptr2));
        
        /* Modify condition component in header */
        if (ptr1 != NULL) {
            *ptr1 = 0;  /* Modifies memory used in condition */
        }
        
        asm volatile("# MODIFIED: *ptr1 = 0");
        
        /* Real work */
        temp = *ptr2 * 3;
        global_ptr = ptr2;
        
    } else {
        temp = -1;
        if (ptr2 != NULL) {
            *ptr2 = 99;
        }
    }
    
    return temp;
}

/* Test with compound condition */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b, int c) {
    static int counter = 0;
    int ret = 0;
    
    /* Compound condition where one part gets modified */
    if (a > b && global_cond < c && counter < 5) {
        /* Header section */
        asm volatile("# COMPOUND CONDITION HEADER");
        asm volatile("# NOTE: counter = %0" : : "r"(counter));
        
        /* Modify part of the condition */
        counter++;  /* Modifies static variable used in condition */
        
        asm volatile("# INCREMENTED counter");
        
        /* Actual computation */
        ret = a * b * c;
        global_cond += ret % 100;
        
        /* More operations to enlarge block */
        for (int i = 0; i < 3; i++) {
            ret += i;
        }
        
    } else {
        ret = a + b + c;
        counter = 0;
    }
    
    return ret;
}

int main(void) {
    int results[3] = {0};
    int data1 = 10, data2 = 20;
    int* ptr1 = &data1;
    int* ptr2 = &data2;
    
    /* Initialize globals */
    global_cond = 0;
    global_ptr = &data1;
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Multiple calls to ensure if-conversion pass is invoked */
    for (int i = 0; i < 5; i++) {
        /* Vary inputs to exercise different paths */
        global_cond = i * 30;
        threshold = 75 + i * 5;
        
        /* Test 1: Global variable condition */
        results[0] += test_if_conversion(i * 10, i * 5);
        
        /* Test 2: Pointer-based condition */
        *ptr1 = i * 15;
        *ptr2 = i * 10;
        results[1] += test_pointer_condition(ptr1, ptr2);
        
        /* Test 3: Compound condition */
        results[2] += test_compound_condition(i, i*2, i*3);
        
        /* Change pointer to NULL sometimes */
        if (i % 2 == 0) {
            global_ptr = NULL;
        } else {
            global_ptr = &data2;
        }
    }
    
    printf("Results: %d, %d, %d\n", results[0], results[1], results[2]);
    
    /* Use results to prevent dead code elimination */
    if (results[0] > 1000 || results[1] > 1000 || results[2] > 1000) {
        printf("Some paths executed\n");
    }
    
    return 0;
}
