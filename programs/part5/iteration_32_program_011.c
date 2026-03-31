/* ifcvt-test.c - Test case for if-conversion header modification check */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
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
        /* This assignment modifies the condition variable in the block header */
        /* Before any real instruction in the then block */
        global_cond = x + y;  /* MODIFIES CONDITION VARIABLE */
        
        /* Generate notes/debug insns via asm comments */
        asm volatile("# NOTE: Start of then block");
        asm volatile("# DEBUG: global_cond modified");
        
        /* Additional statements to create multi-instruction block */
        local_mod = global_cond * 2;
        result = local_mod - threshold;
        
        /* More operations to ensure block isn't optimized away */
        data[0] = result;
        asm volatile("" : : : "memory");  /* Compiler barrier */
        
        /* Function call to create complex RTL */
        result += abs(local_mod);
    } else {
        /* Else block with different computation */
        result = x - y;
        global_cond = result / 2;
    }
    
    return result;
}

/* Another test with pointer-based condition */
__attribute__((noinline, noclone, noipa))
static int test_pointer_cond(int val) {
    static volatile int target = 0;
    int* volatile local_ptr = &target;
    int result = 0;
    
    /* Condition using pointer dereference */
    if (*local_ptr < val && global_cond > 0) {
        /* Modify through pointer - affects condition */
        *local_ptr = val * 2;  /* MODIFIES DEREFERENCED POINTER */
        
        /* Generate notes */
        asm volatile("# NOTE: Pointer modification block");
        
        /* Additional code */
        result = *local_ptr + global_cond;
        global_cond = result / 3;
        
        /* Complex computation */
        for (int i = 0; i < 3; i++) {
            result += data[i];
        }
    } else {
        result = val - target;
        target = result;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone, noipa))
static int test_compound_cond(int a, int b, int c) {
    volatile static int cond1 = 0, cond2 = 0;
    int result = 0;
    
    /* Compound condition */
    if ((cond1 < a) && (cond2 > b) && (global_cond != c)) {
        /* Modify one condition variable immediately */
        cond1 = a + b;  /* MODIFIES CONDITION VARIABLE */
        
        /* Debug notes */
        asm volatile("# DEBUG: cond1 modified in header");
        asm volatile("# NOTE: Compound condition block");
        
        /* Additional operations */
        cond2 = c - a;
        result = cond1 * cond2;
        
        /* Memory operations */
        data[1] = result;
        asm volatile("" : : : "memory");
        
        /* More computations */
        result += global_cond * 2;
    } else {
        result = a + b + c;
        cond1 = result;
        cond2 = result / 2;
    }
    
    return result;
}

int main(void) {
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 2;
    threshold = 10;
    ptr = (int*)&global_cond;
    
    /* Initialize data array */
    for (int i = 0; i < 10; i++) {
        data[i] = i * 2;
    }
    
    /* Call test functions multiple times with different inputs
       to ensure if-conversion pass is invoked */
    for (int i = 0; i < 5; i++) {
        results[0] += test_function(i, i * 2);
        results[1] += test_pointer_cond(i + 1);
        results[2] += test_compound_cond(i, i + 1, i + 2);
        
        /* Vary global condition to take different paths */
        global_cond = (global_cond + 3) % 8;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", results[0], results[1], results[2]);
    
    return 0;
}
