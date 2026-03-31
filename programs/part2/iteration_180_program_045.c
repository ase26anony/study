/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int global_seed = 42;

/* Function to create runtime-dependent values */
int process_values(int argc, char **argv) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create non-constant loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Mix with global volatile to prevent constant propagation */
    int threshold = global_seed % 10;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create a conditional jump that's not always true/false */
        if ((i % 7) == threshold) {
            /* This goto creates a simplejump_p to a label */
            goto target_label;
        }
        
        /* Some computation to create register pressure */
        d = a + b;
        a = b ^ c;
        b = c * d;
        c = d - a;
        
        /* Skip the target instruction when not jumping */
        continue;
        
        /* Target label with a simple, safe instruction */
        /* This should be eligible for delay slot filling */
        target_label:
        /* Simple arithmetic that doesn't trap or conflict with jump resources */
        a = b + c;  /* Candidate for delay slot filling */
        
        /* Additional computation to ensure target isn't isolated */
        b = c * d;
        c = a ^ b;
        
        /* Prevent loop optimization */
        result += a + b + c + d;
    }
    
    /* Use the variables to create observable side effects */
    result = (result * 31) ^ (a * 127) ^ (b * 8191) ^ (c * 131071) ^ (d * 524287);
    
    /* Additional control flow to prevent tail merging */
    if (argc > 2) {
        result += atoi(argv[2]);
    }
    
    return result;
}

/* Second function to increase scheduling complexity */
int helper_function(int x, int y) {
    volatile int v = global_seed;
    int r1 = x + y;
    int r2 = x - y;
    int r3 = x * y;
    
    /* Another conditional jump pattern */
    if ((v % 5) == (x % 3)) {
        goto helper_label;
    }
    
    r1 = r2 ^ r3;
    return r1;
    
    helper_label:
    /* Another candidate instruction for delay slot */
    r2 = r1 + r3;  /* Simple, non-trapping arithmetic */
    r3 = r2 * 2;
    
    return r1 + r2 + r3;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Call multiple times to create different execution paths */
    for (int j = 0; j < 3; ++j) {
        total += process_values(argc, argv);
        total += helper_function(argc, j);
        
        /* Modify global to change behavior */
        global_seed += j * 17;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
