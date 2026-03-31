/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Function with the targeted if-then-else structure */
static int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if ((global_cond > threshold) && (global_ptr != NULL) && (*global_ptr < x)) {
        /* 
         * This is the 'then' block header section.
         * The first non-label, non-note, non-debug instruction
         * that modifies the condition expression should be here.
         */
        
        /* Generate NOTE/DEBUG_INSN instructions first */
        asm volatile("# THEN BLOCK HEADER - NOTE 1");
        asm volatile("# THEN BLOCK HEADER - NOTE 2");
        
        /* 
         * CRITICAL: Modify variable used in condition BEFORE any other
         * real instruction. This should make modified_in_p return true.
         */
        global_cond = y;  /* Modifies condition expression */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional instructions to form a proper basic block */
        result = x * y + 42;
        *global_ptr = result;
        result = use_value(result);
        
        asm volatile("# THEN BLOCK FOOTER");
    } else {
        /* 'else' block */
        result = x - y;
        if (global_ptr) {
            *global_ptr = result;
        }
    }
    
    return result;
}

/* Another test case with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int a, int b) {
    static int static_var = 50;
    int* ptr = &static_var;
    int result = 0;
    
    /* Condition using pointer dereference */
    if ((*ptr > a) && (global_cond < b)) {
        /* Header with notes/debug insns */
        asm volatile("# POINTER TEST HEADER");
        
        /* Modify through pointer - affects condition */
        *ptr = b + 10;  /* Modifies *ptr used in condition */
        
        /* More instructions */
        result = a + b + *ptr;
        global_cond = result % 100;
        
        asm volatile("# POINTER TEST FOOTER");
    } else {
        result = a - b;
        static_var = result;
    }
    
    return result;
}

/* Test with function call in condition */
extern int __attribute__((noinline)) get_value(void);

static int __attribute__((noinline, noclone))
test_function_condition(int x) {
    volatile int local_cond = x;
    int result = 0;
    
    if ((get_value() > 0) && (local_cond < 100)) {
        /* Notes first */
        asm volatile("# FUNC COND HEADER NOTE");
        
        /* Modify local_cond which is part of condition */
        local_cond = 200;  /* This might be tracked differently */
        
        result = x * 2;
        asm volatile("# FUNC COND BODY");
    } else {
        result = x / 2;
    }
    
    return result;
}

int get_value(void) {
    static int counter = 0;
    return ++counter;
}

int main(void) {
    int array[10];
    int i, total = 0;
    
    /* Initialize global variables */
    global_cond = 75;
    global_ptr = &array[0];
    array[0] = 25;
    
    /* Call test functions multiple times to exercise different paths */
    for (i = 0; i < 10; i++) {
        int val1, val2, val3;
        
        /* Vary inputs to take different branches */
        if (i % 3 == 0) {
            global_cond = 150;  /* Will take then-block */
            array[0] = 10;
        } else if (i % 3 == 1) {
            global_cond = 50;   /* Will take else-block */
            array[0] = 100;
        } else {
            global_cond = 75;   /* Borderline case */
            array[0] = 50;
        }
        
        /* Test different functions */
        val1 = test_if_conversion(i * 20, i * 10);
        val2 = test_pointer_modification(i * 5, i * 3);
        val3 = test_function_condition(i * 7);
        
        total += val1 + val2 + val3;
        
        /* Change pointer target */
        global_ptr = &array[i % 5];
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with different optimization barriers */
    {
        volatile int a = 100, b = 200;
        int r1 = test_if_conversion(a, b);
        int r2 = test_pointer_modification(b, a);
        printf("Additional tests: %d, %d\n", r1, r2);
    }
    
    return total != 0 ? 0 : 1;
}
