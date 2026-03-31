/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int limit = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional jump that's not always true/false */
        /* Use modulo with a prime to prevent optimization */
        if ((i % 7) == 0) {
            /* This goto creates a simplejump_p to a label */
            goto target_label;
        }
        
        /* Some other computation to prevent the block from being empty */
        d = a + b;
        continue;
        
    target_label:
        /* Candidate instruction for delay slot filling */
        /* Simple, safe arithmetic that doesn't trap */
        a = b + c;  /* This should be the 'next_trial' instruction */
        
        /* Additional operation to ensure target isn't isolated */
        b = c * d;
        
        /* More operations to create register pressure */
        c = d - a;
        d = a ^ b;
    }
    
    /* Create observable side-effects to prevent dead code elimination */
    result = a + b + c + d;
    
    /* Use the result to prevent optimization */
    if (argc > 2) {
        result += atoi(argv[2]);
    }
    
    return result;
}

/* Second test case with different pattern */
int alternative_test(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 5;
    volatile int s = 6;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Conditional jump based on runtime values */
            if ((p + q) > (r * s)) {
                goto alt_target;
            }
            
            /* Alternative computation path */
            r = p - q;
            continue;
            
        alt_target:
            /* Another candidate for delay slot */
            /* Simple bitwise operation - no trapping */
            p = q & 0xFF;
            
            /* Follow-up operations */
            q = r | s;
            s = p << 2;
        }
        
        /* Modify values to change conditions */
        p += i;
        q -= j;
    }
    
    return p + q + r + s;
}

/* Third test with pointer operations (safe) */
int pointer_test(int *arr, int size) {
    volatile int sum = 0;
    volatile int temp = 0;
    int *safe_ptr = arr;
    
    if (size <= 0) return 0;
    
    /* Ensure pointer is valid */
    if (safe_ptr == NULL) return 0;
    
    for (int i = 0; i < size; i++) {
        /* Conditional jump */
        if ((i % 3) == 0) {
            goto ptr_target;
        }
        
        /* Safe array access */
        temp = safe_ptr[i % size];
        continue;
        
    ptr_target:
        /* Safe assignment - no pointer dereference in candidate */
        sum = temp + i;
        
        /* Safe memory operation after the label */
        safe_ptr[i % size] = sum;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Run all test cases to increase coverage chances */
    result1 = fill_delay_slot_test(argc, argv);
    result2 = alternative_test(argc, result1);
    result3 = pointer_test(test_array, 10);
    
    /* Print results to create observable behavior */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Return non-constant value to prevent optimization */
    return (result1 + result2 + result3) % 256;
}
