/* ifcvt-test.c - Test case for if-conversion header modification check */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Target function with the specific if-then-else structure */
__attribute__((noinline, noclone, optimize("O2")))
int test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Force pointer setup to avoid null dereference */
    ptr = &data[0];
    *ptr = x;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *ptr != 0) {
        /* This is the 'then' block - header starts here */
        
        /* DEBUG_INSN/NOTE instructions via asm comments */
        asm volatile("# DEBUG/NOTE: Start of then block header");
        asm volatile("# Another note in header");
        
        /* CRITICAL: Modify condition variable BEFORE first real instruction
           This should be in the header before then_last_head */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional statements to create multi-instruction block */
        result = x * y;
        data[1] = result;
        
        /* More operations to ensure block isn't trivial */
        if (result > 100) {
            data[2] = result / 2;
        }
        
        /* Function call to create complex RTL */
        result = use_result(result);
        
        /* Final store */
        data[3] = result;
        
        asm volatile("# DEBUG/NOTE: End of then block");
    } else {
        /* Else block with different computation */
        result = x - y;
        data[4] = result;
        global_cond = result / 2;
    }
    
    /* Use result to prevent dead code elimination */
    return use_result(result);
}

/* Alternative test with different condition structure */
__attribute__((noinline, noclone, optimize("O2")))
int test_if_conversion2(int a, int b) {
    static volatile int static_var = 0;
    volatile int local_result = 0;
    
    /* Condition with static variable */
    if (static_var < a && b > 0) {
        /* Header with notes */
        asm volatile("# Header note 1");
        asm volatile("# Header note 2");
        
        /* Modify condition variable in header */
        static_var = a * 2;  /* This modifies test_expr */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Body of then block */
        local_result = a + b + static_var;
        data[5] = local_result;
        
        /* Nested condition to create more complex CFG */
        if (local_result > 50) {
            data[6] = local_result - 50;
        }
        
        asm volatile("# End note");
    } else {
        local_result = a - b;
        static_var = local_result;
    }
    
    return use_result(local_result);
}

/* Test with pointer-based condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_pointer_condition(int val) {
    volatile int* local_ptr = &data[7];
    int ret = 0;
    
    /* Condition depends on pointer dereference */
    if (*local_ptr < val && global_cond > 0) {
        /* Notes in header */
        asm volatile("# Pointer test header");
        
        /* Modify through pointer - affects condition */
        *local_ptr = val + 10;  /* Modifies memory used in condition */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Block body */
        ret = val * 2;
        data[8] = ret;
        
        /* Loop to create more instructions */
        for (int i = 0; i < 3; i++) {
            data[8 + i] += i;
        }
        
        asm volatile("# End pointer test");
    } else {
        *local_ptr = val - 5;
        ret = val / 2;
    }
    
    return use_result(ret);
}

int main(void) {
    int total = 0;
    
    /* Initialize globals */
    global_cond = 3;
    threshold = 5;
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 10; i++) {
        total += test_if_conversion(i, i + 1);
        total += test_if_conversion2(i * 2, i + 2);
        total += test_pointer_condition(i * 3);
        
        /* Vary global condition to take different branches */
        global_cond = (i % 3) * 4;
        threshold = (i % 2) * 10;
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", use_result(total));
    
    return 0;
}
