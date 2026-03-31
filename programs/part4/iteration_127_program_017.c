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

/* Variant 1: for loop with i-- and != 0 condition */
NOINLINE void loop_decrement_for(int n, volatile int *result) {
    int i;
    volatile int local_sum = 0;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter to prevent dead code elimination */
        local_sum += (i * 3) & 0xFF;
    }
    
    *result = local_sum;
}

/* Variant 2: while loop with --i != 0 */
NOINLINE void loop_decrement_while_pre(int n, volatile int *result) {
    int i = n;
    volatile int local_sum = 0;
    
    /* Pre-decrement with explicit != 0 comparison */
    while (--i != 0) {
        local_sum += (i * 5) & 0xFF;
    }
    
    *result = local_sum;
}

/* Variant 3: while loop with i-- != 0 */
NOINLINE void loop_decrement_while_post(int n, volatile int *result) {
    int i = n;
    volatile int local_sum = 0;
    
    /* Post-decrement with explicit != 0 comparison */
    while (i-- != 0) {
        local_sum += (i * 7) & 0xFF;
    }
    
    *result = local_sum;
}

/* Variant 4: do-while loop with counter check */
NOINLINE void loop_decrement_do_while(int n, volatile int *result) {
    int i = n;
    volatile int local_sum = 0;
    
    if (i > 0) {
        do {
            local_sum += (i * 11) & 0xFF;
        } while (--i != 0);  /* Explicit != 0 comparison */
    }
    
    *result = local_sum;
}

/* Variant 5: unsigned counter to ensure register usage */
NOINLINE void loop_decrement_for_unsigned(unsigned int n, volatile int *result) {
    unsigned int i;
    volatile int local_sum = 0;
    
    /* Unsigned decrementing counter */
    for (i = n; i != 0; i--) {
        local_sum += (i * 13) & 0xFF;
    }
    
    *result = local_sum;
}

/* Variant 6: Complex expression in loop condition */
NOINLINE void loop_decrement_complex(int n, volatile int *result) {
    int i = n;
    volatile int local_sum = 0;
    
    /* The comparison should still match (reg - 1) != 0 pattern */
    while ((i - 1) != -1) {  /* Equivalent to i != 0 */
        i--;
        local_sum += (i * 17) & 0xFF;
    }
    
    *result = local_sum;
}

int main(int argc, char *argv[]) {
    volatile int results[6] = {0};
    int loop_count;
    
    /* Make loop count non-constant to prevent unrolling */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        loop_count = global_seed;
    }
    
    /* Ensure loop count is reasonable */
    if (loop_count <= 0) {
        loop_count = 100;
    }
    
    printf("Testing doloop optimization with count = %d\n", loop_count);
    
    /* Execute all loop variants */
    loop_decrement_for(loop_count, &results[0]);
    loop_decrement_while_pre(loop_count, &results[1]);
    loop_decrement_while_post(loop_count, &results[2]);
    loop_decrement_do_while(loop_count, &results[3]);
    loop_decrement_for_unsigned((unsigned int)loop_count, &results[4]);
    loop_decrement_complex(loop_count, &results[5]);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum = %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
