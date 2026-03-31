/* Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static int use_result(int x) {
    volatile int sink = x;
    return sink;
}

/* Main function with carefully constructed control flow */
int main(int argc, char *argv[]) {
    /* Initialize variables - use volatile to prevent constant propagation */
    volatile int init = argc;
    int a = init + 1;
    int b = init + 2;
    int c = init + 3;
    int d = init + 4;
    int e = init + 5;
    
    /* Loop to create scheduling context */
    int iterations = (argc > 1) ? 100 : 200;
    int sum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition to prevent dead code elimination */
        int condition = (i * 17 + argc) % 13;
        
        /* 
         * KEY CONSTRUCT: Conditional jump to label with simple instruction at target
         * This should create a simplejump_p to a label where the next instruction
         * is eligible for delay slot filling
         */
        if (condition == 0) {
            /* Use goto to create explicit label jump */
            goto target_label;
        }
        
        /* Some intermediate computation to separate blocks */
        a = b + c;
        continue_after_label:
        b = c * d;
        c = d - a;
        
        /* Skip the target block when not jumping */
        goto skip_target;
        
        target_label:
        /* 
         * TARGET INSTRUCTION: Simple, safe arithmetic operation
         * - Not a jump
         * - Doesn't reference CC or stack pointer
         * - Non-trapping (addition is safe)
         * - Should compile to single RTL instruction
         */
        d = e + a;  /* Simple register-to-register operation */
        
        /* Continue normal flow */
        goto continue_after_label;
        
        skip_target:
        /* Additional computation to use variables */
        e = a ^ b;
        sum += e;
    }
    
    /* Use results to prevent optimization */
    int result = use_result(a) + use_result(b) + use_result(c) + 
                 use_result(d) + use_result(e) + sum;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}

/* Additional function to increase compilation complexity */
void helper_func(int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        /* Create another conditional jump pattern */
        if ((i & 3) == 0) {
            arr[i] = arr[i] + arr[i+1];
        } else {
            arr[i] = arr[i] - arr[i-1];
        }
    }
}
