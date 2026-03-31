/* Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with complex enough control flow */
int process_values(int argc, char **argv) {
    /* Declare and initialize variables - use different registers */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int divisor = (argc > 2) ? 7 : 11;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Mix in some arithmetic to create register pressure */
        int temp = a * b + c - d;
        
        /* KEY CONSTRUCT: Conditional jump with potential delay slot candidate */
        if ((i % divisor) == 0) {
            /* Use goto to create explicit jump to label */
            goto target_label;
        }
        
        /* Some other computations to prevent optimization */
        a = b + 1;
        b = c * 2;
        c = d - 1;
        d = temp % 13;
        
        /* Skip the target code when not jumping */
        continue;
        
        /* TARGET LABEL: Place simple, safe instruction here */
        target_label:
        /* This is the candidate for delay slot filling */
        /* Simple arithmetic, no trapping, no resource conflicts */
        a = b + c;  /* Should compile to simple add instruction */
        
        /* Continue with other operations after label */
        b = c * d;
        c = a + d;
        d = b - a;
    }
    
    /* Create observable side effects */
    result = a + b + c + d;
    
    /* Use global volatile to prevent dead code elimination */
    global_seed = result;
    
    return result;
}

/* Another function to create more complex control flow graph */
void helper_func(int *ptr, int n) {
    for (int i = 0; i < n; ++i) {
        /* Simple memory operation - safe if n > 0 */
        ptr[i] = i * 2;
        
        /* Another conditional jump opportunity */
        if ((i & 3) == 0) {
            /* Use local label */
            local_label:
            /* Another delay slot candidate */
            *ptr = *ptr + 1;
            continue;
        }
        
        /* Prevent optimization */
        if (ptr[i] > 100) {
            goto local_label;
        }
    }
}

int main(int argc, char **argv) {
    int array[100];
    int sum = 0;
    
    /* Call first function */
    sum = process_values(argc, argv);
    
    /* Call second function */
    helper_func(array, argc > 1 ? 50 : 25);
    
    /* Compute checksum for observable output */
    for (int i = 0; i < 50; ++i) {
        sum += array[i];
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return sum & 0xFF;
}
