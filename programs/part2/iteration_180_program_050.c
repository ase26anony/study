/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 0, b = 1, c = 2, d = 3;
    volatile int result = 0;
    int i, limit;
    
    /* Use argc to create runtime-dependent loop limit */
    limit = (argc > 1) ? 100 : 200;
    
    /* Main loop to provide scheduling context */
    for (i = 0; i < limit; ++i) {
        /* Create a condition that's not always true/false */
        if ((i % 7) == 0) {
            /* This should become a simple conditional jump */
            goto target_label;
        }
        
        /* Some computation to prevent the block from being empty */
        a = b + c;
        continue;
        
    target_label:
        /* Candidate instruction for delay slot filling */
        /* Simple, safe arithmetic that doesn't trap */
        a = b + c;  /* This should be the 'next_trial' instruction */
        
        /* Additional computation to ensure target isn't isolated */
        b = c * d;
        
        /* More operations to create register pressure */
        c = d ^ i;
        d = a + i;
    }
    
    /* Use results to prevent dead code elimination */
    result = a + b + c + d;
    
    /* Print to create observable side effect */
    printf("Result: %d\n", result);
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x, q = y, r = 0, s = 0;
    int j;
    
    for (j = 0; j < 50; ++j) {
        /* Different condition pattern */
        if ((p & 0xF) == (j & 0xF)) {
            goto alt_target;
        }
        
        p = q + j;
        continue;
        
    alt_target:
        /* Another candidate instruction - register-to-register operation */
        r = p + q;  /* Simple add instruction */
        
        /* Follow-up operations */
        s = r * p;
        q = s ^ j;
    }
    
    return p + q + r + s;
}

/* Third function with pointer operations (safe) */
int pointer_pattern(int *arr, int size) {
    volatile int sum = 0;
    int i;
    
    if (size <= 0) return 0;
    
    /* Ensure array access is safe */
    arr[0] = 1;
    
    for (i = 1; i < size; ++i) {
        /* Condition based on array values */
        if ((arr[i-1] & 1) == 0) {
            goto ptr_target;
        }
        
        arr[i] = arr[i-1] + i;
        continue;
        
    ptr_target:
        /* Safe memory store as candidate */
        arr[i] = arr[i-1] * 2;  /* Simple store operation */
        
        /* Additional computation */
        sum += arr[i];
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    int test_array[10];
    
    /* Call first test function */
    result1 = fill_delay_slot_test(argc, argv);
    
    /* Call second test function with runtime values */
    result2 = alternative_pattern(argc, result1);
    
    /* Call third test function */
    result3 = pointer_pattern(test_array, 10);
    
    /* Combine results to ensure all code is used */
    return (result1 + result2 + result3) & 0xFF;
}
