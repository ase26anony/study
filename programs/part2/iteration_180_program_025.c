/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slots_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile for some to prevent optimization */
    volatile int init = argc;
    int a = 0, b = 1, c = 2, d = 3;
    int result = 0;
    
    /* Create runtime-dependent loop bounds */
    int loop_count = (argc > 1) ? 100 : 200;
    
    /* Mix of operations to create register pressure and scheduling opportunities */
    for (int i = 0; i < loop_count; ++i) {
        /* Use volatile to prevent constant propagation */
        volatile int cond_mod = init + i;
        
        /* 
         * KEY CONSTRUCT: Conditional jump with potential delay slot candidate
         * The condition uses runtime values to prevent dead code elimination
         */
        if ((cond_mod % 7) == 0) {
            /* 
             * Use goto to create a simplejump_p to a label
             * The compiler should generate a conditional branch to target_label
             */
            goto target_label;
        }
        
        /* Some intermediate computations to separate the label from the goto */
        a = b + c;          /* Simple arithmetic - potential delay slot candidate */
        b = c * d;          /* Another computation */
        c = d - a;          /* Keep variables alive */
        d = a ^ b;          /* Bitwise operation */
        
        /* Skip the target code when not jumping */
        continue;
        
        /* 
         * TARGET LABEL: Place simple, safe instruction here
         * This should be the instruction considered for delay slot filling
         * Must be non-jump, non-trapping, resource-independent
         */
    target_label:
        /* Simple arithmetic that doesn't trap and uses different registers than condition */
        a = b + c;          /* Candidate for delay slot filling */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        result += a + b;
    }
    
    /* Create observable side effect to prevent dead code elimination */
    result += a + b + c + d;
    
    /* Use result so it can't be optimized away */
    printf("Result: %d\n", result);
    return result;
}

/* Main function with command line arguments */
int main(int argc, char **argv) {
    /* Call the test function multiple times to increase scheduling opportunities */
    int total = 0;
    for (int run = 0; run < 3; ++run) {
        total += fill_delay_slots_test(argc + run, argv);
    }
    
    /* Return value based on computation to prevent optimization */
    return (total > 0) ? 0 : 1;
}

/* Additional function to create more complex control flow */
void create_more_context(int x) {
    int temp = x;
    volatile int v = 0;
    
    /* Another conditional jump pattern */
    for (int i = 0; i < 50; i++) {
        if ((temp++ % 11) == 0) {
            /* Force a jump to label */
            goto another_label;
        }
        
        temp = temp * 3 + 1;
        continue;
        
    another_label:
        /* Another delay slot candidate */
        v = temp + 1;
        temp = v * 2;
    }
}
