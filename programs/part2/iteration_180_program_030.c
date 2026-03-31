/* Program to trigger delay slot filling in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding and preserve control flow */
static volatile int external_seed = 0;

int main(int argc, char *argv[]) {
    /* Initialize variables - use different registers to avoid conflicts */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    external_seed = argc;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional jump that's not trivially predictable */
        if ((i + external_seed) % 7 == 0) {
            /* This goto creates a simplejump_p to target_label */
            goto delay_slot_target;
        }
        
        /* Some computation to use variables and prevent dead code elimination */
        a = b + c;
        b = c ^ d;
        c = d - a;
        d = a + i;
        
        /* Skip the target code when not jumping */
        continue;
        
delay_slot_target:
        /* This is the candidate instruction for delay slot filling */
        /* Simple, non-trapping, resource-independent operation */
        a = b + c;  /* Should compile to simple add instruction */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d + 1;
    }
    
    /* Use the variables to create observable side effects */
    result = a + b + c + d;
    
    /* Mix with another conditional to preserve jump structure */
    if (result % 2 == 0) {
        /* Another potential delay slot candidate */
        volatile int temp = a * b;
        result += temp;
    } else {
        volatile int temp = c * d;
        result -= temp;
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
