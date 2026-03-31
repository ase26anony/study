/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile variable for loop control to prevent unrolling */
    volatile int iterations = 100;
    
    /* Main loop where if-conversion will be attempted */
    for (volatile int i = 0; i < iterations; i++) {
        /* Condition variable - read from volatile global */
        int test_var = global_cond;
        
        /* The critical if-statement for if-conversion */
        if (test_var > 0) {
            /* THEN BLOCK: Operations that do NOT modify test_var */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure block has multiple instructions */
            a = a + (b >> 2);
            b = b | 0x1;
        } else {
            /* ELSE BLOCK: Also doesn't modify test_var */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify test_var for next iteration (but not in the then block!) */
        /* This ensures the condition changes across iterations */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_test(int x, int y) {
    volatile int flag = rand() % 100;
    int result = 0;
    
    for (int i = 0; i < 50; i++) {
        int cond = flag + i;
        
        /* Different condition expression */
        if (cond & 0x1) {  /* Test with bitwise condition */
            /* Safe then block with multiple instructions */
            x = y * 3;
            y = x - 5;
            x = x ^ y;
            y = y << 1;
            result += x;
        } else {
            y = x + 2;
            x = y * 3;
            result -= y;
        }
        
        /* Modify condition variable outside the block */
        flag = (flag * 3) % 100;
    }
    
    return result;
}

int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Call the if-conversion candidate function */
    int result1 = if_conversion_candidate(rand() % 100, rand() % 100);
    int result2 = another_test(rand() % 100, rand() % 100);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
