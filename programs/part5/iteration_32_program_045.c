/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 100;
static volatile int* volatile global_ptr = NULL;
static volatile int global_data[4] = {0};

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
#ifdef __x86_64__
__attribute__((target("arch=x86-64")))
#endif
int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Use a global variable in condition - creates non-trivial test_expr */
    if (global_cond < global_threshold) {
        /* 
         * This assignment modifies the condition variable IN THE HEADER
         * before any real instruction in the then block.
         * The block will start with a label, and GCC may insert
         * NOTE/DEBUG_INSN instructions before this assignment.
         */
        
        /* Force generation of NOTE/DEBUG_INSN in header */
        asm volatile("# This is a comment note" : : : "memory");
        
        /* CRITICAL: Modify the condition variable immediately */
        global_cond = x + y + 50;  /* Modifies test_expr component */
        
        /* Additional statements to create multi-instruction block */
        local_mod = global_cond * 2;
        result = local_mod - 10;
        
        /* More operations to ensure block isn't trivial */
        global_data[0] = result;
        global_data[1] = x * y;
        
        /* Use pointer to create memory references */
        if (global_ptr) {
            *global_ptr = result;
        }
    } else {
        /* else block with different computation */
        result = x - y;
        global_cond = result * 3;
    }
    
    /* Ensure result is used */
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone))
int test_pointer_condition(int val) {
    static volatile int static_data = 0;
    volatile int* ptr = &static_data;
    int temp = 0;
    
    /* Condition with pointer dereference */
    if (*ptr < val && global_threshold > 50) {
        /* Header with note/comment */
        asm volatile("# Pointer condition block" : : : "memory");
        
        /* Modify what the condition depends on */
        *ptr = val * 2;  /* Modifies memory referenced in test_expr */
        
        /* Additional code */
        temp = *ptr + global_data[0];
        global_data[2] = temp;
        
        /* Complex enough to avoid simplification */
        for (int i = 0; i < 2; i++) {
            temp += i;
        }
    } else {
        temp = val / 2;
        *ptr = temp;
    }
    
    return temp;
}

/* Test with compound condition */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b, int c) {
    volatile int mod_var = a;
    int ret = 0;
    
    /* Compound condition where one part gets modified */
    if (mod_var > b && global_cond < c) {
        /* Generate notes/debug in header */
        asm volatile("# Compound condition note 1" : : : "memory");
        asm volatile("# Compound condition note 2" : : : "memory");
        
        /* Modify variable used in condition */
        mod_var = b - 10;  /* Affects first part of compound condition */
        
        /* Body of then block */
        ret = mod_var * c;
        global_data[3] = ret;
        
        /* Call to prevent tail optimization */
        if (ret > 1000) {
            ret /= 2;
        }
    } else {
        ret = a + b + c;
        mod_var = ret;
    }
    
    return ret;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 50;
    global_threshold = 100;
    global_ptr = (int*)&global_data[0];
    
    /* Exercise different paths through the if-conversion */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to take both branches */
        int x = (i % 3) * 40;
        int y = (i % 2) * 60;
        
        results[0] += test_if_conversion(x, y);
        results[1] += test_pointer_condition(x + y);
        results[2] += test_compound_condition(x, y, i * 10);
        
        /* Change global to affect future iterations */
        global_cond += i;
        global_threshold -= (i % 2);
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = results[0] + results[1] + results[2];
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
