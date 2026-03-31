/* ifcvt_coverage.c
 * Target: Trigger uncovered lines 577-583 in ifcvt.cc
 * These lines check if the condition expression is modified in the
 * header portion of the then block before the first real instruction.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
int test_function(int x, int y) {
    volatile int local_modifier = 0;
    int result = 0;
    
    /* Complex condition using global variable - will generate non-trivial RTL */
    if (global_cond < threshold && x > y) {
        /* This is the critical section: we're in the then block header */
        
        /* First, generate some NOTE/DEBUG_INSN instructions */
        /* These will appear in RTL but won't count as "real" instructions */
        asm volatile("# DEBUG/NOTE: Entering then block");
        asm volatile("# Another note instruction");
        
        /* CRITICAL: Modify the condition variable BEFORE any real instruction */
        /* This should trigger modified_in_p() to return true */
        global_cond = 10;  /* Direct modification of condition variable */
        
        /* More notes/debug insns after modification */
        asm volatile("# DEBUG: Condition modified");
        
        /* Now some real instructions to form the block body */
        local_modifier = x * y;
        result = local_modifier + global_cond;
        
        /* Additional operations to ensure block has multiple instructions */
        data[0] = result;
        if (ptr != NULL) {
            *((int*)ptr) = result;  /* Force potential pointer dereference */
        }
        
        /* Function call to create more complex control flow */
        result = abs(result);
    } else {
        /* Else block with different computation */
        result = y - x;
        global_cond--;  /* Modify global in else path too */
    }
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone))
int test_pointer_condition(int val) {
    static volatile int target = 0;
    volatile int* cond_ptr = &target;
    int result = 0;
    
    /* Condition based on pointer dereference */
    if (*cond_ptr < val && global_cond > 0) {
        /* Header with notes first */
        asm volatile("# Note: Pointer condition block");
        
        /* Modify through pointer - affects condition expression */
        *cond_ptr = val * 2;  /* This modifies the dereferenced value */
        
        asm volatile("# Debug: Pointer modified");
        
        /* Real instructions */
        result = val + *cond_ptr;
        global_cond = result % 100;
        
        /* More operations */
        for (int i = 0; i < 3; i++) {
            data[i] = result + i;
        }
    } else {
        result = val * 3;
        target = result;
    }
    
    return result;
}

/* Test with compound condition where only part is modified */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b, int c) {
    volatile int cond1 = a;
    volatile int cond2 = b;
    int result = 0;
    
    /* Compound condition */
    if (cond1 > c && cond2 < (c * 2) && global_cond != 0) {
        /* Notes/debug instructions first */
        asm volatile("# Compound condition block");
        asm volatile("# Multiple notes");
        asm volatile("# Another note");
        
        /* Modify one part of the compound condition */
        cond1 = 0;  /* This affects the condition */
        
        asm volatile("# First condition modified");
        
        /* Block body */
        result = (b - a) * c;
        cond2 = result;
        
        /* Loop to create more instructions */
        for (int i = 0; i < 5; i++) {
            result += data[i];
        }
    } else {
        result = a + b + c;
        cond1 = result;
    }
    
    return result;
}

/* Main function to exercise all test cases */
int main() {
    int results[3] = {0};
    
    /* Initialize global data */
    global_cond = 3;
    threshold = 7;
    ptr = (int*)&data[0];
    
    /* Fill data array */
    for (int i = 0; i < 10; i++) {
        data[i] = i * 2;
    }
    
    printf("Testing if-conversion coverage trigger...\n");
    
    /* Call test functions multiple times with different inputs
     * to ensure if-conversion pass is invoked */
    for (int i = 0; i < 5; i++) {
        results[0] += test_function(i, i * 2);
        results[1] += test_pointer_condition(i + 1);
        results[2] += test_compound_condition(i, i + 1, i + 2);
        
        /* Vary global condition to take different paths */
        global_cond = (global_cond + 1) % 10;
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = results[0] + results[1] + results[2];
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
