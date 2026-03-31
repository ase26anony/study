/* ifcvt_test.c - Test case for if-conversion header modification check */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 100;
static volatile int* volatile global_ptr = NULL;
static volatile int global_data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Target function with the critical if-then-else structure */
__attribute__((noinline, noclone, optimize("O2")))
int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable */
    if (global_cond > global_threshold) {
        /* This assignment modifies the condition variable IN THE HEADER */
        /* It should appear before then_last_head in RTL */
        global_cond = 50;  /* MODIFIES CONDITION VARIABLE */
        
        /* Generate NOTE/DEBUG_INSN instructions in header */
        /* These should appear as NOTE_P or DEBUG_INSN_P in RTL */
        asm volatile("# HEADER NOTE 1" : : : "memory");
        asm volatile("# HEADER NOTE 2" : : : "memory");
        
        /* Additional statements to create a proper basic block */
        local_mod = x * y;
        result = use_value(local_mod);
        
        /* More operations to ensure block isn't trivial */
        global_data[0] = result;
        result += global_ptr ? *global_ptr : 0;
        
        /* Function call to create complex RTL */
        result = abs(result);
    } else {
        /* Else branch with different operations */
        result = x - y;
        global_cond += 10;  /* Modify condition for next iteration */
    }
    
    return result;
}

/* Second test with pointer-based condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_pointer_condition(int x) {
    static volatile int* ptr = NULL;
    static volatile int target = 0;
    int result = 0;
    
    /* Initialize pointer once */
    if (ptr == NULL) {
        ptr = &target;
        target = 100;
    }
    
    /* Condition with pointer dereference */
    if (*ptr > 50) {
        /* Modify through pointer - changes condition expression */
        *ptr = 25;  /* MODIFIES DEREFERENCED CONDITION */
        
        /* Notes in header */
        asm volatile("# PTR NOTE 1" : : : "memory");
        asm volatile("# PTR NOTE 2" : : : "memory");
        
        /* Block body */
        result = x * 2;
        target = result;
        result += (int)ptr;
    } else {
        result = x / 2;
        *ptr += 20;
    }
    
    return result;
}

/* Third test with compound condition */
__attribute__((noinline, noclone, optimize("O2")))
int test_compound_condition(int a, int b) {
    static volatile int cond1 = 0;
    static volatile int cond2 = 100;
    int result = 0;
    
    /* Compound condition */
    if (cond1 < 50 && cond2 > 75) {
        /* Modify one part of compound condition */
        cond1 = 60;  /* MODIFIES CONDITION OPERAND */
        
        /* Header notes */
        asm volatile("# COMPOUND NOTE" : : : "memory");
        
        /* Block body */
        result = a + b;
        cond2 -= result % 10;
        result *= 2;
    } else {
        result = a - b;
        cond1 += 5;
        cond2 -= 3;
    }
    
    return result;
}

/* Main function to exercise all test cases */
int main(void) {
    int i, result = 0;
    
    /* Initialize globals */
    global_cond = 150;  /* Start above threshold */
    global_threshold = 100;
    global_ptr = (int*)&global_data[5];
    *global_ptr = 200;
    
    printf("Testing if-conversion header modification check...\n");
    
    /* Multiple calls to trigger if-conversion */
    for (i = 0; i < 10; i++) {
        /* Vary inputs to take different paths */
        int input1 = (i * 17) % 100;
        int input2 = (i * 23) % 100;
        
        /* Call test functions */
        result += test_if_conversion(input1, input2);
        result += test_pointer_condition(input1);
        result += test_compound_condition(input1, input2);
        
        /* Modify global condition to potentially change branch direction */
        global_cond += (i * 7) % 50;
        
        /* Ensure compiler doesn't optimize away calls */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    printf("Result: %d (should be non-zero)\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        return 1;
    }
    
    return 0;
}
