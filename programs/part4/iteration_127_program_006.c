#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
volatile int global_seed = 1000;

/* Function prototypes */
NOINLINE void loop_decrement_for(int n, volatile int *result);
NOINLINE void loop_decrement_while_predec(int n, volatile int *result);
NOINLINE void loop_decrement_while_postdec(int n, volatile int *result);
NOINLINE void loop_decrement_do_while(int n, volatile int *result);
NOINLINE void loop_decrement_for_unsigned(unsigned int n, volatile int *result);

/* Variant 1: for loop with i != 0 condition */
NOINLINE void loop_decrement_for(int n, volatile int *result) {
    int i;
    int local_sum = 0;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter to prevent dead code elimination */
        local_sum += (i * 3) & 0xFF;
    }
    
    *result = local_sum;
}

/* Variant 2: while loop with --i != 0 */
NOINLINE void loop_decrement_while_predec(int n, volatile int *result) {
    int i = n;
    int local_sum = 0;
    
    while (--i != 0) {
        /* Different side effect pattern */
        local_sum ^= (i * 7);
    }
    
    *result = local_sum;
}

/* Variant 3: while loop with i-- != 0 */
NOINLINE void loop_decrement_while_postdec(int n, volatile int *result) {
    int i = n;
    int local_sum = 0;
    
    while (i-- != 0) {
        /* Another side effect pattern */
        local_sum |= (i * 11);
    }
    
    *result = local_sum;
}

/* Variant 4: do-while loop with explicit check */
NOINLINE void loop_decrement_do_while(int n, volatile int *result) {
    int i = n;
    int local_sum = 0;
    
    if (i > 0) {
        do {
            local_sum += (i * 13) % 256;
        } while (--i != 0);
    }
    
    *result = local_sum;
}

/* Variant 5: unsigned counter to ensure no overflow issues */
NOINLINE void loop_decrement_for_unsigned(unsigned int n, volatile int *result) {
    unsigned int i;
    int local_sum = 0;
    
    for (i = n; i != 0; i--) {
        local_sum += (int)(i * 17) % 100;
    }
    
    *result = local_sum;
}

/* Variant 6: Complex expression in loop condition */
NOINLINE void loop_decrement_complex(int n, volatile int *result) {
    int i = n;
    int local_sum = 0;
    
    while ((i - 1) != -1) {  /* Equivalent to i != 0 */
        local_sum += i * 19;
        i--;
    }
    
    *result = local_sum;
}

/* Variant 7: Counter in register with arithmetic */
NOINLINE void loop_decrement_register(int n, volatile int *result) {
    register int i asm ("r12") = n;  /* Hint to use register */
    int local_sum = 0;
    
    while (i != 0) {
        local_sum += (i * 23) >> 2;
        i--;
    }
    
    *result = local_sum;
}

int main(int argc, char *argv[]) {
    volatile int results[7] = {0};
    int total = 0;
    
    /* Make loop bound non-constant at compile time */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        loop_bound = global_seed;
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_do_while(loop_bound, &results[3]);
    loop_decrement_for_unsigned((unsigned int)loop_bound, &results[4]);
    loop_decrement_complex(loop_bound, &results[5]);
    loop_decrement_register(loop_bound, &results[6]);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    printf("Checksum: %d\n", total);
    
    /* Additional test with different bounds */
    for (int test = 1; test <= 3; test++) {
        int bound = loop_bound / (test * 2) + 1;
        volatile int temp_result;
        
        loop_decrement_for(bound, &temp_result);
        total += temp_result;
        
        loop_decrement_while_postdec(bound, &temp_result);
        total += temp_result;
    }
    
    printf("Final checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
