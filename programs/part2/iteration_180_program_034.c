/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slots_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent constant folding */
    volatile int a = 0;
    volatile int b = argc * 3;
    volatile int c = argc * 5;
    volatile int d = argc * 7;
    volatile int e = 0;
    volatile int f = 0;
    
    /* Use command-line arguments to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a conditional check based on runtime value */
        /* The condition should not be always true/false to prevent optimization */
        if ((i + argc) % 7 == 0) {
            /* This is the critical simple conditional jump */
            /* The compiler should generate a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Some other code to prevent the jump from being optimized away */
        e = b + c;
        f = c - d;
        a = e * f;
        
        /* Skip the target code when not jumping */
        continue;
        
    target_label:
        /* This is the candidate instruction for delay slot filling (next_trial) */
        /* Simple arithmetic that doesn't trap and uses independent resources */
        a = b + c;  /* Should compile to simple register operation */
        
        /* Additional operations to ensure target isn't isolated */
        d = c * 2;
        e = d - b;
        
        /* Continue loop */
    }
    
    /* Additional computations to create observable side-effects */
    int result = a + b + c + d + e + f;
    
    /* Use the result to prevent dead code elimination */
    if (result > 1000) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second test case with different pattern */
int alternative_test(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 0;
    volatile int s = 0;
    
    /* Nested loop for more complex control flow */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 20; ++j) {
            /* Conditional jump with fillable delay slot candidate */
            if ((i * j + x) % 11 == 0) {
                goto alt_target;
            }
            
            r = p * q;
            s = r + i;
            continue;
            
        alt_target:
            /* Another safe candidate instruction */
            p = q + 1;  /* Simple increment, no trapping */
            
            /* More operations */
            q = p * 2;
            r = q - 1;
        }
    }
    
    return p + q + r + s;
}

/* Third test with pointer operations (safe ones) */
int pointer_test(int *ptr1, int *ptr2, int n) {
    volatile int sum = 0;
    volatile int temp = 0;
    
    if (ptr1 && ptr2 && n > 0) {
        for (int i = 0; i < n; ++i) {
            /* Conditional jump */
            if ((i + *ptr1) % 13 == 0) {
                goto ptr_target;
            }
            
            temp = ptr1[i] + ptr2[i];
            sum += temp;
            continue;
            
        ptr_target:
            /* Safe memory operation - array bounds already checked */
            temp = ptr1[0] + 5;  /* Always valid if ptr1 is valid */
            
            /* Additional operations */
            sum += temp;
            ptr2[0] = temp;
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1 = fill_delay_slots_test(argc, argv);
    
    /* Create some test data for other functions */
    int x = argc * 2;
    int y = argc * 3;
    int result2 = alternative_test(x, y);
    
    /* Test with arrays for pointer test */
    int arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) {
        arr1[i] = i * argc;
        arr2[i] = i * argc + 1;
    }
    int result3 = pointer_test(arr1, arr2, 10);
    
    /* Combine results to ensure all code is used */
    int final_result = result1 + result2 + result3;
    
    printf("Final: %d\n", final_result);
    return final_result % 256;
}
