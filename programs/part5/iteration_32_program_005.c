/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -march=x86-64 ifcvt-test.c -o ifcvt-test */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
dummy_work(int x) {
    return x * 2 + 1;
}

/* Function with the targeted if-then-else structure */
static int __attribute__((noinline, noclone))
test_if_conversion(int a, int b, int* out) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *global_ptr != 0) {
        /* 
         * HEADER SECTION OF THEN BLOCK:
         * 1. Label (implicit - start of then block)
         * 2. DEBUG_INSN/NOTE instructions (from asm comments)
         * 3. Modification of condition variable BEFORE first real instruction
         */
        
        /* Generate NOTE/DEBUG_INSN instructions via asm comments */
        asm volatile("# DEBUG/NOTE: Entering then block");
        asm volatile("# Another debug note");
        
        /* CRITICAL: Modify condition variable in header before any real instruction */
        /* This should make modified_in_p(test_expr, insn) return true */
        global_cond = 50;  /* Direct modification of condition variable */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* 
         * Now the "real" instructions begin - this marks then_last_head
         * The loop in ifcvt.cc checks from BB_HEAD to then_last_head
         */
        int temp = dummy_work(a);
        
        /* Additional instructions to create a multi-instruction block */
        *out = temp + b;
        result = *global_ptr * 3;
        
        asm volatile("# DEBUG/NOTE: More work in then block");
        result += dummy_work(b);
    } else {
        /* Else block with different computation */
        result = dummy_work(b) - a;
        *out = result;
        
        /* Also modify global_cond here to ensure different paths */
        global_cond = 200;
    }
    
    return result;
}

/* Another test case with pointer-based condition */
static int __attribute__((noinline, noclone))
test_pointer_modification(int x) {
    static int data[4] = {10, 20, 30, 40};
    int* ptr = &data[0];
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*ptr > 15 && x > 0) {
        /* Header with debug notes */
        asm volatile("# Test 2: Then block header");
        asm volatile("# Multiple notes before modification");
        
        /* Modify the dereferenced pointer value */
        *ptr = 5;  /* This modifies the memory used in condition */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Real instructions begin here */
        result = x * 2;
        data[1] = result;
        
        asm volatile("# More work");
        result += dummy_work(x);
    } else {
        result = x / 2;
        data[2] = result;
    }
    
    return result;
}

/* Test with compound condition where only part is modified */
static int __attribute__((noinline, noclone))
test_partial_modification(int a, int b) {
    static int counter = 0;
    volatile int local_cond = a;
    
    /* Compound condition with local and static variables */
    if (local_cond > 0 && counter < 10) {
        /* Debug notes in header */
        asm volatile("# Test 3: Header notes 1");
        asm volatile("# Test 3: Header notes 2");
        asm volatile("# Test 3: Header notes 3");
        
        /* Modify part of the condition (counter) */
        counter = 15;  /* This modifies counter used in condition */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Real work */
        int r1 = dummy_work(a);
        int r2 = dummy_work(b);
        return r1 + r2;
    }
    
    return a - b;
}

int main(void) {
    int results[3];
    int output;
    
    /* Initialize globals */
    static int init_data = 25;
    global_ptr = &init_data;
    global_cond = 150;  /* Will satisfy condition for first test */
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Test 1: Global variable modification in then block header */
    printf("Test 1: ");
    results[0] = test_if_conversion(10, 20, &output);
    printf("Result: %d, Output: %d, Global: %d\n", 
           results[0], output, global_cond);
    
    /* Test 2: Pointer modification in then block header */
    printf("Test 2: ");
    results[1] = test_pointer_modification(30);
    printf("Result: %d\n", results[1]);
    
    /* Test 3: Partial condition modification */
    printf("Test 3: ");
    results[2] = test_partial_modification(5, 3);
    printf("Result: %d\n", results[2]);
    
    /* Run again with different inputs to exercise both paths */
    global_cond = 50;  /* Will NOT satisfy condition for first test */
    printf("\nRunning with different conditions...\n");
    
    results[0] = test_if_conversion(5, 15, &output);
    printf("Test 1 (else path): %d\n", results[0]);
    
    results[1] = test_pointer_modification(-5);
    printf("Test 2 (else path): %d\n", results[1]);
    
    results[2] = test_partial_modification(-1, 2);
    printf("Test 3 (else path): %d\n", results[2]);
    
    return 0;
}
