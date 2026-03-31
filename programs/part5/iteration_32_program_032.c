/* ifcvt_test.c - Test case for GCC if-conversion pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fdump-rtl-all ifcvt_test.c -o ifcvt_test */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 10;
static volatile int* global_ptr = NULL;
static volatile int global_array[10] = {0};

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Function with attribute to target specific architectures if needed */
#ifdef __x86_64__
__attribute__((target("default")))
#endif
static int __attribute__((noinline, noclone))
test_if_conversion(int x, int y) {
    volatile int result = 0;
    
    /* Complex condition using global variable - creates non-trivial RTL expression */
    if (global_cond > global_threshold) {
        /* This assignment modifies the condition variable IN THE HEADER
           before any real instruction in the then block */
        global_cond = x;  /* MODIFIES CONDITION VARIABLE */
        
        /* Generate NOTE/DEBUG_INSN instructions using asm statements */
        /* These should appear before then_last_head in RTL */
        asm volatile("# DEBUG/NOTE: Start of then block");
        asm volatile("# Another note instruction");
        
        /* Additional statements to create a multi-instruction block */
        result = x + y;
        global_array[0] = result;
        
        /* More operations to ensure block has sufficient instructions */
        result *= 2;
        global_ptr = &global_array[1];
        *global_ptr = result;
        
        /* Function call to create complex control flow */
        result = use_value(result);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    } else {
        /* Else block with different operations */
        result = x - y;
        global_cond = y;
    }
    
    return result;
}

/* Another test case with pointer-based condition */
static int __attribute__((noinline, noclone))
test_pointer_condition(int x) {
    static volatile int data = 5;
    volatile int* ptr = &data;
    volatile int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr > 0 && global_cond < 20) {
        /* Modify the dereferenced value - affects condition */
        *ptr = x;  /* MODIFIES CONDITION EXPRESSION */
        
        /* Generate notes/debug insns */
        asm volatile("# NOTE: Pointer modification block");
        
        /* Additional operations */
        result = *ptr * 2;
        global_array[2] = result;
        
        /* Complex operation sequence */
        for (int i = 0; i < 3; i++) {
            result += i;
        }
    } else {
        result = x * 3;
        data = 10;
    }
    
    return result;
}

/* Test with compound condition */
static int __attribute__((noinline, noclone))
test_compound_condition(int a, int b, int c) {
    volatile int local_cond = global_cond;
    volatile int result = 0;
    
    /* Compound condition */
    if (a > b && local_cond < c) {
        /* Modify one operand of the compound condition */
        local_cond = a + b;  /* MODIFIES CONDITION OPERAND */
        
        /* Notes in the header */
        asm volatile("# Compound condition block note 1");
        asm volatile("# Compound condition block note 2");
        
        /* Block body */
        result = a * b * c;
        global_array[3] = result;
        
        /* Nested control flow to create interesting block structure */
        if (result > 100) {
            result /= 2;
        }
    } else {
        result = a + b + c;
        local_cond = result;
    }
    
    /* Use the value to prevent dead code elimination */
    return use_value(result);
}

/* Main function to exercise different paths */
int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_ptr = (int*)&global_array[0];
    
    /* Test 1: Vary global_cond to take different branches */
    printf("Testing if-conversion patterns...\n");
    
    for (int i = 0; i < 20; i++) {
        global_cond = i;  /* Vary condition */
        global_threshold = 15;
        
        /* Call test functions with different parameters
           to ensure if-conversion pass is invoked */
        results[0] = test_if_conversion(i, i * 2);
        results[1] = test_pointer_condition(i);
        results[2] = test_compound_condition(i, i/2, i*3);
        
        /* Use results to prevent optimization */
        if (i % 5 == 0) {
            printf("Iteration %d: %d, %d, %d\n", 
                   i, results[0], results[1], results[2]);
        }
    }
    
    /* Additional test with edge cases */
    global_cond = 100;  /* Will trigger then block */
    results[0] = test_if_conversion(1, 2);
    
    global_cond = 5;    /* Will trigger else block */
    results[1] = test_if_conversion(3, 4);
    
    printf("Final results: %d, %d\n", results[0], results[1]);
    
    return 0;
}
