#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural optimization */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_source = 1000;

/* Function 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, compare with != 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        sum += i * 2;
        /* Additional side effect to volatile */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 2: while loop with pre-decrement */
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

/* Function 3: while loop with post-decrement */
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

/* Function 4: do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int n, int *result) {
    int sum = 0;
    int cnt = n;
    if (cnt > 0) {
        do {
            sum += cnt * 7;
            g_volatile_sink = cnt;
        } while (--cnt != 0);  /* Explicit --cnt != 0 comparison */
    }
    *result = sum;
}

/* Function 5: for loop with complex counter but same pattern */
NOOPT void loop_decrement_complex(int n, int *result) {
    int sum = 0;
    /* Start from n, decrement by 1 each iteration */
    for (int i = n; i != 0; i = i - 1) {
        /* Explicit i - 1 pattern in RTL */
        sum += i * 11;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 6: unsigned counter (might generate different but valid pattern) */
NOOPT void loop_decrement_unsigned(unsigned int n, int *result) {
    unsigned int sum = 0;
    for (unsigned int i = n; i != 0; i--) {
        sum += i * 13;
        g_volatile_sink = (int)i;
    }
    *result = (int)sum;
}

int main(int argc, char *argv[]) {
    int results[6] = {0};
    int total = 0;
    
    /* Make loop bound non-constant using volatile or command line */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile to prevent compile-time constant */
        loop_bound = g_volatile_source;
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_dowhile(loop_bound, &results[3]);
    loop_decrement_complex(loop_bound, &results[4]);
    loop_decrement_unsigned((unsigned int)loop_bound, &results[5]);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        total += results[i];
        printf("Loop %d result: %d\n", i, results[i]);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use the volatile sink to prevent optimization */
    printf("Volatile sink value: %d\n", g_volatile_sink);
    
    return total != 0 ? 0 : 1;
}
