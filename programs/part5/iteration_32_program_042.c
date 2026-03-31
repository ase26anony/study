/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile ptr_cond = NULL;
static int data_array[4] = {0, 0, 0, 0};

/* Function to prevent optimizations */
static int __attribute__((noinline, noclone)) 
side_effect(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

/* Target function with the specific if-then-else structure */
static int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if ((global_cond > 0) && (ptr_cond != NULL) && (*ptr_cond < 100)) {
        /* Block header starts here (GCC will generate a label) */
        
        /* Generate NOTE/DEBUG_INSN instructions using asm comments */
        asm volatile("# HEADER: Debug note 1");
        asm volatile("# HEADER: Debug note 2");
        
        /* CRITICAL: Modify condition variable BEFORE first real instruction
           This should be in the block header before then_last_head */
        global_cond = x;  /* Modifies variable used in condition */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional instructions to create a multi-instruction block */
        int temp = side_effect(y);
        data_array[0] = temp;
        result = temp * 2;
        
        /* More operations to ensure block is non-trivial */
        for (int i = 0; i < 2; i++) {
            data_array[i] += result;
        }
        
        asm volatile("# FOOTER: End of then block");
    } else {
        /* Else block with different operations */
        result = y - x;
        if (ptr_cond) {
            *ptr_cond = result;
        }
        asm volatile("# Else block executed");
    }
    
    return result;
}

/* Alternate test with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int threshold) {
    static int local_static = 50;
    int* ptr = &local_static;
    int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr > threshold && global_cond != 0) {
        /* Header with notes */
        asm volatile("# Note before modification");
        
        /* Modify through pointer - affects condition expression */
        *ptr = threshold - 1;  /* Modifies memory used in condition */
        
        asm volatile("" : : : "memory");
        
        /* Real work */
        result = side_effect(*ptr);
        data_array[1] = result;
        
        asm volatile("# Note after work");
    } else {
        result = threshold * 2;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
static int __attribute__((noinline, noclone))
test_compound_condition(int a, int b) {
    volatile int cond1 = a;
    static int cond2 = 0;
    int result = 0;
    
    /* Compound condition */
    if (cond1 > 10 && cond2 < 20) {
        /* Notes/debug insns in header */
        asm volatile("# Compound condition block start");
        
        /* Modify second part of compound condition */
        cond2 = b;  /* Affects cond2 in condition */
        
        asm volatile("" : : : "memory");
        
        /* Additional instructions */
        result = a * b + cond2;
        for (int i = 0; i < 3; i++) {
            data_array[i] += i;
        }
    } else {
        result = a + b;
        cond2 = result;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 5;
    ptr_cond = &data_array[0];
    *ptr_cond = 50;
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Test 1: Global variable modification in header */
    printf("Test 1: ");
    results[0] = test_if_conversion(10, 20);
    printf("Result: %d, Global: %d\n", results[0], global_cond);
    
    /* Test 2: Pointer-based modification */
    printf("Test 2: ");
    results[1] = test_pointer_modification(40);
    printf("Result: %d\n", results[1]);
    
    /* Test 3: Compound condition */
    printf("Test 3: ");
    results[2] = test_compound_condition(15, 25);
    printf("Result: %d\n", results[2]);
    
    /* Test with different conditions to exercise both paths */
    global_cond = -1;  /* Force else path */
    printf("Test 4 (else path): ");
    int else_result = test_if_conversion(5, 5);
    printf("Result: %d\n", else_result);
    
    /* Verify no array bounds issues */
    printf("Final array: [%d, %d, %d, %d]\n", 
           data_array[0], data_array[1], data_array[2], data_array[3]);
    
    return 0;
}
