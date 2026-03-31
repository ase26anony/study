/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile ptr_cond = NULL;
static int threshold = 5;
static int side_effect = 0;

/* Function to prevent optimization */
int __attribute__((noinline, noclone)) 
use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Helper to generate notes/DEBUG_INSN in the header */
void __attribute__((noinline, noclone))
generate_notes(void) {
    /* These asm statements generate NOTE/DEBUG_INSN in RTL */
    asm volatile("# HEADER NOTE 1" : : : "memory");
    asm volatile("# HEADER NOTE 2" : : : "memory");
    /* This should appear as a NOTE before the real instruction */
    asm volatile("" : : : "memory");
}

/* The critical function containing the targeted if-then-else structure */
int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if ((global_cond < threshold) && (ptr_cond != NULL && *ptr_cond > 0)) {
        /* This is the 'then' block - starts with implicit label */
        
        /* Generate notes/DEBUG_INSN in the header portion */
        generate_notes();
        
        /* CRITICAL: Modify the condition variable BEFORE any real instruction
           This should be in the header before then_last_head */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional instructions to create a multi-instruction block */
        result = x * y;
        side_effect += result;
        
        /* More operations to ensure block isn't trivial */
        if (result > 100) {
            result /= 2;
        }
        
        /* Function call to create complex RTL */
        result = use_result(result);
        
        /* Store through pointer used in condition */
        if (ptr_cond) {
            *ptr_cond = result;
        }
    } else {
        /* 'else' block with different computation */
        result = x - y;
        global_cond = result / 2;
    }
    
    return result;
}

/* Another test case with different condition structure */
int __attribute__((noinline, noclone))
test_pointer_modification(int* arr, int size) {
    int sum = 0;
    volatile int* volatile volatile_ptr = arr;
    
    /* Condition using pointer that will be modified in header */
    if (volatile_ptr != NULL && size > 0 && arr[0] != 0) {
        /* Header notes */
        asm volatile("# Pointer test header note" : : : "memory");
        
        /* Modify the pointer dereference expression */
        arr[0] = size * 2;  /* Modifies memory accessed in condition */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Real work */
        for (int i = 0; i < size && i < 10; i++) {
            sum += arr[i];
        }
        
        /* Additional modification */
        volatile_ptr = &arr[size-1];
    } else {
        sum = -1;
    }
    
    return sum;
}

/* Test with static variable condition */
static int static_counter = 0;
int __attribute__((noinline, noclone))
test_static_condition(int val) {
    int output = 0;
    
    /* Condition using static variable */
    if (static_counter < 10 && val > 0) {
        /* Multiple notes in header */
        asm volatile("# Static test note 1" : : : "memory");
        asm volatile("# Static test note 2" : : : "memory");
        asm volatile("# Static test note 3" : : : "memory");
        
        /* Modify static variable used in condition */
        static_counter = val;  /* This modification should be detected */
        
        /* Real instruction after modification */
        output = val * 2 + static_counter;
        
        /* More complex operations */
        for (int i = 0; i < 3; i++) {
            output += i;
        }
    } else {
        output = val / 2;
        static_counter--;
    }
    
    return output;
}

int main(void) {
    int results[3] = {0};
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Initialize globals */
    global_cond = 3;
    ptr_cond = &test_array[0];
    threshold = 10;
    
    printf("Testing if-conversion patterns...\n");
    
    /* Multiple calls to ensure if-conversion pass runs */
    for (int i = 0; i < 5; i++) {
        /* Test 1: Global variable modification in header */
        results[0] += test_if_conversion(i, i+1);
        
        /* Test 2: Pointer-based condition */
        ptr_cond = (i % 2) ? &test_array[0] : NULL;
        results[1] += test_pointer_modification(test_array, 10);
        
        /* Test 3: Static variable condition */
        results[2] += test_static_condition(i);
        
        /* Vary global condition to take different paths */
        global_cond = (global_cond + 1) % 15;
    }
    
    printf("Results: %d, %d, %d\n", results[0], results[1], results[2]);
    printf("Final global_cond: %d, side_effect: %d\n", global_cond, side_effect);
    
    return 0;
}
