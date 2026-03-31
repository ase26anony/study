/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent excessive optimization */
volatile int global_seed = 42;

/* Function to create runtime-dependent values */
int fill_delay_slots_test(int argc, char **argv) {
    /* Declare and initialize local variables */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Mix with global to prevent constant propagation */
    int mod_base = global_seed % 10 + 5;
    
    /* Main loop to provide scheduling context */
    for (int i = 0; i < loop_limit; ++i) {
        /* Create a conditional check based on runtime values */
        /* The condition should not be trivially true/false */
        if ((i % mod_base) == 0) {
            /* This goto creates a simple conditional jump */
            /* The compiler should generate a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Some intermediate computations to create register pressure */
        a = b + c;
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        continue;
        
        /* Target label - the instruction after this is the candidate for delay slot */
        target_label:
        /* CRITICAL: This is the candidate instruction (next_trial) */
        /* Simple, safe arithmetic that doesn't trap */
        a = b + c;  /* Should compile to a simple add instruction */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d - a;
        
        /* More computations to create live ranges */
        result += (a ^ b) | (c & d);
    }
    
    /* Use the variables to create observable side effects */
    /* This prevents dead code elimination */
    int checksum = a + b * 3 + c * 5 + d * 7 + result;
    
    /* Print to ensure code isn't optimized away */
    if (argc > 2) {
        printf("Checksum: %d\n", checksum);
    }
    
    return checksum;
}

/* Second test case with different pattern */
int alternative_test(int iterations) {
    int x = 0, y = 1, z = 2, w = 3;
    int temp = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Another conditional jump pattern */
        if ((i & 0x3) == 0) {
            goto alt_target;
        }
        
        x = y + z;
        y = z * w;
        continue;
        
        alt_target:
        /* Candidate instruction for delay slot */
        z = x + y;  /* Simple register-to-register operation */
        
        w = z - x;
        temp += w;
    }
    
    return x + y + z + w + temp;
}

/* Main function with multiple test patterns */
int main(int argc, char **argv) {
    int result1 = fill_delay_slots_test(argc, argv);
    
    /* Use command line to vary behavior */
    int iters = (argc > 1) ? atoi(argv[1]) : 50;
    if (iters < 10) iters = 10;
    
    int result2 = alternative_test(iters);
    
    /* Combine results to prevent optimization */
    int final_result = result1 ^ result2;
    
    /* Ensure side effect */
    if (final_result != 0 || argc > 3) {
        printf("Final: %d\n", final_result);
    }
    
    return final_result & 0xFF;
}
