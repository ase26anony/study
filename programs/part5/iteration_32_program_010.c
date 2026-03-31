/* ifcvt-test.c - Test case for GCC if-conversion header modification check */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 5;
static int data[10] = {0};

/* Function to prevent optimization */
__attribute__((noinline, noclone)) 
int test_if_conversion(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *global_ptr != 0) {
        /* This assignment modifies global_cond which is part of the condition.
           It occurs in the block header before any real instruction */
        global_cond = x + y;  /* MODIFICATION IN HEADER */
        
        /* Generate NOTE/DEBUG_INSN instructions in the header */
        asm volatile("# DEBUG/NOTE: Start of then block" : : : "memory");
        asm volatile("# Another note instruction" : : : "memory");
        
        /* Additional statements to create a multi-instruction block */
        result = x * y;
        data[result % 10] = global_cond;
        result += *global_ptr;
        
        /* More operations to ensure block isn't optimized away */
        for (int i = 0; i < 3; i++) {
            result += data[i];
        }
    } else {
        result = x - y;
        global_ptr = &result;
    }
    
    return result;
}

/* Another test with different condition structure */
__attribute__((noinline, noclone))
int test_pointer_modification(int val) {
    static int counter = 0;
    int* local_ptr = &counter;
    
    /* Condition involving pointer dereference */
    if (*local_ptr < val && global_cond > 0) {
        /* Modify the pointer target which affects the condition */
        *local_ptr = val * 2;  /* MODIFICATION IN HEADER */
        
        /* Insert asm comments that become NOTE instructions */
        asm volatile("# NOTE: modifying condition variable" : : : "memory");
        asm volatile("# DEBUG info here" : : : "memory");
        
        /* Additional code */
        int temp = val;
        for (int i = 0; i < 4; i++) {
            temp += local_ptr[i % 2];
        }
        return temp;
    }
    
    return val;
}

/* Test with compound condition where one part is modified */
__attribute__((noinline, noclone))
int test_compound_condition(int a, int b) {
    static int cond_a = 0, cond_b = 0;
    
    /* Compound condition */
    if ((cond_a > a) && (cond_b < b) && (global_cond != 0)) {
        /* Modify cond_a which is part of the condition */
        cond_a = b - a;  /* MODIFICATION IN HEADER */
        
        /* Multiple asm notes */
        asm volatile("# First note" : : : "memory");
        asm volatile("# Second note" : : : "memory");
        asm volatile("# Third note" : : : "memory");
        
        /* Real work */
        int sum = 0;
        for (int i = 0; i < a; i++) {
            sum += data[i % 10];
        }
        cond_b = sum;
        return sum + cond_a;
    }
    
    return a + b;
}

int main(void) {
    int result = 0;
    
    /* Initialize globals */
    global_cond = 3;
    int local_var = 10;
    global_ptr = &local_var;
    
    /* Call test functions multiple times to exercise different paths */
    for (int i = 0; i < 10; i++) {
        global_cond = i;
        threshold = i / 2;
        
        /* Vary inputs to take both branches */
        if (i % 3 == 0) {
            local_var = 0;  /* Force else branch */
        } else {
            local_var = i + 1;  /* Force then branch */
        }
        
        result += test_if_conversion(i, i * 2);
        result += test_pointer_modification(i);
        result += test_compound_condition(i, i + 1);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
