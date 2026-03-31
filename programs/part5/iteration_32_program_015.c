/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone, noipa))
static int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *(&global_cond) != 0) {
        /* 
         * This is the 'then' block (then_bb).
         * The header starts here with the block label.
         * We need instructions in the header before then_last_head
         * that modify test_expr (which involves global_cond).
         */
        
        /* First, generate some NOTE/DEBUG_INSN instructions */
        /* These will be in the header but before the modifying instruction */
        asm volatile("# DEBUG/NOTE: Entering then block" : : : "memory");
        asm volatile("# Another note instruction" : : : "memory");
        
        /* 
         * CRITICAL: Modify the condition variable BEFORE any other
         * non-label, non-note, non-debug instruction.
         * This should make modified_in_p(test_expr, insn) return true
         * when scanning the header.
         */
        global_cond = x + y;  /* This modifies test_expr! */
        
        /* Additional asm notes after modification */
        asm volatile("# Note after modification" : : : "memory");
        
        /* 
         * Now add other instructions to ensure we have a proper block
         * with then_last_head pointing somewhere after the header.
         */
        local_mod = x * y;
        result = local_mod + global_cond;
        
        /* More operations to create sufficient instructions */
        for (int i = 0; i < 3; i++) {
            data[i] = result + i;
        }
        
        /* Function call to prevent tail merging */
        result += abs(result);
    } else {
        /* else block */
        result = x - y;
        global_cond = threshold - 1;
    }
    
    return result;
}

/* Another test case with pointer-based condition */
__attribute__((noinline, noclone, noipa))
static int test_pointer_condition(int val) {
    static volatile int target = 0;
    volatile int* volatile local_ptr = &target;
    int result = 0;
    
    /* Condition involving pointer dereference */
    if (*local_ptr < val && target != 0) {
        /* Header with notes/debug */
        asm volatile("# Pointer test header note 1" : : : "memory");
        asm volatile("# Pointer test header note 2" : : : "memory");
        
        /* Modify through pointer - affects test_expr */
        *local_ptr = val * 2;
        
        /* More notes */
        asm volatile("# After pointer modification" : : : "memory");
        
        /* Real work */
        result = *local_ptr + val;
        for (int i = 0; i < 4; i++) {
            data[i + 5] = result + i;
        }
    } else {
        result = val * 3;
        target = val;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone, noipa))
static int test_compound_condition(int a, int b, int c) {
    volatile static int cond1 = 0, cond2 = 0;
    int result = 0;
    
    /* Compound condition */
    if ((cond1 > a) && (cond2 < b) && (global_cond != c)) {
        /* Multiple notes in header */
        asm volatile("# Compound condition note A" : : : "memory");
        asm volatile("# Compound condition note B" : : : "memory");
        asm volatile("# Compound condition note C" : : : "memory");
        
        /* Modify one part of the compound condition */
        cond1 = b - a;  /* Affects test_expr */
        
        /* Another note */
        asm volatile("# After cond1 modification" : : : "memory");
        
        /* Block body */
        result = cond1 + cond2 + global_cond;
        cond2 = result / 2;
        
        /* Ensure multiple instructions */
        for (volatile int i = 0; i < 2; i++) {
            result += data[i];
        }
    } else {
        result = a + b + c;
        cond1 = a;
        cond2 = b;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Initialize */
    ptr = (int*)&global_cond;
    global_cond = 3;
    threshold = 5;
    
    printf("Testing if-conversion coverage...\n");
    
    /* Call test functions multiple times with different inputs
     * to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 10; i++) {
        total += test_if_conversion(i, i * 2);
        total += test_pointer_condition(i);
        total += test_compound_condition(i, i + 1, i + 2);
        
        /* Vary global condition to take different branches */
        global_cond = (i % 3) * 2;
        threshold = (i % 2) ? 3 : 7;
    }
    
    printf("Total result: %d\n", total);
    printf("Final global_cond: %d\n", global_cond);
    
    return total != 0 ? 0 : 1;
}
