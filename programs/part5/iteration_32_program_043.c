/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone, noipa))
static int test_if_conversion(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond < threshold && *(&global_cond) != 0) {
        /* This assignment modifies the condition variable in the block header */
        /* Before any real instruction in the then block */
        
        /* First, generate some NOTE/DEBUG_INSN instructions */
        asm volatile("# THEN BLOCK START - NOTE 1" : : : "memory");
        asm volatile("# NOTE 2 - Before modification" : : : "memory");
        
        /* CRITICAL: Modify condition variable before first real instruction */
        /* This should be in the header portion of then_bb */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* More notes/debug insns after modification */
        asm volatile("# NOTE 3 - After modification" : : : "memory");
        
        /* Now the "real" instructions begin */
        local_mod = global_cond * 2;
        result = local_mod + (x - y);
        
        /* Additional instructions to ensure block has body */
        for (int i = 0; i < 3; i++) {
            data[i] = result + i;
        }
        
        asm volatile("# THEN BLOCK END" : : : "memory");
    } else {
        /* Else block with different computation */
        result = x * y - global_cond;
        asm volatile("# ELSE BLOCK" : : : "memory");
    }
    
    return result;
}

/* Another test case with pointer-based condition */
__attribute__((noinline, noclone, noipa))
static int test_pointer_condition(int val) {
    static volatile int target = 0;
    volatile int* volatile cond_ptr = &target;
    int output = 0;
    
    /* Condition using pointer dereference */
    if (*cond_ptr < val && (target > 0 || val < 100)) {
        /* Header with notes first */
        asm volatile("# PTR TEST - NOTE A" : : : "memory");
        asm volatile("# PTR TEST - NOTE B" : : : "memory");
        
        /* Modify through pointer - affects condition */
        *cond_ptr = val * 2;
        
        asm volatile("# PTR TEST - NOTE C" : : : "memory");
        
        /* Body instructions */
        output = *cond_ptr + target;
        for (int i = 0; i < 4; i++) {
            output += data[i];
        }
    } else {
        output = val - target;
    }
    
    return output;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone, noipa))
static int test_compound_condition(int a, int b, int c) {
    volatile static int cond1 = 0, cond2 = 0;
    int res = 0;
    
    /* Compound condition */
    if ((cond1 < a) && (cond2 > b) && (c != 0)) {
        /* Multiple notes in header */
        asm volatile("# COMPOUND - Header note 1" : : : "memory");
        asm volatile("# COMPOUND - Header note 2" : : : "memory");
        asm volatile("# COMPOUND - Header note 3" : : : "memory");
        
        /* Modify one condition variable */
        cond1 = a + c;
        
        asm volatile("# COMPOUND - After mod note" : : : "memory");
        
        /* Block body */
        res = cond1 * cond2;
        cond2 = b - c;
        res += cond2;
        
        /* More operations */
        for (volatile int i = 0; i < 2; i++) {
            res += i;
            asm volatile("# LOOP IN THEN" : : : "memory");
        }
    } else {
        res = a + b + c;
        cond1 = b;
        cond2 = c;
    }
    
    return res;
}

int main(void) {
    int total = 0;
    
    /* Initialize */
    ptr = (int*)&global_cond;
    global_cond = 2;
    threshold = 10;
    
    /* Call test functions multiple times to ensure if-conversion pass runs */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to exercise different paths */
        total += test_if_conversion(i, i * 2);
        total += test_pointer_condition(i);
        total += test_compound_condition(i, i + 1, i + 2);
        
        /* Change global condition periodically */
        if (i % 3 == 0) {
            global_cond = i;
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
