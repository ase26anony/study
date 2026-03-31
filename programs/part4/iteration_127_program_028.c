/* doloop_coverage.c
 * Designed to trigger GCC's doloop_optimize pattern matching for (reg - 1) != 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_counter;

/* Function 1: Classic for loop with i-- and != 0 condition */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, exit when != 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        sum += i * 2;
        /* Volatile write to prevent dead code elimination */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 2: While loop with pre-decrement and explicit comparison */
NOOPT void loop_decrement_while_predec(int n, int *result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: --cnt != 0 */
    while (--cnt != 0) {
        sum += cnt * 3;
        g_volatile_sink = cnt;
    }
    *result = sum;
}

/* Function 3: While loop with post-decrement */
NOOPT void loop_decrement_while_postdec(int n, int *result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: cnt-- != 0 */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 5;
        g_volatile_sink = cnt;
    }
    *result = sum;
}

/* Function 4: Do-while with pre-decrement check */
NOOPT void loop_decrement_dowhile(int n, int *result) {
    int sum = 0;
    int cnt = n;
    if (cnt > 0) {
        do {
            sum += cnt * 7;
            g_volatile_sink = cnt;
        } while (--cnt != 0);
    }
    *result = sum;
}

/* Function 5: For loop with explicit decrement in body */
NOOPT void loop_decrement_for_explicit(int n, int *result) {
    int sum = 0;
    int i;
    /* Initialize counter from volatile to prevent constant propagation */
    i = g_volatile_counter;
    if (i > n) i = n;
    
    for (; i != 0; ) {
        sum += i * 11;
        g_volatile_sink = i;
        i--;  /* Explicit decrement */
    }
    *result = sum;
}

/* Function 6: Nested loops to create more complex scenario */
NOOPT void loop_decrement_nested(int n, int *result) {
    int sum = 0;
    int outer = n / 2;
    if (outer < 1) outer = 1;
    
    for (int i = outer; i != 0; i--) {
        int inner = n;
        /* Inner loop with decrementing counter */
        while (inner-- != 0) {
            sum += i * inner;
            g_volatile_sink = inner;
        }
    }
    *result = sum;
}

/* Function 7: Unsigned counter (might generate different RTL) */
NOOPT void loop_decrement_unsigned(unsigned int n, unsigned int *result) {
    unsigned int sum = 0;
    /* Unsigned decrement to zero */
    for (unsigned int i = n; i != 0; i--) {
        sum += i * 13;
        g_volatile_sink = (int)i;
    }
    *result = sum;
}

/* Function 8: Counter with arithmetic in condition */
NOOPT void loop_decrement_arithmetic(int n, int *result) {
    int sum = 0;
    int cnt = n;
    /* The condition (cnt - 1) != 0 might generate PLUS with -1 */
    while ((cnt - 1) != 0) {
        cnt--;
        sum += cnt * 17;
        g_volatile_sink = cnt;
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    int results[8] = {0};
    int total = 0;
    
    /* Make loop bound non-constant using command line or volatile */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile to prevent compile-time constant */
        g_volatile_counter = 100;
        loop_bound = g_volatile_counter;
    }
    
    /* Ensure bound is reasonable */
    if (loop_bound < 10) loop_bound = 100;
    if (loop_bound > 10000) loop_bound = 1000;
    
    printf("Testing doloop pattern with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_dowhile(loop_bound, &results[3]);
    loop_decrement_for_explicit(loop_bound, &results[4]);
    loop_decrement_nested(loop_bound, &results[5]);
    
    unsigned int uresult;
    loop_decrement_unsigned((unsigned int)loop_bound, &uresult);
    results[6] = (int)uresult;
    
    loop_decrement_arithmetic(loop_bound, &results[7]);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 8; i++) {
        total += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Total checksum = %d\n", total);
    
    /* Use result to affect program output */
    if (total != 0) {
        printf("All loops executed successfully.\n");
    }
    
    return total == 0 ? 1 : 0;
}
