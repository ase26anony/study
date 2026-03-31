/* Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with complex enough control flow */
int process_values(int argc, char **argv) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int mod_base = (argc > 2) ? 7 : 11;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        d = temp - a;
        
        /* KEY CONSTRUCT: Conditional jump with potential delay slot candidate */
        /* The condition is runtime-dependent to prevent optimization */
        if ((i % mod_base) == (global_seed & 3)) {
            /* Jump to label where target instruction is safe for delay slot */
            goto target_label;
        }
        
        /* Alternative path */
        a = b + d;
        b = c ^ i;
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* Candidate for delay slot: simple arithmetic, no traps */
        /* Uses different registers than jump condition (i, mod_base) */
        a = b + c;  /* This should be eligible for delay slot filling */
        
        /* Continue with other operations */
        c = d * 2;
        b = a ^ 0xFF;
    }
    
    /* Additional operations to use variables and prevent dead code elimination */
    result = a + b + c + d;
    
    /* Create observable side effect */
    if (argc > 1) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Another function to increase compilation complexity */
void helper_func(int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        /* Simple operations that might create scheduling opportunities */
        arr[i] = (arr[i] * 3) + 1;
        
        /* Another conditional jump pattern */
        if ((i & 1) && (arr[i] > 100)) {
            /* Small block with simple instruction at target */
            arr[i] = arr[i] - 50;
        }
    }
}

int main(int argc, char **argv) {
    int array[50];
    
    /* Initialize array with varying values */
    for (int i = 0; i < 50; ++i) {
        array[i] = i * (argc + 1);
    }
    
    /* Call helper to create more instruction patterns */
    helper_func(array, 50);
    
    /* Main processing with the key construct */
    int result = process_values(argc, argv);
    
    /* Use result to prevent optimization */
    if (result > 1000) {
        printf("Large result: %d\n", result);
    }
    
    return result & 0xFF;
}
