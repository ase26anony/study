/* ifcvt-test.c - Test case for GCC if-conversion pass */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -S ifcvt-test.c */
/* Or for ARM: gcc -O2 -march=armv7-a -mtune=cortex-a9 -fdump-rtl-ifcvt -S ifcvt-test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Function with the targeted if-then-else structure */
static int __attribute__((noinline, noclone))
test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial test_expr in RTL */
    if (global_cond > threshold && *(&global_cond) != 0) {
        /* 
         * This is the 'then' block (then_bb).
         * The header starts with an implicit label.
         * We need to modify the condition variable BEFORE the first
         * real instruction in the block.
         */
        
        /* Generate NOTE/DEBUG_INSN instructions first */
        /* These should appear before then_last_head */
        asm volatile("# DEBUG/NOTE: Start of then block");
        asm volatile("# Another note instruction");
        
        /* CRITICAL: Modify condition variable in the header section */
        /* This modification happens before any non-note instruction */
        global_cond = x + y;  /* Modifies test_expr component */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional statements to create a multi-instruction block */
        /* These come after the modification */
        result = x * y;
        ptr = &result;
        data[0] = result;
        
        /* More operations to ensure block isn't optimized away */
        for (int i = 1; i < 5; i++) {
            data[i] = result + i;
        }
        
        asm volatile("# End of then block operations");
    } else {
        /* Else block with different operations */
        result = x - y;
        global_cond = result;
        ptr = &global_cond;
    }
    
    return use_result(result);
}

/* Another test case with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_condition(int* arr, int idx) {
    volatile int local = 0;
    
    /* Condition involving pointer dereference */
    if (arr != NULL && arr[idx] > 0 && global_cond < 10) {
        /* Header with notes/debug */
        asm volatile("# Pointer condition then block");
        
        /* Modify condition component - arr[idx] affects the condition */
        arr[idx] = -1;  /* Modifies memory referenced in condition */
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Additional operations */
        for (int i = 0; i < idx; i++) {
            local += arr[i];
        }
        
        global_cond += local;
    } else {
        local = idx * 2;
        if (arr) arr[idx] = local;
    }
    
    return use_result(local);
}

/* Test with static variable condition */
static int __attribute__((noinline, noclone))
test_static_condition(int val) {
    static int counter = 0;
    int ret = 0;
    
    /* Condition using static variable */
    if (counter++ < 3 && global_cond > 0) {
        /* Notes in header */
        asm volatile("# Static condition block");
        asm volatile("# Additional note");
        
        /* Modify global_cond which is part of the condition */
        global_cond = val;  /* Modification in header */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Block body */
        ret = val * 2;
        for (int i = 0; i < val && i < 10; i++) {
            data[i] = ret + i;
        }
    } else {
        ret = val / 2;
        global_cond = ret;
    }
    
    counter = counter % 5;
    return use_result(ret);
}

int main(void) {
    int results[3] = {0};
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Initialize globals */
    global_cond = 1;
    threshold = 3;
    ptr = &global_cond;
    
    /* Call test functions multiple times to exercise different paths */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to take different branches */
        global_cond = i % 7;
        
        /* Test 1: Should trigger then block for some iterations */
        results[0] += test_if_conversion(i, i + 1);
        
        /* Test 2: Pointer-based condition */
        results[1] += test_pointer_condition(test_array, i % 10);
        
        /* Test 3: Static variable condition */
        results[2] += test_static_condition(i);
        
        /* Modify array to affect future conditions */
        test_array[i % 10] = i;
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 3; i++) {
        final_result += results[i];
    }
    
    printf("Final result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
