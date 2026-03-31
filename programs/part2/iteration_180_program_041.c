/* Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 0;

/* Function with the critical pattern */
int process_values(int argc, char **argv) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int mod_base = (argc > 2) ? 7 : 11;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create runtime-dependent condition */
        int condition = (i % mod_base) == (global_seed % 3);
        
        /* Critical pattern: conditional jump to label */
        if (condition) {
            /* Simple conditional jump that should become simplejump_p */
            goto target_label;
        }
        
        /* Some computations to use variables */
        a = b + c;
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* This instruction should be eligible for delay slot filling */
        /* It doesn't use condition codes, stack pointer, or conflict with jump */
        a = b + c;  /* Simple arithmetic, no trapping */
        
        /* Continue with other operations */
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        /* Prevent tail merging of blocks */
        result += (a + b + c + d);
    }
    
    /* Additional code to prevent dead code elimination */
    for (int i = 0; i < 10; ++i) {
        a = (a * b + c) % 256;  /* Mod with small divisor to avoid trapping */
        b = (b ^ c) | d;
        c = c + (d << 2);
        d = d - (a >> 1);
    }
    
    /* Create observable side effect */
    return result + a + b + c + d;
}

/* Another function to increase register pressure */
void helper_func(int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        arr[i] = (arr[i] * 3 + 7) & 0xFF;
    }
}

int main(int argc, char **argv) {
    int array[50];
    
    /* Initialize array with values */
    for (int i = 0; i < 50; ++i) {
        array[i] = i * 3 + (argc % 5);
    }
    
    /* Call helper to create more complex control flow */
    helper_func(array, 50);
    
    /* Process values with the critical pattern */
    int result = process_values(argc, argv);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Also use array to prevent dead store elimination */
    int sum = 0;
    for (int i = 0; i < 50; ++i) {
        sum += array[i];
    }
    printf("Array sum: %d\n", sum);
    
    return (result + sum) > 1000 ? 0 : 1;
}
