/* ifcvt_test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int global_modifier = 1;
static volatile int* volatile global_ptr = NULL;
static volatile int threshold = 5;

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
int test_function(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial test_expr in RTL */
    if (global_cond > threshold && *(&global_cond) != 0) {
        /* 
         * CRITICAL: Modify the condition variable immediately in the block
         * This instruction should appear in the block's header section
         * before then_last_head
         */
        global_cond = x + y;  /* This modifies test_expr */
        
        /* Generate some notes/debug instructions before real code */
        /* These will be in the header but won't count as "real" instructions */
        asm volatile("# NOTE: Starting then block" : : : "memory");
        asm volatile("# DEBUG: global_cond modified" : : : "memory");
        
        /* Now some real instructions after the modification */
        result = global_cond * 2;
        
        /* More operations to ensure block has multiple instructions */
        global_modifier = result / 3;
        if (global_ptr) {
            *global_ptr = result;
        }
        
        /* Additional asm to prevent optimization */
        asm volatile("" : : : "memory");
    } else {
        /* Else block with different computation */
        result = x - y;
        global_cond = result;
    }
    
    /* Use result to prevent dead code elimination */
    return result + global_modifier;
}

/* Another test with different condition pattern */
__attribute__((noinline, noclone))
int test_function2(int a, int b) {
    volatile static int local_static = 10;
    int* ptr = (int*)&local_static;
    
    /* Condition with pointer dereference */
    if (*ptr > 0 && a < b) {
        /* Modify through pointer - affects condition expression */
        *ptr = a * b;  /* This modifies what was tested in condition */
        
        /* Notes/debug insns */
        asm volatile("# NOTE: Pointer modification" : : : "memory");
        
        /* Real work */
        int temp = *ptr + 100;
        
        /* More operations */
        for (int i = 0; i < 3; i++) {
            temp += i;
        }
        
        return temp;
    }
    
    return a + b;
}

/* Test with compound condition where only part is modified */
__attribute__((noinline, noclone))
int test_function3(void) {
    static volatile int cond1 = 0;
    static volatile int cond2 = 10;
    
    /* Compound condition */
    if (cond1 < 20 && cond2 > 5) {
        /* Modify one part of the compound condition */
        cond1 = 50;  /* This modifies test_expr (part of it) */
        
        /* Debug/note instructions */
        asm volatile("# DEBUG: cond1 modified" : : : "memory");
        
        /* Real code */
        int val = cond1 + cond2;
        
        /* Ensure block has multiple instructions */
        val *= 2;
        val -= 5;
        
        return val;
    }
    
    return 0;
}

int main(void) {
    int results = 0;
    
    /* Initialize global pointer */
    int local_var = 100;
    global_ptr = (int*)&local_var;
    
    /* Test different paths to exercise if-conversion */
    for (int i = 0; i < 10; i++) {
        global_cond = i;
        threshold = i / 2;
        
        /* Call test function with varying inputs */
        results += test_function(i, i * 2);
        results += test_function2(i, i + 1);
        results += test_function3();
        
        /* Vary the condition */
        if (i % 3 == 0) {
            global_cond = 0;
        }
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", results);
    
    /* Additional test with explicit branch probability */
    for (int j = 0; j < 100; j++) {
        if (j % 7 == 0) {
            global_cond = j;
            results += test_function(j, j + 3);
        }
    }
    
    return results != 0 ? 0 : 1;
}
