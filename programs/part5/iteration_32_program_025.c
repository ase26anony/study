/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;
static int data_array[10] = {0};

/* Function to prevent optimizations */
static int __attribute__((noinline, noclone)) 
helper_func(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x * 2;
}

/* Target function with the specific if-then-else structure */
static int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int a, int b) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if ((global_cond < threshold) && (global_ptr != NULL) && (*global_ptr > 0)) {
        /* 
         * This is the 'then' block header.
         * GCC will generate: 
         * 1. A label for the block start
         * 2. Possibly NOTE/DEBUG_INSN instructions from asm statements
         * 3. The modification of global_cond BEFORE then_last_head
         */
        
        /* Generate NOTE/DEBUG_INSN instructions in header */
        asm volatile("# DEBUG/NOTE: Start of then block header" : : : "memory");
        asm volatile("# Another note in header" : : : "memory");
        
        /* CRITICAL: Modify condition variable in the header before any real instruction */
        global_cond = a + b;  /* This modifies test_expr used in the condition */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional statements to create a multi-instruction block */
        result = helper_func(a);
        *global_ptr = result;
        data_array[0] = b;
        result += helper_func(b);
        
        /* More operations to ensure block is substantial */
        for (int i = 0; i < 3; i++) {
            data_array[i] = result + i;
        }
    } else {
        /* Else block with different operations */
        result = helper_func(b - a);
        if (global_ptr) {
            *global_ptr = result;
        }
    }
    
    return result;
}

/* Another test case with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int x) {
    static int local_static = 50;
    int* ptr = &local_static;
    int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr > 0 && global_cond < 200) {
        /* Header with notes and modification */
        asm volatile("# Note: pointer test header" : : : "memory");
        
        /* Modify through pointer - affects condition */
        *ptr = x * 2;  /* Modifies *ptr used in condition */
        
        asm volatile("" : : : "memory");
        
        /* Body of then block */
        result = helper_func(*ptr);
        global_cond += result;
        data_array[1] = result;
        
        /* Additional operations */
        for (int i = 0; i < 5; i++) {
            data_array[i % 5] = result + i;
        }
    } else {
        result = helper_func(x);
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
static int __attribute__((noinline, noclone))
test_compound_condition(int a, int b, int c) {
    volatile int cond1 = a;
    static int cond2 = 0;
    int result = 0;
    
    /* Compound condition */
    if ((cond1 > 10) && (cond2 < 50) && (global_cond > 0)) {
        /* Header section */
        asm volatile("# Compound condition header note 1" : : : "memory");
        asm volatile("# Compound condition header note 2" : : : "memory");
        
        /* Modify cond2 which is used in the condition */
        cond2 = b + c;  /* This affects the condition */
        
        asm volatile("" : : : "memory");
        
        /* Block body */
        result = helper_func(cond1 + cond2);
        global_cond = result % 100;
        
        /* More operations */
        int temp = result;
        for (int i = 0; i < 4; i++) {
            temp = helper_func(temp);
            data_array[i] = temp;
        }
        result = temp;
    } else {
        result = helper_func(a + b + c);
        cond2 = result / 2;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    int test_data = 1;
    
    /* Initialize global data */
    global_ptr = &data_array[5];
    *global_ptr = 42;
    global_cond = 75;
    threshold = 150;
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Multiple calls to ensure if-conversion pass is invoked */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to exercise different paths */
        global_cond = 50 + (i * 10);
        *global_ptr = (i % 3) ? 10 : -5;
        
        /* Call test functions with varying parameters */
        results[0] += test_if_conversion(i, i * 2);
        results[1] += test_pointer_modification(i * 3);
        results[2] += test_compound_condition(i, i + 1, i + 2);
        
        /* Change threshold occasionally */
        if (i % 4 == 0) {
            threshold = 100 + (i * 5);
        }
    }
    
    printf("Results: %d, %d, %d\n", results[0], results[1], results[2]);
    printf("Final global_cond: %d\n", global_cond);
    printf("Final *global_ptr: %d\n", *global_ptr);
    
    return 0;
}
