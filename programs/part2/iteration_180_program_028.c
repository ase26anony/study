/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int process_values(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int e = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Create a non-trivial control flow graph with a loop */
    for (int i = 0; i < loop_limit; ++i) {
        /* Create a conditional check that's not always true/false */
        if (i % 7 == 0) {
            /* This should compile to a simple conditional jump */
            goto target_label;
        }
        
        /* Some computation to keep variables alive */
        e = a + b;
        a = b + 1;
        continue;
        
        /* Target label with a simple, safe instruction */
        target_label:
        /* Simple arithmetic that doesn't trap and uses different variables */
        d = b + c;  /* Candidate for delay slot filling */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d - a;
    }
    
    /* Create observable side effects to prevent dead code elimination */
    int result = a + b + c + d + e;
    
    /* Use result to affect return value */
    return (result % 256);
}

/* Another function to increase scheduling complexity */
void helper_func(int *x, int *y, int n) {
    for (int i = 0; i < n; i++) {
        /* Simple operations that create register pressure */
        x[i] = y[i] * 2;
        y[i] = x[i] + i;
    }
}

int main(int argc, char **argv) {
    int array1[50], array2[50];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 50; i++) {
        array1[i] = i + 1;
        array2[i] = i * 2;
    }
    
    /* Call helper to create more instruction scheduling context */
    helper_func(array1, array2, 50);
    
    /* Process values with the key construct */
    int result = process_values(argc, argv);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Also use arrays to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        sum += array1[i] + array2[i];
    }
    printf("Array sum: %d\n", sum % 1000);
    
    return result;
}
