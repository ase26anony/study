/* Program to trigger if-conversion header modification check */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone, noipa))
static int test_function(int x, int y) {
    volatile int local_mod = 0;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond < threshold && *(&global_cond) != 0) {
        /* This is the 'then' block - header starts here */
        
        /* Generate NOTE/DEBUG_INSN instructions via asm comments */
        asm volatile("# HEADER: Debug note 1");
        asm volatile("# HEADER: Debug note 2");
        asm volatile("# HEADER: Debug note 3");
        
        /* CRITICAL: Modify condition variable BEFORE first real instruction
           This should be in the header section before then_last_head */
        global_cond = x + y;  /* Modifies test_expr variable */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Additional instructions to create a proper block */
        local_mod = global_cond * 2;
        data[0] = local_mod;
        result = data[0] + x;
        
        /* More asm notes to ensure header has multiple notes */
        asm volatile("# MID-BLOCK: Another note");
        
        /* Additional computation */
        for (int i = 0; i < 3; i++) {
            data[i] = result + i;
        }
        
        result += y;
    } else {
        /* Else block with different computation */
        result = x - y;
        global_cond = result;
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (result) : : "memory");
    return result;
}

/* Another test case with pointer-based condition */
__attribute__((noinline, noclone, noipa))
static int test_function2(int val) {
    static volatile int* cond_ptr = NULL;
    static volatile int target = 10;
    int result = 0;
    
    /* Initialize pointer if needed */
    if (cond_ptr == NULL) {
        cond_ptr = &global_cond;
    }
    
    /* Condition using pointer dereference */
    if (*cond_ptr < target && val > 0) {
        /* Header with notes */
        asm volatile("# TEST2: Header note 1");
        asm volatile("# TEST2: Header note 2");
        
        /* Modify through pointer - affects condition expression */
        *cond_ptr = val * 2;  /* Modifies what test_expr dereferences */
        
        asm volatile("" : : : "memory");
        
        /* Additional code */
        result = *cond_ptr + 5;
        data[1] = result;
        
        /* Loop to create more instructions */
        for (int i = 0; i < 4; i++) {
            data[i] += result;
        }
    } else {
        result = val - target;
        *cond_ptr = result;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone, noipa))
static int test_function3(int a, int b, int c) {
    volatile static int cond1 = 0;
    volatile static int cond2 = 0;
    int result = 0;
    
    /* Initialize */
    cond1 = a;
    cond2 = b;
    
    /* Compound condition */
    if (cond1 > 0 && cond2 < c) {
        /* Multiple notes in header */
        asm volatile("# TEST3: Note A");
        asm volatile("# TEST3: Note B");
        asm volatile("# TEST3: Note C");
        
        /* Modify one condition variable */
        cond1 = -1;  /* Modifies test_expr operand */
        
        asm volatile("" : : : "memory");
        
        /* Additional computations */
        result = cond1 + cond2;
        for (int i = 0; i < 5; i++) {
            data[i % 10] = result + i;
        }
        
        result *= 2;
    } else {
        result = cond1 - cond2;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 10; i++) {
        data[i] = i;
    }
    
    ptr = &global_cond;
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 100; i++) {
        global_cond = i % 20;
        threshold = 10;
        
        /* Vary inputs to take both branches */
        result += test_function(i, i * 2);
        result += test_function2(i % 15);
        result += test_function3(i, i + 5, i * 3);
        
        /* Alternate global_cond to affect conditions */
        global_cond = (i % 3 == 0) ? 0 : 15;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with specific values */
    global_cond = 1;
    threshold = 10;
    result = test_function(5, 3);
    printf("Test 1: %d\n", result);
    
    global_cond = 20;
    result = test_function(5, 3);
    printf("Test 2: %d\n", result);
    
    return 0;
}
