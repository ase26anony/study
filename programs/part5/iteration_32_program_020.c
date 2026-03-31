/* ifcvt_test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
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
    volatile int local_mod = x;
    static int static_counter = 0;
    
    /* Complex condition using global variable */
    if (global_cond < threshold) {
        /* This is the 'then' block - header starts here */
        
        /* DEBUG_INSN/NOTE instructions via asm comments */
        asm volatile("# DEBUG/NOTE: Entering then block");
        asm volatile("# Another note instruction");
        
        /* CRITICAL: Modify condition variable BEFORE first real instruction
           This should be in the block header before then_last_head */
        global_cond = y + 2;  /* Modifies the variable used in condition */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional instructions to form a proper block */
        local_mod += global_cond;
        static_counter++;
        data[static_counter % 10] = local_mod;
        
        /* More asm notes to ensure header has multiple NOTE/DEBUG_INSN */
        asm volatile("# Processing data");
        
        return use_result(local_mod + static_counter);
    } else {
        /* 'else' block */
        ptr = &global_cond;
        *ptr = x - y;
        return use_result(*ptr);
    }
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_pointer_condition(int x) {
    static int buffer[5] = {1, 2, 3, 4, 5};
    volatile int* volatile cond_ptr = buffer;
    
    /* Condition with pointer dereference */
    if (*cond_ptr > 2 && global_cond < 10) {
        /* Header with notes */
        asm volatile("# Note: Pointer condition block");
        asm volatile("# DEBUG comment");
        
        /* Modify through pointer - affects condition */
        *cond_ptr = x;  /* Modifies memory used in condition */
        
        asm volatile("" : : : "memory");
        
        /* Additional code */
        buffer[1] = *cond_ptr + 1;
        buffer[2] = buffer[1] * 2;
        
        asm volatile("# End of pointer block header");
        
        return use_result(buffer[0] + buffer[1]);
    }
    return 0;
}

/* Test with compound condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_compound_condition(int a, int b, int c) {
    volatile int cond1 = a;
    volatile int cond2 = b;
    
    /* Compound condition */
    if (cond1 > cond2 && global_cond < threshold) {
        /* Notes in header */
        asm volatile("# Compound condition block");
        asm volatile("# Header note 1");
        asm volatile("# Header note 2");
        
        /* Modify one operand of the compound condition */
        cond1 = c;  /* Modifies variable used in condition */
        
        asm volatile("" : : : "memory");
        
        /* More instructions */
        int result = cond1 * 2 + cond2;
        data[result % 10] = result;
        
        return use_result(result);
    }
    return -1;
}

int main(void) {
    int results[20];
    int result_idx = 0;
    
    /* Initialize */
    global_cond = 0;
    threshold = 5;
    
    /* Call test function multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to take both branches */
        int x = i * 2;
        int y = i + 1;
        
        /* First call - likely takes 'then' branch */
        results[result_idx++] = test_if_conversion(x, y);
        
        /* Modify global to potentially take 'else' branch */
        global_cond = threshold + i;
        results[result_idx++] = test_if_conversion(x, y);
        
        /* Reset for next iteration */
        global_cond = i % 3;
    }
    
    /* Test pointer condition */
    for (int i = 0; i < 5; i++) {
        results[result_idx++] = test_pointer_condition(i);
    }
    
    /* Test compound condition */
    for (int i = 0; i < 5; i++) {
        results[result_idx++] = test_compound_condition(i, i-1, i+2);
    }
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < result_idx; i++) {
        sum += results[i];
    }
    
    printf("Result checksum: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
