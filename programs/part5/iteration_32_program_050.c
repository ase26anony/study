/* ifcvt-test.c - Test case for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int threshold = 5;
static volatile int* volatile ptr = NULL;
static volatile int buffer[10] = {0};

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
    if ((global_cond < threshold) && (ptr != NULL) && (*ptr > 0)) {
        /* HEADER SECTION START (target for uncovered lines) */
        /* Label will be generated here by GCC for the basic block start */
        
        /* Generate NOTE/DEBUG_INSN instructions using asm comments */
        /* These should appear before the first "real" instruction */
        asm volatile("# DEBUG NOTE 1: Beginning of then block");
        asm volatile("# DEBUG NOTE 2: Test condition variables");
        
        /* CRITICAL: Modify condition variable BEFORE then_last_head */
        /* This modification should be detected by modified_in_p() */
        global_cond = x + y;  /* Modifies variable used in condition */
        
        /* Additional asm notes after modification */
        asm volatile("# DEBUG NOTE 3: After condition modification");
        
        /* Now the "real" instructions begin - this marks then_last_head */
        /* Additional statements to create a multi-instruction block */
        local_mod = global_cond * 2;
        result = local_mod + (x - y);
        
        /* More operations to ensure block isn't optimized away */
        buffer[0] = result;
        buffer[1] = global_cond;
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
    } else {
        /* Else block with different computation */
        result = x * y - global_cond;
        ptr = &buffer[0];
        global_cond = threshold - 1;
    }
    
    /* Use result to prevent dead code elimination */
    return result + buffer[0];
}

/* Another test function with different condition pattern */
__attribute__((noinline, noclone))
int test_pointer_modification(int val) {
    static volatile int* static_ptr = NULL;
    static volatile int target = 0;
    
    /* Initialize pointer on first call */
    if (static_ptr == NULL) {
        static_ptr = &target;
    }
    
    /* Condition with pointer dereference */
    if (static_ptr != NULL && *static_ptr < val) {
        /* Header with notes/debug insns */
        asm volatile("# Note: Pointer modification test");
        
        /* Modify through pointer - affects condition expression */
        *static_ptr = val * 2;  /* This modifies memory used in condition */
        
        asm volatile("# Note: After pointer store");
        
        /* Real instructions */
        int temp = *static_ptr + 10;
        target = temp;
        
        /* Additional operations */
        for (int i = 0; i < 3; i++) {
            buffer[i] = temp + i;
        }
        
        return temp;
    }
    
    return val;
}

/* Function with compound condition */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b, int c) {
    volatile int cond1 = a;
    volatile int cond2 = b;
    int result = 0;
    
    /* Compound condition where one part gets modified */
    if ((cond1 > 0) && (cond2 < c) && (global_cond != 0)) {
        /* Notes in header */
        asm volatile("# Compound condition test header");
        
        /* Modify one condition variable */
        cond1 = b * 2;  /* Affects condition expression */
        
        asm volatile("# After cond1 modification");
        
        /* Block body */
        result = cond1 + cond2 + global_cond;
        
        /* Multiple operations */
        for (int i = 0; i < 5; i++) {
            result += buffer[i % 10];
        }
        
        global_cond = result % 100;
    } else {
        result = a + b + c;
        cond1 = result;
        cond2 = result * 2;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Initialize global state */
    global_cond = 3;
    threshold = 10;
    ptr = &buffer[0];
    buffer[0] = 1;
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 20; i++) {
        /* Vary inputs to take both branches */
        if (i % 3 == 0) {
            global_cond = i;
            ptr = NULL;  /* Force else branch */
        } else if (i % 3 == 1) {
            global_cond = 0;  /* Below threshold */
            ptr = &buffer[0];
            buffer[0] = i;  /* Positive value */
        } else {
            global_cond = 20;  /* Above threshold */
            ptr = &buffer[0];
            buffer[0] = -1;  /* Non-positive */
        }
        
        /* Exercise the main test function */
        total += test_if_conversion(i, i * 2);
        
        /* Exercise pointer modification test */
        total += test_pointer_modification(i);
        
        /* Exercise compound condition test */
        total += test_compound_condition(i, i + 1, i + 2);
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
