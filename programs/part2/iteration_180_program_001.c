/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 0, b = 1, c = 2, d = 3;
    volatile int result = 0;
    volatile int *ptr = &result;
    
    /* Use argc to create runtime-dependent loop bounds */
    int limit = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a condition that's not always true/false */
        if (i % 7 == 0) {
            /* This should become a simple conditional jump to label */
            goto target_label;
        }
        
        /* Some computation to use variables and prevent dead code elimination */
        a = b + c;
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        continue;
        
        /* Target label with a simple, safe instruction */
        target_label:
        /* This instruction should be eligible for delay slot filling:
           - Simple arithmetic operation
           - No resource conflicts with jump
           - Non-trapping
           - Not a jump itself */
        a = b + c;  /* Candidate for delay slot */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d - a;
    }
    
    /* Create observable side-effects using modified variables */
    result = a + b + c + d;
    
    /* Use the result to prevent dead code elimination */
    if (result > 1000) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second test case with different pattern */
int nested_delay_test(int x, int y) {
    volatile int a = x, b = y, c = 0, d = 0;
    volatile int temp;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 20; ++j) {
            /* Multiple conditional jumps */
            if ((i * j) % 11 == 0) {
                goto inner_label;
            }
            
            a = b + i;
            b = a - j;
            continue;
            
            inner_label:
            /* Another candidate instruction - register move pattern */
            c = a;  /* Simple move operation */
            
            /* More operations */
            d = b * 2;
            a = c + d;
        }
        
        /* Outer loop jump */
        if (i % 13 == 0) {
            goto outer_label;
        }
        
        temp = a * b;
        continue;
        
        outer_label:
        d = c;  /* Another simple move candidate */
        a = d + 1;
    }
    
    return a + b + c + d;
}

/* Third test with pointer operations (safe) */
int pointer_delay_test(int *arr, int size) {
    volatile int sum = 0;
    volatile int *ptr = arr;
    
    if (size <= 0) return 0;
    
    for (int i = 0; i < size; ++i) {
        /* Conditional jump based on array value */
        if (arr[i] % 5 == 0) {
            goto compute_label;
        }
        
        sum += arr[i];
        continue;
        
        compute_label:
        /* Safe memory store - non-trapping if arr is valid */
        arr[i] = sum;  /* Candidate: store operation */
        
        /* Additional computation */
        sum = sum * 2 + 1;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Run all test cases to increase coverage chances */
    result1 = fill_delay_slot_test(argc, argv);
    result2 = nested_delay_test(argc, 10);
    result3 = pointer_delay_test(test_array, 10);
    
    /* Combine results to create observable output */
    printf("Final results: %d, %d, %d\n", result1, result2, result3);
    
    return (result1 + result2 + result3) > 0 ? 0 : 1;
}
