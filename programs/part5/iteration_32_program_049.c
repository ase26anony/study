/* ifcvt_test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization and create notes/debug insns */
__attribute__((noinline, noclone)) 
static void create_notes(void) {
    /* These asm statements generate NOTE or DEBUG_INSN in RTL */
    asm volatile("# HEADER NOTE 1" : : : "memory");
    asm volatile("# HEADER NOTE 2" : : : "memory");
    asm volatile("# HEADER NOTE 3" : : : "memory");
}

/* Test function with the critical if-then-else structure */
__attribute__((noinline, noclone, optimize("O2")))
static int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond < threshold && *(&global_cond) != 0) {
        /* This block should have a header with:
           - Label (implicit at block start)
           - NOTE/DEBUG_INSN from create_notes()
           - Modification of condition variable BEFORE first real instruction
        */
        
        /* Generate notes in the block header */
        create_notes();
        
        /* CRITICAL: Modify condition variable in the header 
           This should be before then_last_head in RTL */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* Additional statements to create multi-instruction block */
        local_mod = global_cond * 2;
        ptr = &local_mod;
        data[0] = local_mod;
        result = data[0] + threshold;
        
        /* More operations to ensure block isn't optimized away */
        for (int i = 1; i < 5; i++) {
            data[i] = data[i-1] + i;
            result += data[i];
        }
    } else {
        /* Else block with different computation */
        result = x - y;
        global_cond = result / 2;
        ptr = &global_cond;
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (result) : : "memory");
    return result;
}

/* Second test with different condition structure */
__attribute__((noinline, noclone))
static int test_pointer_modification(int val) {
    static volatile int* cond_ptr = NULL;
    volatile int buffer[5] = {1, 2, 3, 4, 5};
    int sum = 0;
    
    /* Initialize pointer to condition data */
    if (cond_ptr == NULL) {
        cond_ptr = &buffer[2];
    }
    
    /* Condition using pointer dereference */
    if (*cond_ptr > 0 && val > 0) {
        /* Header notes */
        asm volatile("# NOTE: Pointer test header" : : : "memory");
        
        /* Modify through pointer - affects condition expression */
        *cond_ptr = val * 2;  /* Modifies memory used in condition */
        
        /* Additional code */
        for (int i = 0; i < 5; i++) {
            sum += buffer[i];
            asm volatile("# LOOP NOTE" : : : "memory");
        }
        
        /* Complex computation */
        sum = sum * val + global_cond;
    } else {
        sum = val + *cond_ptr;
        *cond_ptr = sum / 3;
    }
    
    return sum;
}

/* Third test with function call in condition */
__attribute__((noinline, noclone))
static volatile int counter = 0;

static int get_counter(void) {
    return counter++;
}

__attribute__((noinline, noclone))
static int test_function_condition(int a, int b) {
    int res = 0;
    
    /* Function call in condition */
    if (get_counter() < 10 && a > b) {
        /* Header section */
        asm volatile("# FUNCTION CONDITION HEADER" : : : "memory");
        asm volatile("# ANOTHER NOTE" : : : "memory");
        
        /* Modify global used in condition */
        counter = a * b;  /* Modifies variable used by get_counter() */
        
        /* Block body */
        res = a * b + counter;
        for (int i = 0; i < 3; i++) {
            res += test_if_conversion(i, res);
        }
    } else {
        res = a - b + counter;
    }
    
    return res;
}

int main(void) {
    int total = 0;
    
    /* Initialize globals */
    global_cond = 3;
    threshold = 7;
    
    /* Allocate and initialize pointer target */
    volatile int target = 42;
    ptr = &target;
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 10; i++) {
        total += test_if_conversion(i, i * 2);
        total += test_pointer_modification(i);
        total += test_function_condition(i, i / 2 + 1);
        
        /* Vary global condition to take different branches */
        global_cond = (i % 3) * 5;
        threshold = (i % 4) + 3;
    }
    
    printf("Result: %d\n", total);
    
    /* Additional calls with edge cases */
    global_cond = 0;
    total += test_if_conversion(0, 0);
    
    global_cond = 100;
    threshold = 50;
    total += test_if_conversion(10, 20);
    
    printf("Final result: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
