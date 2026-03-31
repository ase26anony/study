/* ifcvt_test.c - Test case for GCC if-conversion pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int data_array[10] = {0};

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
int test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should create a non-trivial test_expr in RTL */
    if (global_cond > 0 && *global_ptr < 100) {
        /* 
         * CRITICAL: Modify the condition variable BEFORE any real instruction
         * The block starts with a label, then we need notes/debug insns
         */
        
        /* Generate NOTE instructions using inline asm comments */
        asm volatile("# NOTE: Entering then block");
        asm volatile("# DEBUG: x = %0" : : "r"(x));
        asm volatile("# DEBUG: y = %0" : : "r"(y));
        
        /* 
         * This modifies global_cond which is part of the condition
         * It happens BEFORE then_last_head in the header section
         */
        global_cond = x * 2;  /* MODIFIES CONDITION VARIABLE */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Now the "real" instructions start */
        result = y + 10;
        data_array[result % 10] = x;
        
        /* More operations to ensure block has multiple instructions */
        for (int i = 0; i < 3; i++) {
            result += data_array[i];
        }
        
        /* Another asm to generate notes */
        asm volatile("# NOTE: Processing data");
    } else {
        /* Else block with different operations */
        result = x - y;
        global_ptr = &data_array[5];
    }
    
    /* Use result to prevent dead code elimination */
    return result + global_cond;
}

/* Another test with different condition structure */
__attribute__((noinline, noclone))
int test_pointer_condition(int threshold) {
    static int local_static = 50;
    int* ptr = &local_static;
    int value = 0;
    
    /* Condition with pointer dereference */
    if (*ptr > threshold && global_cond != 0) {
        /* Generate debug/note instructions first */
        asm volatile("# DEBUG: threshold = %0" : : "r"(threshold));
        asm volatile("# NOTE: Pointer condition met");
        
        /* Modify *ptr which is part of the condition */
        *ptr = threshold - 5;  /* MODIFIES DEREFERENCED POINTER */
        
        asm volatile("" : : : "memory");
        
        /* Real instructions after modification */
        value = *ptr * 2;
        global_cond = value / 3;
        
        asm volatile("# NOTE: Value computed");
    } else {
        value = threshold * 3;
        ptr = &global_cond;
    }
    
    return value + *ptr;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b, int c) {
    volatile int cond1 = a;
    static int cond2 = 100;
    
    if ((cond1 > b) && (cond2 < c)) {
        /* Notes/debug first */
        asm volatile("# DEBUG: a=%0, b=%1, c=%2" : : "r"(a), "r"(b), "r"(c));
        asm volatile("# NOTE: Compound condition");
        
        /* Modify cond2 which is in the condition */
        cond2 = a + b;  /* MODIFIES CONDITION VARIABLE */
        
        asm volatile("" : : : "memory");
        
        /* Real work */
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += data_array[i] + i;
        }
        
        asm volatile("# NOTE: Loop completed");
        return sum + cond2;
    }
    
    return a + b + c;
}

int main() {
    int total = 0;
    
    /* Initialize global data */
    global_cond = 1;
    int local_data = 42;
    global_ptr = &local_data;
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 10; i++) {
        global_cond = i % 3;
        local_data = 50 + i;
        
        /* Exercise the critical path */
        total += test_if_conversion(i, i * 2);
        
        /* Also test else path */
        if (i % 2 == 0) {
            global_cond = -1;  /* Force else path */
            total += test_if_conversion(i, i * 2);
        }
        
        /* Test other functions */
        total += test_pointer_condition(i * 10);
        total += test_compound_condition(i, i + 5, i + 10);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
