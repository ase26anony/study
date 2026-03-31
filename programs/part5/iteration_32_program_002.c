/* ifcvt_test.c - Target the uncovered lines in GCC's if-conversion pass */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int global_threshold = 100;
static int global_a = 0, global_b = 0, global_c = 0, global_d = 0;

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Test function with complex condition and early modification in then-block */
__attribute__((noinline, noclone))
int test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using globals - creates non-trivial test_expr */
    if ((global_cond < global_threshold) && 
        (global_ptr != NULL) && 
        (*global_ptr > 0) &&
        (global_a > global_b || global_c < global_d)) {
        
        /* This is the critical then-block header */
        /* First, generate some NOTE/DEBUG_INSN instructions */
        asm volatile("# DEBUG/NOTE: Start of then block" : : : "memory");
        asm volatile("# Another note instruction" : : : "memory");
        
        /* CRITICAL: Modify condition variable BEFORE first real instruction */
        /* This should be in the header before then_last_head */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional statements to create a multi-instruction block */
        result = x * y;
        global_a = result / 2;
        
        /* More operations to ensure block has sufficient instructions */
        if (global_ptr) {
            *global_ptr += result;
        }
        
        /* Function call to create complex RTL */
        result = use_result(result);
        
        /* Additional arithmetic */
        for (int i = 0; i < 3; i++) {
            result += i;
        }
        
    } else {
        /* Else block with different computation */
        result = x - y;
        global_b = result * 2;
        
        /* Ensure else block isn't empty */
        asm volatile("# Else block note" : : : "memory");
    }
    
    return result;
}

/* Another test case with pointer-based condition */
__attribute__((noinline, noclone))
int test_pointer_modification(int* ptr1, int* ptr2) {
    int local_result = 0;
    
    /* Condition involving pointer dereference */
    if (ptr1 && ptr2 && (*ptr1 > *ptr2)) {
        
        /* Header with notes/debug instructions */
        asm volatile("# Pointer test note 1" : : : "memory");
        asm volatile("# Pointer test note 2" : : : "memory");
        
        /* CRITICAL: Modify what the condition depends on */
        *ptr1 = 0;  /* Dereference modifies memory used in condition */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Additional code */
        local_result = *ptr2 * 2;
        *ptr2 = local_result;
        
        /* More operations */
        for (int i = 0; i < 4; i++) {
            local_result += i * i;
        }
        
    } else {
        local_result = -1;
        asm volatile("# Pointer else note" : : : "memory");
    }
    
    return local_result;
}

/* Test with static variable condition */
static int static_counter = 0;
__attribute__((noinline, noclone))
int test_static_condition(int val) {
    int ret = 0;
    
    /* Condition using static variable */
    if (static_counter < 10) {
        
        /* Notes in header */
        asm volatile("# Static condition note" : : : "memory");
        
        /* Modify the static variable used in condition */
        static_counter += val;  /* Direct modification */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Block body */
        ret = val * 3;
        for (int i = 0; i < val % 5; i++) {
            ret += static_counter;
        }
        
    } else {
        ret = val * 2;
        static_counter = 0;
    }
    
    return ret;
}

int main(void) {
    int array1[2] = {50, 30};
    int array2[2] = {20, 40};
    int total = 0;
    
    /* Initialize globals */
    global_cond = 50;
    global_ptr = &array1[0];
    global_a = 75;
    global_b = 25;
    global_c = 10;
    global_d = 90;
    
    /* Call test functions multiple times to exercise different paths */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to take both branches */
        int x = (i % 3) * 20;
        int y = (i % 4) * 15;
        
        /* Modify condition variables between calls */
        global_threshold = 80 + (i % 20);
        global_cond = 60 - (i % 15);
        
        /* Test 1: Complex global condition */
        total += test_if_conversion(x, y);
        
        /* Test 2: Pointer-based condition */
        if (i % 2 == 0) {
            total += test_pointer_modification(&array1[0], &array2[0]);
        } else {
            total += test_pointer_modification(&array2[1], &array1[1]);
        }
        
        /* Test 3: Static variable condition */
        total += test_static_condition(i);
        
        /* Flip some global conditions */
        global_a ^= global_b;
        global_b ^= global_a;
        global_a ^= global_b;
    }
    
    printf("Total result: %d\n", total);
    printf("Final global_cond: %d\n", global_cond);
    printf("Final static_counter: %d\n", static_counter);
    
    return total != 0 ? 0 : 1;
}
