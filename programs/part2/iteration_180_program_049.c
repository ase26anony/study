/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_mod = 7;

/* Function to create runtime-dependent values */
int process_values(int argc, char **argv) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Create a complex enough control flow to survive optimization */
    for (int i = 0; i < iterations; ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        d = temp + global_seed;
        
        /* Critical construct: conditional jump with potential delay slot candidate */
        /* The condition must not be trivially true/false */
        if ((i % global_mod) == (argc & 3)) {
            /* Jump to label where a simple, safe instruction follows */
            goto target_label;
        }
        
        /* Alternative path with different operations */
        a = c - b;
        b = d ^ a;
        continue;
        
    target_label:
        /* Candidate instruction for delay slot filling */
        /* Simple, non-trapping, resource-independent operation */
        a = b + c;  /* This should be the 'next_trial' instruction */
        
        /* Additional operations to ensure it's not the only instruction in block */
        c = d | 0xFF;
        d = a * 2;
    }
    
    /* Use results to prevent dead code elimination */
    result = a ^ b ^ c ^ d;
    
    /* Additional loop to create more scheduling context */
    for (int j = 0; j < 10; ++j) {
        /* More operations that use different registers */
        int x = result + j;
        int y = x * global_seed;
        result = y & 0xFFFF;
        
        /* Another conditional jump structure */
        if ((j & 1) == (argc & 1)) {
            /* Different target with simple instruction */
            result += global_mod;
        } else {
            result -= global_mod;
        }
    }
    
    return result;
}

/* Second function with similar pattern but different register usage */
int secondary_pattern(int base) {
    int p = base, q = base + 1, r = base + 2, s = base + 3;
    volatile int mod = 5;  /* volatile to prevent constant propagation */
    
    for (int k = 0; k < 50; ++k) {
        /* Create register pressure with different operations */
        int tmp = q * r;
        s = tmp - p;
        
        /* Another conditional jump candidate */
        if ((k % mod) == (base & 1)) {
            goto alt_target;
        }
        
        p = r + s;
        q = p ^ k;
        continue;
        
    alt_target:
        /* Another simple candidate instruction */
        p = q - r;  /* Simple subtraction, non-trapping */
        
        /* Follow-up operations */
        r = s & 0x7F;
        s = p + k;
    }
    
    return p + q + r + s;
}

int main(int argc, char **argv) {
    int result1, result2, final_result;
    
    /* Process command line to create runtime-dependent values */
    result1 = process_values(argc, argv);
    result2 = secondary_pattern(argc);
    
    /* Combine results in non-trivial way */
    final_result = (result1 * 31) ^ (result2 * 17);
    
    /* Print to create observable side effect */
    printf("Result: %d\n", final_result);
    
    return (final_result == 0) ? 1 : 0;
}
