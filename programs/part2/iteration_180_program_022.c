/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create the specific pattern needed for delay slot filling */
int target_function(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int e = 0;
    int f = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int divisor = (argc > 2) ? 7 : 11;  /* Prevent constant folding */
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional check based on runtime value */
        /* The condition should not be always true/false */
        if ((i % divisor) == 0) {
            /* This is the critical simple conditional jump */
            /* It should jump to a label where the next instruction is safe to move */
            goto delay_slot_candidate;
            
            /* Code here won't be reached when the jump is taken */
            e = a + b;  /* Some computation to prevent empty block */
        } else {
            /* Alternate path to ensure the jump isn't always taken */
            f = c - d;
        }
        
        /* Continue with other operations */
        a = b + 1;
        b = c * 2;
        c = d - 1;
        d = a + b;
        
        /* Skip the label code when not jumping */
        continue;
        
delay_slot_candidate:
        /* CRITICAL: This instruction should be eligible for delay slot filling */
        /* It must be a simple, non-trapping, non-jump instruction */
        /* It should not conflict with resources used by the jump */
        e = b + c;  /* Simple arithmetic - safe, no trapping */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        f = d * 2;
        a = e + f;
    }
    
    /* Create observable side-effects to prevent dead code elimination */
    int result = a + b + c + d + e + f;
    
    /* Use the result to prevent optimization */
    if (argc > 3) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternate_pattern(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    int r = 0;
    
    /* Different loop structure */
    for (int j = 0; j < 50; ++j) {
        /* Another conditional jump pattern */
        if ((p + j) & 1) {
            goto alt_target;
        }
        
        p = q + j;
        q = p * 2;
        continue;
        
alt_target:
        /* Another candidate for delay slot */
        r = p + q;
        
        /* More operations */
        p = r - j;
        q = p >> 1;
    }
    
    return p + q + r;
}

/* Third function with pointer operations (carefully crafted to avoid trapping) */
int safe_pointer_ops(int *ptr1, int *ptr2, int n) {
    /* Ensure pointers are not NULL to avoid trapping */
    if (!ptr1 || !ptr2) return -1;
    
    int sum = 0;
    volatile int temp = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Conditional jump */
        if ((i & 3) == 0) {
            goto ptr_target;
        }
        
        ptr1[i] = i;
        continue;
        
ptr_target:
        /* Safe memory operation - using known valid pointer */
        temp = ptr2[i];
        
        /* Additional operation */
        sum += temp;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1 = target_function(argc, argv);
    
    /* Call alternate patterns */
    int result2 = alternate_pattern(argc, result1);
    
    /* Create arrays for pointer operations */
    int arr1[100], arr2[100];
    for (int i = 0; i < 100; ++i) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    int result3 = safe_pointer_ops(arr1, arr2, 
                                  (argc > 1) ? 50 : 25);
    
    /* Combine results to create final output */
    int final_result = result1 ^ result2 ^ result3;
    
    printf("Final: %d\n", final_result);
    return final_result % 256;
}
