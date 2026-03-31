/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS architecture: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int fill_delay_slot_pattern(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Main loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a non-trivial condition that can't be optimized away */
        int condition = (i * argc) % 7;
        
        /* KEY CONSTRUCT: Conditional jump with potential delay slot candidate */
        if (condition == 0) {
            /* Use goto to create a simple jump to label */
            goto target_label;
        }
        
        /* Some intermediate computations to create register pressure */
        e = f + g;
        f = g - h;
        g = h * e;
        h = e ^ f;
        
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* This should be the candidate for delay slot filling */
        /* Simple arithmetic that doesn't trap and uses independent registers */
        a = b + c;  /* next_trial candidate */
        
        /* Continue with other operations so target isn't isolated */
        d = c * a;
        b = d - 1;
    }
    
    /* Additional computations to use modified variables */
    int result = a + b + c + d + e + f + g + h;
    
    /* Create observable side effect */
    printf("Result checksum: %d\n", result);
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
void another_pattern(int x, int y, int z) {
    volatile int v1 = x;
    volatile int v2 = y;
    volatile int v3 = z;
    int temp;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Another conditional jump pattern */
            if ((i * j + x) % 11 == 0) {
                goto another_target;
            }
            
            v1 = v2 ^ v3;
            v2 = v3 << 2;
            v3 = v1 | v2;
            
            continue;
            
            another_target:
            /* Another candidate instruction - register move pattern */
            temp = v1 + v2;  /* Should be safe for delay slot */
            
            v3 = temp * 3;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Pattern2: %d %d %d\n", v1, v2, v3);
}

/* Third pattern focusing on pointer operations without trapping */
void safe_pointer_pattern(int *ptr1, int *ptr2, int n) {
    /* Use stack-allocated arrays to ensure valid pointers */
    int arr1[10] = {1,2,3,4,5,6,7,8,9,10};
    int arr2[10] = {10,9,8,7,6,5,4,3,2,1};
    
    volatile int sum = 0;
    
    for (int i = 0; i < n && i < 10; i++) {
        /* Conditional jump */
        if ((i + n) % 5 == 0) {
            goto ptr_target;
        }
        
        sum += arr1[i];
        
        continue;
        
        ptr_target:
        /* Safe memory store - using known valid addresses */
        arr2[i] = arr1[i] + 1;  /* Should not trap */
        
        sum += arr2[i];
    }
    
    printf("Sum: %d\n", sum);
}

int main(int argc, char **argv) {
    int result1 = fill_delay_slot_pattern(argc, argv);
    
    another_pattern(argc, result1, argc * 2);
    
    int local_var = 5;
    safe_pointer_pattern(&local_var, &local_var, argc);
    
    /* Return value based on all computations */
    return result1 % 256;
}
