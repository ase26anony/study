#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_counter;
volatile int g_volatile_result;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n) {
    volatile int local_sum = 0;
    /* for loop with explicit != 0 comparison */
    for (int i = n; i != 0; i--) {
        local_sum += i * 3;  /* Side effect depending on counter */
        g_volatile_result = local_sum; /* Ensure side effect is visible */
    }
    g_volatile_counter = local_sum;
}

NOOPT void loop_decrement_while_predec(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    /* while loop with pre-decrement and != 0 comparison */
    while (--cnt != 0) {
        local_sum += cnt * 7;
        g_volatile_result = local_sum;
    }
    g_volatile_counter = local_sum;
}

NOOPT void loop_decrement_while_postdec(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    /* while loop with post-decrement and != 0 comparison */
    while (cnt-- != 0) {
        local_sum += (cnt + 1) * 11;
        g_volatile_result = local_sum;
    }
    g_volatile_counter = local_sum;
}

NOOPT void loop_decrement_do_while(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    /* do-while with explicit check, ensuring at least one iteration */
    if (cnt > 0) {
        do {
            local_sum += cnt * 13;
            g_volatile_result = local_sum;
        } while (--cnt != 0);
    }
    g_volatile_counter = local_sum;
}

NOOPT void loop_decrement_for_unsigned(unsigned int n) {
    volatile unsigned int local_sum = 0;
    /* Using unsigned int to ensure register usage */
    for (unsigned int i = n; i != 0; i--) {
        local_sum += i * 17;
        g_volatile_result = local_sum;
    }
    g_volatile_counter = local_sum;
}

NOOPT void loop_decrement_complex_expr(int n) {
    volatile int local_sum = 0;
    /* More complex expression that should still match the pattern */
    int cnt = n;
    while ((cnt - 1) != -1) {  /* Equivalent to cnt != 0 */
        local_sum += cnt * 19;
        g_volatile_result = local_sum;
        cnt--;
    }
    g_volatile_counter = local_sum;
}

int main(int argc, char *argv[]) {
    /* Use non-constant loop bound to prevent unrolling */
    int loop_bound;
    
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent compile-time constant */
        volatile int seed = time(NULL);
        loop_bound = (seed % 1000) + 100;  /* 100-1099 iterations */
    }
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    int sum1 = g_volatile_counter;
    
    loop_decrement_while_predec(loop_bound);
    int sum2 = g_volatile_counter;
    
    loop_decrement_while_postdec(loop_bound);
    int sum3 = g_volatile_counter;
    
    loop_decrement_do_while(loop_bound);
    int sum4 = g_volatile_counter;
    
    loop_decrement_for_unsigned(loop_bound);
    int sum5 = g_volatile_counter;
    
    loop_decrement_complex_expr(loop_bound);
    int sum6 = g_volatile_counter;
    
    /* Compute checksum to prevent dead code elimination */
    int total_checksum = sum1 + sum2 + sum3 + sum4 + sum5 + sum6;
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
