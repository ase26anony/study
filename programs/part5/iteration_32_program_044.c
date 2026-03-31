/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone))
static int use_value(int val) {
    asm volatile("" : "+r" (val) : : "memory");
    return val;
}

/* Function with the targeted if-then-else structure */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#endif
int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Force pointer setup */
    ptr = &local_mod;
    
    /* Complex condition expression using global variable */
    /* This should generate non-trivial RTL test_expr */
    if (global_cond > threshold && *ptr == 0) {
        /* BEGIN: Header portion of then_bb */
        /* Label will be generated here by compiler */
        
        /* Debug/Note instructions - these should be skipped by the check */
        asm volatile("# DEBUG NOTE 1" : : : "memory");
        asm volatile("# DEBUG NOTE 2" : : : "memory");
        __attribute__((used)) volatile int debug_var = 0;
        
        /* CRITICAL: Modify condition variable BEFORE first real instruction */
        /* This modification should be detected by modified_in_p */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional statements to create multi-instruction block */
        local_mod = use_value(x);
        result = local_mod * 2;
        data[0] = result;
        
        /* More operations to ensure block is substantial */
        for (int i = 1; i < 4; i++) {
            data[i] = data[i-1] + i;
        }
        
        result += use_value(y);
        /* END: then_bb */
    } else {
        /* else block with different computation */
        result = use_value(x) - use_value(y);
        global_cond = threshold - 1;
    }
    
    /* Ensure result is used */
    return result + global_cond;
}

/* Second test case with different condition type */
__attribute__((noinline, noclone))
int test_pointer_modification(int* arr, int size) {
    int sum = 0;
    volatile int* volatile cond_ptr = &arr[0];
    
    /* Condition with pointer dereference */
    if (cond_ptr != NULL && *cond_ptr < size) {
        /* Debug notes */
        asm volatile("# Pointer test note" : : : "memory");
        
        /* Modify through pointer that affects condition */
        *cond_ptr = size + 1;  /* Modifies dereferenced value */
        
        /* Additional code */
        for (int i = 0; i < size && i < 10; i++) {
            sum += arr[i];
            asm volatile("# Loop note" : : : "memory");
        }
        
        sum *= 2;
    } else {
        sum = -1;
    }
    
    return sum;
}

/* Third test with function call in condition */
static volatile int counter = 0;
__attribute__((noinline, noclone))
int condition_func(void) {
    return counter++;
}

__attribute__((noinline, noclone))
int test_function_condition(int val) {
    int ret = 0;
    
    /* Function call in condition */
    if (condition_func() > val && global_cond < 10) {
        /* Notes/debug */
        asm volatile("# Function cond note 1" : : : "memory");
        asm volatile("# Function cond note 2" : : : "memory");
        
        /* Modify global used in condition */
        global_cond = 15;  /* Makes condition false for next evaluation */
        
        /* Work */
        ret = val * 3;
        for (int i = 0; i < 3; i++) {
            ret += i;
            data[i] = ret;
        }
    } else {
        ret = val / 2;
        global_cond = 5;
    }
    
    return ret;
}

int main(void) {
    int total = 0;
    
    /* Initialize */
    global_cond = 0;
    threshold = 5;
    
    /* Test multiple paths to exercise if-conversion */
    for (int i = 0; i < 10; i++) {
        global_cond = i;
        
        /* Call test function with varying inputs */
        total += test_if_conversion(i, i * 2);
        total += test_if_conversion(i * 3, i);
        
        /* Test pointer version */
        int arr[5] = {1, 2, 3, 4, 5};
        total += test_pointer_modification(arr, 5);
        
        /* Test function call condition */
        total += test_function_condition(i);
        
        /* Vary threshold to change branch behavior */
        threshold = (i % 3) + 3;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional volatile store to force memory operations */
    asm volatile("" : : "r"(total) : "memory");
    
    return total > 0 ? 0 : 1;
}
