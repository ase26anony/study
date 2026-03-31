/* ifcvt_test.c - Test case for GCC if-conversion pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fdump-rtl-all ifcvt_test.c -o ifcvt_test */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 5;
static volatile int* volatile global_ptr = NULL;
static volatile int global_data[10] = {0};

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#elif defined(__arm__)
__attribute__((target("arch=armv7-a")))
#endif
int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial test_expr in RTL */
    if ((global_cond < global_threshold) && (global_ptr != NULL) && (*global_ptr > 0)) {
        /* CRITICAL: Modify condition variable BEFORE first real instruction in then block */
        /* The block starts with a label, then may have debug/note insns */
        
        /* Generate debug/note instructions first */
        asm volatile("# DEBUG/NOTE: Entering then block" : : : "memory");
        asm volatile("# DEBUG/NOTE: Before modification" : : : "memory");
        
        /* This modifies global_cond which is part of the condition */
        /* It should appear in the header before then_last_head */
        global_cond = x + y;  /* MODIFICATION OF CONDITION VARIABLE */
        
        /* More debug/note instructions after modification */
        asm volatile("# DEBUG/NOTE: After modification" : : : "memory");
        
        /* Additional statements to create a multi-instruction block */
        /* This ensures then_last_head is not the first non-debug instruction */
        local_mod = global_cond * 2;
        result = local_mod + *global_ptr;
        
        /* More operations to prevent block simplification */
        global_data[0] = result;
        global_data[1] = local_mod;
        
        asm volatile("# DEBUG/NOTE: End of then block" : : : "memory");
    } else {
        /* Else block with different computation */
        result = x - y;
        global_cond = result / 2;
        asm volatile("# DEBUG/NOTE: Else block executed" : : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    return result + global_data[0];
}

/* Another test case with different condition structure */
__attribute__((noinline, noclone))
int test_pointer_condition(int* ptr1, int* ptr2) {
    volatile int temp = 0;
    int result = 0;
    
    /* Condition with pointer comparison and dereference */
    if (ptr1 != ptr2 && *ptr1 > *ptr2) {
        /* Debug/note instructions in header */
        asm volatile("# Note: Pointer comparison true" : : : "memory");
        
        /* Modify dereferenced value - affects condition expression */
        *ptr1 = *ptr1 / 2;  /* Modification of condition expression */
        
        asm volatile("# Debug: After pointer mod" : : : "memory");
        
        /* Additional instructions */
        temp = *ptr1 + *ptr2;
        result = temp * 3;
        
        /* More operations */
        for (int i = 0; i < 3; i++) {
            global_data[i] += result;
        }
    } else {
        result = *ptr2 - *ptr1;
        asm volatile("# Note: Else path taken" : : : "memory");
    }
    
    return result;
}

/* Test with static variable condition */
static volatile int static_counter = 0;

__attribute__((noinline, noclone))
int test_static_condition(int a, int b) {
    int res = 0;
    
    /* Condition using static variable */
    if (static_counter < 10 && a > b) {
        /* Multiple asm statements to generate notes */
        asm volatile("# Header note 1" : : : "memory");
        asm volatile("# Header note 2" : : : "memory");
        
        /* Modify static_counter which is in the condition */
        static_counter += a;  /* CRITICAL MODIFICATION */
        
        asm volatile("# After static mod" : : : "memory");
        
        /* Body of then block */
        res = a * b + static_counter;
        
        /* Complex enough to avoid simplification */
        for (int i = 0; i < 5; i++) {
            res += global_data[i % 10];
        }
        
        asm volatile("# End of then" : : : "memory");
    } else {
        static_counter -= b;
        res = b - a;
    }
    
    return res;
}

int main() {
    int result = 0;
    
    /* Initialize global pointer with valid data */
    int heap_data = 20;
    global_ptr = (int*)&heap_data;
    
    printf("Testing if-conversion scenarios...\n");
    
    /* Test 1: Global variable condition */
    global_cond = 3;  /* Less than threshold (5) */
    for (int i = 0; i < 10; i++) {
        result += test_if_conversion(i, i * 2);
    }
    printf("Test 1 result: %d\n", result);
    
    /* Test 2: Pointer condition */
    int val1 = 100, val2 = 50;
    int* p1 = &val1;
    int* p2 = &val2;
    
    for (int i = 0; i < 5; i++) {
        result += test_pointer_condition(p1, p2);
        val1 += 10;  /* Change values to exercise different paths */
        val2 += 5;
    }
    printf("Test 2 result: %d\n", result);
    
    /* Test 3: Static variable condition */
    static_counter = 5;
    for (int i = 0; i < 8; i++) {
        result += test_static_condition(i * 10, i * 5);
    }
    printf("Test 3 result: %d\n", result);
    
    /* Additional test with NULL pointer to take else path */
    global_ptr = NULL;
    result += test_if_conversion(1, 2);
    
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
