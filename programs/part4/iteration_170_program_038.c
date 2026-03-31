/* Target: ifcvt.cc lines 577-583 - validation that "then" block doesn't modify condition */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to preserve control flow */
__attribute__((noinline)) 
int ifcvt_target_function(volatile int cond_init, int start_a, int start_b) {
    volatile int cond = cond_init;  /* volatile prevents constant propagation */
    int a = start_a;
    int b = start_b;
    
    /* Volatile loop limit prevents unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* This is the test_expr - condition variable 'cond' */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Line 1: arithmetic on different variable */
            b = a * 2;      /* Line 2: arithmetic on different variable */
            a = a ^ b;      /* Line 3: bitwise operation */
            b = b & 0xFF;   /* Line 4: bitwise operation with constant */
            
            /* Add more non-modifying instructions to ensure the loop
               in ifcvt.cc has multiple iterations to check */
            a = a + i;      /* Line 5: use loop index */
            b = b - 1;      /* Line 6: decrement */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify 'cond' here - outside the conditional blocks */
        /* This ensures the condition changes across iterations but
           isn't modified within the then/else blocks being validated */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Add some loop-carried dependency to prevent over-optimization */
        a = a + (cond & 1);
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

int main() {
    /* Use volatile source for initial condition */
    volatile int seed = 42;
    
    /* Call the target function multiple times with different inputs
       to increase chance of if-conversion analysis */
    int result = 0;
    for (int j = 0; j < 10; j++) {
        result += ifcvt_target_function(seed + j, j * 10, j * 20);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
