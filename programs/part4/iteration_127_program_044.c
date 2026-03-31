/* doloop_coverage.c
 * Designed to trigger GCC's doloop optimization pattern matching
 * for (reg - 1) != 0 comparison pattern
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Classic for loop with i-- and != 0 condition */
NOOPT unsigned int loop_decrement_for(int iterations) {
    volatile unsigned int result = 0;
    unsigned int i;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = iterations; i != 0; i--) {
        result ^= (i * 137) + global_seed;
    }
    return result;
}

/* Function 2: While loop with pre-decrement and != 0 comparison */
NOOPT unsigned int loop_decrement_while(int iterations) {
    volatile unsigned int result = 0;
    unsigned int cnt = iterations;
    
    /* Pre-decrement in while condition */
    while (--cnt != 0) {
        result ^= (cnt * 7919) + global_seed;
    }
    /* Handle last iteration */
    result ^= global_seed;
    return result;
}

/* Function 3: While loop with post-decrement and != 0 comparison */
NOOPT unsigned int loop_decrement_while_post(int iterations) {
    volatile unsigned int result = 0;
    unsigned int cnt = iterations;
    
    /* Post-decrement in while condition */
    while (cnt-- != 0) {
        result ^= (cnt * 65537) + global_seed;
    }
    return result;
}

/* Function 4: Do-while with decrement and != 0 check */
NOOPT unsigned int loop_decrement_dowhile(int iterations) {
    volatile unsigned int result = 0;
    unsigned int cnt = iterations;
    
    if (cnt == 0) return 0;
    
    do {
        result ^= (cnt * 257) + global_seed;
    } while (--cnt != 0);
    
    return result;
}

/* Function 5: For loop with compound decrement in condition */
NOOPT unsigned int loop_decrement_for_compound(int iterations) {
    volatile unsigned int result = 0;
    unsigned int i = iterations;
    
    for (; i != 0; ) {
        result ^= (i * 1009) + global_seed;
        i--;
    }
    return result;
}

/* Function 6: Nested loops to create more complex pattern */
NOOPT unsigned int loop_nested_decrement(int outer_iter, int inner_iter) {
    volatile unsigned int result = 0;
    unsigned int i, j;
    
    for (i = outer_iter; i != 0; i--) {
        for (j = inner_iter; j != 0; j--) {
            result ^= (i * j * 31337) + global_seed;
        }
    }
    return result;
}

/* Function 7: Loop with if condition inside, but still decrement counter */
NOOPT unsigned int loop_decrement_with_if(int iterations) {
    volatile unsigned int result = 0;
    unsigned int cnt = iterations;
    
    while (cnt != 0) {
        if ((cnt & 1) == 0) {
            result ^= (cnt * 8191) + global_seed;
        } else {
            result ^= (cnt * 127) + global_seed;
        }
        cnt--;
    }
    return result;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    unsigned int final_result = 0;
    int base_iterations;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 1000;
    } else {
        /* Use volatile read to prevent constant propagation */
        base_iterations = global_seed * 23 + 1000;
    }
    
    printf("Testing doloop optimization patterns with %d iterations\n", base_iterations);
    
    /* Call all loop variants to increase coverage probability */
    final_result ^= loop_decrement_for(base_iterations);
    final_result ^= loop_decrement_while(base_iterations);
    final_result ^= loop_decrement_while_post(base_iterations);
    final_result ^= loop_decrement_dowhile(base_iterations);
    final_result ^= loop_decrement_for_compound(base_iterations);
    final_result ^= loop_nested_decrement(base_iterations / 10, 10);
    final_result ^= loop_decrement_with_if(base_iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: 0x%08x\n", final_result);
    
    /* Additional volatile store to ensure side effects */
    volatile unsigned int sink = final_result;
    (void)sink;
    
    return 0;
}
