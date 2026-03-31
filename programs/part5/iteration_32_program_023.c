/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 5;
static int data_array[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_value(int x) {
    volatile int dummy = x;
    return dummy;
}

/* Target function with the specific if-then-else structure */
__attribute__((noinline, noclone, optimize("O2")))
int test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *global_ptr != 0) {
        /* 
         * Header section begins here (BB_HEAD)
         * First instruction in the block modifies the condition expression
         * This should be before then_last_head
         */
        global_cond = x;  /* MODIFIES condition variable immediately */
        
        /* Insert notes/debug insns via asm comments */
        asm volatile("# NOTE: Beginning of then block header");
        asm volatile("# DEBUG: x = %0" : : "r"(x));
        
        /* Additional statements to create a multi-instruction block */
        result = x * y;
        data_array[0] = result;
        
        /* More operations to ensure block isn't trivial */
        for (int i = 1; i < 4; i++) {
            data_array[i] = data_array[i-1] + i;
        }
        
        /* Function call to prevent merging */
        result = use_value(result);
        
        asm volatile("# NOTE: End of then block operations");
    } else {
        /* Else block with different operations */
        result = y - x;
        global_ptr = &result;
        asm volatile("# NOTE: Else block executed");
    }
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Alternative test with different condition structure */
__attribute__((noinline, noclone))
int test_pointer_modification(int val) {
    static int local_static = 0;
    int* ptr = &local_static;
    int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr > 0 && global_cond < 10) {
        /* Modify through pointer - affects condition expression */
        *ptr = val;  /* This modifies the dereferenced value in condition */
        
        /* Debug/note instructions */
        asm volatile("# DEBUG: Modifying pointer target");
        asm volatile("# NOTE: ptr = %0" : : "r"(ptr));
        
        /* Additional operations */
        result = val * 2;
        for (int i = 0; i < 3; i++) {
            result += i;
            asm volatile("# Loop iteration %0" : : "i"(i));
        }
    } else {
        result = val / 2;
        local_static++;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b, int c) {
    volatile int cond1 = a;
    static int cond2 = 0;
    int result = 0;
    
    /* Compound condition */
    if (cond1 > b && cond2 < c) {
        /* Modify one part of the compound condition */
        cond2 = a + b;  /* Modifies cond2 used in condition */
        
        /* Multiple asm notes */
        asm volatile("# NOTE: Compound condition block");
        asm volatile("# DEBUG: cond2 modified");
        asm volatile("# Line marker note");
        
        /* Substantial computation */
        result = a * b * c;
        for (int i = 0; i < 5; i++) {
            result += data_array[i % 10];
        }
        
        /* Another asm to ensure notes in header */
        asm volatile("# Computation complete");
    } else {
        result = a + b + c;
        cond2 = 0;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 3;
    global_ptr = &global_cond;
    
    /* Allocate and initialize data for pointer test */
    int* heap_ptr = (int*)malloc(sizeof(int));
    *heap_ptr = 7;
    global_ptr = heap_ptr;
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Multiple calls to ensure if-conversion pass runs */
    for (int i = 0; i < 5; i++) {
        global_cond = i;
        threshold = i + 1;
        
        /* Call test functions with varying inputs */
        results[0] += test_if_conversion(i, i * 2);
        results[1] += test_pointer_modification(i * 3);
        results[2] += test_compound_condition(i, i + 1, i + 2);
        
        /* Modify global pointer occasionally */
        if (i % 2 == 0) {
            global_ptr = &data_array[0];
            *heap_ptr = i * 5;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", results[0], results[1], results[2]);
    
    free(heap_ptr);
    return 0;
}
