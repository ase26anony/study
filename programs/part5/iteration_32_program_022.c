/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int data_array[4] = {0, 0, 0, 0};

/* Function to prevent optimizations */
static int __attribute__((noinline, noclone)) 
side_effect(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

/* Target function with the specific if-then-else structure */
static int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int threshold) {
    int result = 0;
    static int static_counter = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *global_ptr != 0) {
        /* 
         * HEADER SECTION START (target for uncovered lines)
         * This should generate: label, possibly debug/note insns,
         * then modification of condition variable before then_last_head
         */
        
        /* Generate NOTE/DEBUG_INSN via asm comments */
        asm volatile("# DEBUG/NOTE: Entering then block");
        asm volatile("# Another note instruction");
        
        /* CRITICAL: Modify condition variable EARLY in header */
        /* This should be before the first "real" instruction in RTL */
        global_cond = threshold - 1;  /* Modifies test_expr component */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* 
         * Additional instructions to create multi-instruction block
         * Ensure then_last_head points somewhere after the modification
         */
        static_counter++;
        result = side_effect(global_cond);
        data_array[result % 4] = static_counter;
        
        /* More operations to ensure block isn't trivial */
        if (static_counter % 2) {
            result *= 2;
        }
        
        asm volatile("# DEBUG/NOTE: Exiting then block header");
    } else {
        /* Else block with different behavior */
        result = side_effect(threshold);
        global_cond += 2;
    }
    
    return result;
}

/* Second test case with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int val) {
    int local = 0;
    int* ptr = &local;
    
    /* Condition depends on pointer dereference */
    if (*ptr == 0 && global_ptr != NULL) {
        /* Header with notes and modification */
        asm volatile("# Note: pointer test block");
        
        /* Modify what the pointer points to - affects condition */
        *ptr = val;  /* Modifies memory referenced in condition */
        
        asm volatile("" : : : "memory");
        
        /* Additional instructions */
        local = side_effect(local);
        global_ptr = &local;
        
        asm volatile("# End of header notes");
    } else {
        local = val * 2;
    }
    
    return local;
}

/* Third test with compound condition */
static int __attribute__((noinline, noclone))
test_compound_condition(int a, int b) {
    static int mod_target = 10;
    int result = 0;
    
    /* Compound condition where one part gets modified */
    if (a > b && mod_target < 20) {
        asm volatile("# Compound condition block");
        asm volatile("# Another note");
        
        /* Modify part of the compound condition */
        mod_target = 25;  /* Makes mod_target < 20 false if re-evaluated */
        
        asm volatile("" : : : "memory");
        
        /* More code */
        result = a - b + mod_target;
        result = side_effect(result);
        
        asm volatile("# End compound block");
    } else {
        result = b - a;
        mod_target -= 2;
    }
    
    return result;
}

int main(void) {
    int i, total = 0;
    
    /* Initialize globals */
    global_cond = 5;
    data_array[0] = 1;
    data_array[1] = 2;
    global_ptr = &data_array[0];
    
    printf("Testing if-conversion patterns...\n");
    
    /* Call test functions multiple times to exercise different paths */
    for (i = 0; i < 10; i++) {
        /* Vary inputs to take both branches */
        int threshold = (i % 3) * 5;
        
        /* First test - should sometimes take then branch */
        total += test_if_conversion(threshold);
        
        /* Modify global to affect next iteration's condition */
        global_cond += i;
        *global_ptr = i % 2;
        
        /* Second test */
        total += test_pointer_modification(i);
        
        /* Third test */
        total += test_compound_condition(i, i / 2 + 1);
        
        /* Change pointer occasionally */
        if (i % 4 == 0) {
            global_ptr = &data_array[i % 4];
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Final global_cond: %d\n", global_cond);
    printf("Final data_array[0]: %d\n", data_array[0]);
    
    return total != 0 ? 0 : 1;
}
