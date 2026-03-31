/* Target: reorg.cc - fill_simple_delay_slots */
/* This program creates patterns that should trigger delay slot filling:
   - Conditional jump to label
   - Simple, safe instruction at label target
   - Resource-independent operations
   - Avoids trapping instructions
   - Survives optimization passes */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
static volatile int external_seed = 42;

/* Function to create runtime-dependent values */
int process_values(int argc, char **argv) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Create some volatile values to prevent optimization */
    volatile int v1 = external_seed;
    volatile int v2 = argc * 3;
    
    /* Main loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition that's not always true/false */
        int condition = (i + v1) % 13;
        
        /* 
         * KEY CONSTRUCT: Conditional jump with potential delay slot fill
         * The compiler should see: if (condition == 0) goto target_label;
         * followed by target_label: a = b + c;
         */
        if (condition == 0) {
            /* Force use of goto to create explicit jump to label */
            goto target_label;
        }
        
        /* Alternate path - some computation */
        d = a * b + i;
        continue;
        
    target_label:
        /* 
         * TARGET INSTRUCTION: Simple, safe operation for delay slot
         * Must not reference CC, stack pointer, or conflict with jump resources
         * Simple register-to-register operation
         */
        a = b + c;  /* Simple addition, no trapping */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d + i;
        c = d ^ v2;  /* Bitwise operation - safe, no trapping */
    }
    
    /* Use results to prevent dead code elimination */
    result = a + b + c + d;
    
    /* Create observable side effect */
    if (argc > 2) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternate_pattern(int x, int y) {
    int p = x, q = y, r = 0, s = 0;
    volatile int mod = external_seed % 17;
    
    /* Loop with nested condition */
    for (int j = 0; j < 50; ++j) {
        /* Complex enough to avoid if-conversion */
        if ((j + mod) % 7 == 0) {
            /* Another goto pattern */
            if ((p ^ q) & 0xF) {
                goto alt_target;
            }
            r = p * q;
        } else {
            s = p + q;
        }
        continue;
        
    alt_target:
        /* Another safe delay slot candidate */
        p = q + (j & 0xFF);  /* Mask ensures no overflow issues */
        
        /* Follow-up computation */
        q = r ^ s;
        r = p * 3;
    }
    
    return p + q + r + s;
}

/* Third pattern: pointer arithmetic without dangerous dereferencing */
int safe_pointer_pattern(int *arr, int size) {
    int sum = 0;
    int temp1 = 0, temp2 = 0;
    
    if (size > 0 && arr) {
        for (int k = 0; k < size && k < 10; ++k) {
            /* Safe pointer arithmetic - no null deref */
            int *ptr = arr + k;
            
            /* Conditional jump pattern */
            if ((k + external_seed) % 11 == 0) {
                goto ptr_target;
            }
            
            temp1 = arr[k] & 0x7FFF;  /* Mask to ensure positive */
            continue;
            
        ptr_target:
            /* Safe delay slot candidate: register operation */
            temp2 = temp1 + k;
            
            /* Use result */
            sum += temp2;
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    
    /* Process with main pattern */
    result1 = process_values(argc, argv);
    
    /* Alternate pattern */
    result2 = alternate_pattern(argc, result1);
    
    /* Safe pointer pattern */
    int small_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result3 = safe_pointer_pattern(small_array, 10);
    
    /* Combine results to create observable output */
    int final_result = result1 ^ result2 ^ result3;
    
    /* Print only if verbose mode requested */
    if (argc > 1 && argv[1][0] == '-') {
        printf("Final: %d (from %d, %d, %d)\n", 
               final_result, result1, result2, result3);
    }
    
    return final_result & 0xFF;  /* Return non-zero but bounded value */
}
