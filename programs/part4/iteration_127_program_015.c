#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
volatile int global_seed = 1000;

/* Function prototypes */
NOINLINE void loop_decrement_for(int n, volatile int *result);
NOINLINE void loop_decrement_while_pre(int n, volatile int *result);
NOINLINE void loop_decrement_while_post(int n, volatile int *result);
NOINLINE void loop_decrement_do_while(int n, volatile int *result);
NOINLINE void loop_decrement_for_unsigned(unsigned int n, volatile int *result);

/* Variant 1: for loop with i != 0 condition */
NOINLINE void loop_decrement_for(int n, volatile int *result) {
    int i;
    volatile int local_sum = 0;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        local_sum += i * 3;
        /* Additional side effect to prevent optimization */
        *result = local_sum;
    }
    
    *result = local_sum;
}

/* Variant 2: while loop with --i != 0 */
NOINLINE void loop_decrement_while_pre(int n, volatile int *result) {
    int cnt = n;
    volatile int local_sum = 0;
    
    while (--cnt != 0) {
        /* Different side effect pattern */
        local_sum ^= (cnt * 7);
        *result = local_sum;
    }
    
    *result = local_sum;
}

/* Variant 3: while loop with cnt-- != 0 */
NOINLINE void loop_decrement_while_post(int n, volatile int *result) {
    int cnt = n;
    volatile int local_sum = 0;
    
    while (cnt-- != 0) {
        /* Side effect with modulo to vary pattern */
        local_sum += (cnt % 13);
        *result = local_sum;
    }
    
    *result = local_sum;
}

/* Variant 4: do-while with pre-check */
NOINLINE void loop_decrement_do_while(int n, volatile int *result) {
    int i = n;
    volatile int local_sum = 0;
    
    if (i != 0) {
        do {
            local_sum += i * 11;
            *result = local_sum;
        } while (--i != 0);
    }
    
    *result = local_sum;
}

/* Variant 5: unsigned counter */
NOINLINE void loop_decrement_for_unsigned(unsigned int n, volatile int *result) {
    unsigned int i;
    volatile int local_sum = 0;
    
    for (i = n; i != 0; i--) {
        local_sum += (int)i * 5;
        *result = local_sum;
    }
    
    *result = local_sum;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    volatile int results[5] = {0};
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        loop_bound = global_seed % 100 + 50;
    }
    
    printf("Using loop bound: %d\n", loop_bound);
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_pre(loop_bound, &results[1]);
    loop_decrement_while_post(loop_bound, &results[2]);
    loop_decrement_do_while(loop_bound, &results[3]);
    loop_decrement_for_unsigned((unsigned int)loop_bound, &results[4]);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
