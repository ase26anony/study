/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_value(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x;
}

/* Function with the targeted if-then-else structure */
static int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Initialize pointer to global data */
    ptr = &data[0];
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *ptr == 0) {
        /* 
         * This is the 'then' block header.
         * GCC will generate: 
         * 1. A label for the block start
         * 2. Possibly NOTE/DEBUG_INSN instructions from asm statements
         * 3. The modification of global_cond BEFORE then_last_head
         */
        
        /* Generate NOTE/DEBUG_INSN with asm comments */
        asm volatile("# HEADER: Beginning of then block");
        asm volatile("# DEBUG: global_cond = %0" : : "r"(global_cond));
        
        /* CRITICAL: Modify the condition variable in the header */
        /* This should happen before then_last_head is reached */
        global_cond = x;  /* Modifies test_expr used in condition */
        
        /* Generate more notes to ensure we're in header section */
        asm volatile("# MODIFICATION DONE");
        asm volatile("# DEBUG: post-modification");
        
        /* 
         * Additional instructions to create a multi-instruction block
         * These come after the modification but are part of the same BB
         */
        local_mod = y * 2;
        result = use_value(local_mod);
        
        /* More operations to prevent block simplification */
        data[1] = result;
        data[2] = global_cond + 1;
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
    } else {
        /* Else block with different behavior */
        result = use_value(y);
        global_cond = y - 1;
        data[0] = result;
    }
    
    /* Use result to prevent dead code elimination */
    return result + global_cond;
}

/* Alternate test with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int seed) {
    static volatile int static_var = 0;
    volatile int* volatile local_ptr = &static_var;
    int temp = 0;
    
    /* Condition using pointer dereference */
    if (*local_ptr < seed && global_cond > 0) {
        /* Header with notes and modification */
        asm volatile("# TEST 2 HEADER");
        asm volatile("# Note instruction 1");
        
        /* Modify through pointer - affects condition expression */
        *local_ptr = seed * 2;  /* Modifies *local_ptr used in condition */
        
        asm volatile("# Note instruction 2");
        asm volatile("# Modification complete");
        
        /* Body of then block */
        temp = seed * 3;
        for (int i = 0; i < 3; i++) {
            data[i] = temp + i;
        }
        
        return temp;
    }
    
    return seed;
}

/* Test with function call in condition */
extern int __attribute__((noinline)) get_value(void);

static int __attribute__((noinline, noclone))
test_function_condition(int base) {
    volatile int counter = base;
    
    /* Condition with function call */
    if ((get_value() > 0) && (counter < 100)) {
        /* Header section */
        asm volatile("# FUNCTION CONDITION HEADER");
        
        /* Modify variable used in condition */
        counter = base * 2;  /* Modifies counter in condition */
        
        asm volatile("# Counter modified");
        
        /* Block body */
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += data[i];
        }
        
        return sum + counter;
    }
    
    return base;
}

int get_value(void) {
    static int val = 0;
    return val++;
}

int main(void) {
    int total = 0;
    
    /* Initialize data */
    for (int i = 0; i < 10; i++) {
        data[i] = i;
    }
    
    /* Call test functions multiple times with different inputs
     * to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 100; i++) {
        global_cond = i % 10;
        threshold = 5;
        ptr = &data[i % 10];
        
        /* Vary inputs to take both branches */
        total += test_if_conversion(i, i * 2);
        total += test_pointer_modification(i);
        total += test_function_condition(i);
        
        /* Alternate global_cond to affect condition */
        global_cond = (i % 3 == 0) ? 10 : 2;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
