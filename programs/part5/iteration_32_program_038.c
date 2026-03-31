/* ifcvt_coverage.c - Target specific if-conversion coverage test */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 100;
static volatile int* volatile global_ptr = NULL;
static int static_counter = 0;

/* Function to prevent optimization */
__attribute__((noinline, noclone)) 
int target_function(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable - creates non-trivial test_expr */
    if (global_cond > global_threshold && x < y) {
        /* 
         * This is the 'then' block (then_bb).
         * The header starts here with the block label.
         * 
         * We need to modify the condition expression (global_cond) 
         * BEFORE the first real instruction in the block.
         * 
         * GCC will generate NOTE/DEBUG_INSN instructions here for:
         * - Block labels
         * - Line number notes
         * - Debug information
         * 
         * We'll insert our modification right after those notes
         * but before any other "real" computation.
         */
        
        /* MODIFICATION OF CONDITION EXPRESSION IN HEADER */
        /* This modifies global_cond which is part of the test_expr */
        global_cond = 50;  /* This should trigger modified_in_p() */
        
        /* Additional statements to ensure block has multiple instructions */
        /* and isn't optimized away */
        asm volatile("# Begin real computation" : : : "memory");
        
        local_mod = x * y;
        result = local_mod + static_counter;
        
        /* More computation to ensure block is substantial */
        for (int i = 0; i < 3; i++) {
            result += i;
        }
        
        /* Store to volatile to prevent dead code elimination */
        global_ptr = &result;
        
        asm volatile("# End of then block" : : : "memory");
    } else {
        /* else block */
        result = x - y;
        static_counter++;
    }
    
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone))
int pointer_condition_test(int* ptr1, int* ptr2) {
    int temp = 0;
    
    /* Condition involving pointer dereference */
    if (ptr1 && *ptr1 > 0 && ptr2 && *ptr2 < 100) {
        /*
         * Modify the dereferenced value that was used in condition
         * This should be detected by modified_in_p()
         */
        *ptr1 = -1;  /* Modifies condition expression */
        
        /* Generate some notes/debug insns via inline asm */
        asm volatile("# Note: Pointer modification block" : : : "memory");
        
        temp = *ptr1 + *ptr2;
        
        /* Additional operations */
        for (int i = 0; i < 4; i++) {
            temp += i * 2;
        }
    } else {
        temp = 1000;
    }
    
    return temp;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone))
int compound_condition_test(int a, int b, int c) {
    volatile int cond_a = a;
    volatile int cond_b = b;
    int result = 0;
    
    /* Compound condition */
    if (cond_a > 10 && cond_b < 20 && c != 0) {
        /* Modify cond_a which was used in the condition */
        cond_a = 5;  /* This modification should be in the header */
        
        /* Debug note */
        asm volatile("# Debug: compound condition block" : : : "memory");
        
        result = (cond_a * cond_b) / c;
        
        /* Loop to add more instructions */
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += i * i;
        }
        result += sum;
    } else {
        result = a + b + c;
    }
    
    return result;
}

int main(void) {
    int results[10];
    int test_count = 0;
    
    /* Initialize global pointer */
    int heap_var = 42;
    global_ptr = &heap_var;
    
    /* Test 1: Vary global_cond to take different paths */
    printf("Running if-conversion coverage tests...\n");
    
    for (int i = 0; i < 5; i++) {
        global_cond = 150;  /* Will enter then block */
        results[test_count++] = target_function(i * 20, i * 10);
        
        global_cond = 50;   /* Will skip then block */
        results[test_count++] = target_function(i * 30, i * 15);
    }
    
    /* Test 2: Pointer-based condition */
    int ptr1_val = 50;
    int ptr2_val = 30;
    int* ptr1 = &ptr1_val;
    int* ptr2 = &ptr2_val;
    
    results[test_count++] = pointer_condition_test(ptr1, ptr2);
    
    /* Test with NULL pointer to take else path */
    results[test_count++] = pointer_condition_test(NULL, ptr2);
    
    /* Test 3: Compound condition */
    for (int i = 0; i < 3; i++) {
        results[test_count++] = compound_condition_test(15, 10, 2);  /* Takes then */
        results[test_count++] = compound_condition_test(5, 25, 1);   /* Takes else */
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < test_count; i++) {
        final_result += results[i];
    }
    
    printf("Final result: %d (used to prevent optimization)\n", final_result);
    printf("Test completed. Check RTL dumps for if-conversion behavior.\n");
    
    return final_result != 0 ? 0 : 1;
}
