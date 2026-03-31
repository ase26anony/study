/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;
static int data_array[4] = {0, 0, 0, 0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_value(int x) {
    return x * 2;
}

/* Target function with specific if-then-else structure */
__attribute__((noinline, noclone, optimize("O2")))
int test_if_conversion(int input) {
    int result = 0;
    int local_var = input;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond < threshold && *(&global_cond) != 0) {
        /* This is the 'then' block (then_bb) */
        
        /* Generate NOTE/DEBUG_INSN instructions in header */
        /* These should appear before the first real instruction */
        asm volatile("# DEBUG/NOTE: Start of then block" : : : "memory");
        asm volatile("# Another note instruction" : : : "memory");
        
        /* CRITICAL: Modify condition variable BEFORE first non-note instruction
           This should make modified_in_p return true in the uncovered check */
        global_cond = 50;  /* Direct modification of condition variable */
        
        /* Additional asm to ensure we have more notes/debug */
        asm volatile("# Note after modification" : : : "memory");
        
        /* Now the "real" instructions begin - this is where then_last_head points */
        /* These come after the header section being checked */
        result = use_value(local_var);
        data_array[0] = result;
        data_array[1] = result + 1;
        data_array[2] = result + 2;
        
        /* More operations to ensure block has multiple instructions */
        for (int i = 0; i < 2; i++) {
            data_array[3] += i;
        }
        
        /* Function call to create complex RTL */
        result += abs(local_var);
        
    } else {
        /* Else block */
        result = -input;
        global_cond += 10;
    }
    
    /* Additional computation to prevent tail merging */
    result ^= (result >> 3);
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_pointer_condition(int seed) {
    static int buffer[10] = {0};
    int* ptr = buffer;
    int sum = 0;
    
    /* Initialize pointer */
    global_ptr = &buffer[0];
    buffer[0] = seed;
    
    /* Condition with pointer dereference */
    if (*global_ptr > 0 && global_ptr != NULL) {
        /* Header with notes/debug */
        asm volatile("# Pointer test header note 1" : : : "memory");
        asm volatile("# Pointer test header note 2" : : : "memory");
        
        /* Modify through pointer - affects condition expression */
        *global_ptr = -5;  /* This modifies the dereferenced value */
        
        /* More notes */
        asm volatile("# After pointer modification" : : : "memory");
        
        /* Real instructions start here */
        for (int i = 0; i < 5; i++) {
            buffer[i] = seed + i;
            sum += buffer[i];
        }
        
        /* Complex operation */
        sum = (sum << 2) | (sum >> 30);
        
    } else {
        sum = seed * 3;
        global_ptr = &buffer[5];
    }
    
    return sum;
}

/* Test with compound condition */
__attribute__((noinline, noclone, optimize("O3")))
int test_compound_condition(int a, int b) {
    static int counter = 0;
    int x = a;
    int y = b;
    int ret = 0;
    
    /* Compound condition */
    if ((x > y) && (global_cond < 100) && (counter++ < 1000)) {
        /* Header section */
        asm volatile("# Compound condition header" : : : "memory");
        asm volatile("# Additional debug note" : : : "memory");
        
        /* Modify one of the condition variables */
        x = 0;  /* x was used in (x > y) condition */
        
        /* More notes */
        asm volatile("# Note after x modification" : : : "memory");
        
        /* Real work */
        ret = x - y;
        for (int i = 0; i < 4; i++) {
            ret += i * i;
        }
        
        /* Store to global */
        data_array[ret % 4] = ret;
        
    } else {
        ret = y - x;
        global_cond++;
    }
    
    return ret;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 1;  /* Non-zero to take then branch first */
    global_ptr = &global_cond;
    
    printf("Testing if-conversion patterns...\n");
    
    /* Test 1: Global variable condition */
    printf("Test 1: global_cond = %d\n", global_cond);
    results[0] = test_if_conversion(42);
    printf("  Result: %d, global_cond now: %d\n", results[0], global_cond);
    
    /* Test 2: Pointer condition */
    printf("\nTest 2: Pointer condition\n");
    results[1] = test_pointer_condition(7);
    printf("  Result: %d\n", results[1]);
    
    /* Test 3: Compound condition */
    printf("\nTest 3: Compound condition\n");
    results[2] = test_compound_condition(10, 5);
    printf("  Result: %d\n", results[2]);
    
    /* Run again with different values to exercise both paths */
    global_cond = 200;  /* Above threshold to take else branch */
    printf("\nRe-running with global_cond = %d (should take else branch)\n", global_cond);
    test_if_conversion(99);
    printf("  global_cond after: %d\n", global_cond);
    
    /* Final check */
    int final_result = results[0] + results[1] + results[2];
    printf("\nFinal combined result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
