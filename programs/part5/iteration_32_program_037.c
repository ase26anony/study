/* ifcvt_test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int buffer[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone, noipa))
static int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond < threshold && *(&buffer[0]) != 0) {
        /* This assignment modifies a variable used in the condition
           and occurs in the block header before then_last_head */
        global_cond = x + y;  /* MODIFIES CONDITION VARIABLE */
        
        /* Generate NOTE/DEBUG_INSN instructions in the header */
        asm volatile("# DEBUG/NOTE: Start of then block" : : : "memory");
        asm volatile("# Another note" : : : "memory");
        
        /* Additional statements to create multi-instruction block */
        local_mod = global_cond * 2;
        result = local_mod + (x - y);
        
        /* More operations to ensure block isn't optimized away */
        for (int i = 0; i < 3; i++) {
            buffer[i] = result + i;
        }
        
        /* Prevent tail merging */
        asm volatile("" : : : "memory");
    } else {
        /* Else block with different computation */
        result = x * y - global_cond;
        buffer[0] = result;
    }
    
    return result;
}

/* Another test with different condition pattern */
__attribute__((noinline, noclone, noipa))
static int test_pointer_modification(int val) {
    static volatile int* cond_ptr = NULL;
    static volatile int data[5] = {1, 2, 3, 4, 5};
    
    if (cond_ptr == NULL) {
        cond_ptr = &data[0];
    }
    
    /* Condition using pointer dereference */
    if (*cond_ptr > 2 && global_cond < 10) {
        /* Modify the pointer target which is part of condition */
        *cond_ptr = val;  /* MODIFIES DEREFERENCED CONDITION VARIABLE */
        
        /* Generate notes/debug insns */
        asm volatile("# Note: modifying condition pointer" : : : "memory");
        asm volatile("# Another debug marker" : : : "memory");
        
        /* Additional code */
        int temp = *cond_ptr * 3;
        data[1] = temp;
        
        return temp + global_cond;
    }
    
    return val;
}

/* Test with compound condition */
__attribute__((noinline, noclone, noipa))
static int test_compound_condition(int a, int b, int c) {
    volatile static int cond1 = 0;
    volatile static int cond2 = 0;
    int result = 0;
    
    /* Compound condition */
    if ((cond1 > a) && (cond2 < b) && (global_cond != c)) {
        /* Modify one of the condition variables */
        cond1 = b * 2;  /* MODIFIES CONDITION VARIABLE */
        
        /* Notes in header */
        asm volatile("# Compound condition block" : : : "memory");
        
        /* Body of then block */
        result = cond1 + cond2 + global_cond;
        for (int i = 0; i < 4; i++) {
            buffer[i] += result;
        }
        
        return result;
    }
    
    return a + b + c;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 3;
    threshold = 7;
    buffer[0] = 1;  /* Make pointer condition true */
    
    /* Test 1: Global variable modification in then block */
    printf("Test 1: global condition modification\n");
    for (int i = 0; i < 5; i++) {
        results[0] += test_if_conversion(i, i * 2);
        /* Vary global to take different paths */
        global_cond = (global_cond + 1) % 10;
    }
    
    /* Test 2: Pointer-based condition */
    printf("Test 2: pointer modification\n");
    ptr = (int*)buffer;
    for (int i = 0; i < 4; i++) {
        results[1] += test_pointer_modification(i * 3);
    }
    
    /* Test 3: Compound condition */
    printf("Test 3: compound condition\n");
    for (int i = 0; i < 6; i++) {
        results[2] += test_compound_condition(i, i+1, i+2);
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = results[0] + results[1] + results[2];
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
