#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 1000;
static volatile int g_volatile_sink;

/* Different loop variants to increase hit probability */

/* Variant 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Explicit != 0 comparison with decrement */
    for (int i = n; i != 0; i--) {
        sum += i * 2;  /* Side effect depending on counter */
        g_volatile_sink = sum;  /* Prevent dead code elimination */
    }
    *result = sum;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Pre-decrement with explicit != 0 comparison */
    while (--i != 0) {
        sum += (i + 1) * 3;
        g_volatile_sink = sum;
    }
    *result = sum;
}

/* Variant 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Post-decrement with explicit != 0 comparison */
    while (i-- != 0) {
        sum += (i + 1) * 5;
        g_volatile_sink = sum;
    }
    *result = sum;
}

/* Variant 4: do-while with pre-decrement check */
NOOPT void loop_decrement_dowhile(int n, int *result) {
    int sum = 0;
    int i = n;
    if (i > 0) {
        do {
            sum += i * 7;
            g_volatile_sink = sum;
        } while (--i != 0);  /* Explicit != 0 comparison */
    }
    *result = sum;
}

/* Variant 5: for loop with explicit decrement in body */
NOOPT void loop_decrement_for_explicit(int n, int *result) {
    int sum = 0;
    int i;
    /* Counter decremented in body with explicit != 0 check */
    for (i = n; i != 0; ) {
        sum += i * 11;
        g_volatile_sink = sum;
        i--;  /* Decrement in body */
    }
    *result = sum;
}

/* Variant 6: unsigned counter to avoid signed overflow issues */
NOOPT void loop_decrement_unsigned(unsigned int n, int *result) {
    int sum = 0;
    /* Unsigned counter with != 0 comparison */
    for (unsigned int i = n; i != 0; i--) {
        sum += (int)i * 13;
        g_volatile_sink = sum;
    }
    *result = sum;
}

/* Main function with non-constant loop bound */
int main(int argc, char *argv[]) {
    int results[6] = {0};
    int total = 0;
    
    /* Make loop bound non-constant to prevent unrolling */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);  /* From command line */
    } else {
        loop_bound = g_volatile_bound;  /* From volatile variable */
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 100;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_dowhile(loop_bound, &results[3]);
    loop_decrement_for_explicit(loop_bound, &results[4]);
    loop_decrement_unsigned((unsigned int)loop_bound, &results[5]);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 6; i++) {
        total += results[i];
        printf("Result %d: %d\n", i, results[i]);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile operation to ensure loops execute */
    g_volatile_sink = total;
    
    return total != 0 ? 0 : 1;
}
