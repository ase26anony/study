/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and complex control flow */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize several integer variables */
    volatile int a = 1;  /* volatile to prevent optimization */
    int b = 2;
    int c = 3;
    int d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Complex loop to provide scheduling context */
    for (int i = 0; i < loop_limit; ++i) {
        /* Create runtime-dependent condition to prevent constant folding */
        int condition = i % 7;
        
        /* KEY CONSTRUCT: Conditional jump to label with potential delay slot candidate */
        if (condition == 0) {
            /* Use goto to create a simplejump_p pattern */
            goto target_label;
        }
        
        /* Some intermediate computations to create register pressure */
        b = c * d;
        c = b - a;
        d = a + i;
        
        /* Skip the target instruction when not jumping */
        continue;
        
        /* TARGET LABEL: Place simple, safe instruction here */
        /* This should compile to a non-jump, non-trapping instruction */
        /* that doesn't conflict with jump resources */
        target_label:
        a = b + c;  /* Simple arithmetic - safe, no trapping */
        
        /* Continue with other operations after the label */
        b = c * d;
        d = a + 1;
    }
    
    /* Additional computations to create observable side-effects */
    result = a + b + c + d;
    
    /* Use result to prevent dead code elimination */
    if (argc > 2) {
        result += atoi(argv[2]);
    }
    
    return result;
}

/* Second test case with different pattern */
int alternative_test(int x, int y) {
    volatile int p = x;
    int q = y;
    int r = 3;
    int s = 4;
    
    /* Loop with multiple branches */
    for (int i = 0; i < 50; i++) {
        /* Multiple conditions to create different jump paths */
        if ((i & 1) == 0) {
            if (p > q) {
                goto alt_target;
            }
            q = p + r;
        } else {
            if (q < r) {
                goto alt_target;
            }
            p = q - s;
        }
        
        /* Some intermediate computation */
        r = s * i;
        continue;
        
        alt_target:
        s = p + q;  /* Another simple, safe candidate for delay slot */
        
        /* More computations after target */
        p = q * 2;
        r = s - 1;
    }
    
    return p + q + r + s;
}

/* Third test: Pointer arithmetic without dangerous dereferencing */
int pointer_safe_test(int *arr, int n) {
    int sum = 0;
    volatile int idx = 0;
    
    if (n <= 0) return 0;
    
    for (int i = 0; i < n; i++) {
        /* Conditional jump based on array value */
        if (arr[i] > 100) {
            goto ptr_target;
        }
        
        sum += arr[i];
        idx = i * 2;
        continue;
        
        ptr_target:
        idx = i + 1;  /* Safe: no pointer dereference, no trapping */
        
        /* Additional safe computation */
        sum -= arr[i];
    }
    
    return sum + idx;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    int test_array[10] = {50, 150, 30, 200, 40, 60, 170, 80, 90, 110};
    
    /* Call first test */
    result1 = fill_delay_slot_test(argc, argv);
    
    /* Call second test with runtime values */
    result2 = alternative_test(argc, result1);
    
    /* Call third test with safe pointer access */
    result3 = pointer_safe_test(test_array, 10);
    
    /* Print results to create observable output */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Return combined result to prevent optimization */
    return (result1 + result2 + result3) % 256;
}
