#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_source = 1000;

/* Function 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, compare with 0 */
    for (int i = n; i != 0; i--) {
        sum += i * 3;  /* Side effect depending on counter */
        g_volatile_sink = i;  /* Prevent dead code elimination */
    }
    *result = sum;
}

/* Function 2: while loop with --i != 0 */
NOOPT void loop_decrement_while(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Pre-decrement pattern */
    while (--i != 0) {
        sum += (i + 1) * 7;
        g_volatile_sink = i;
    }
    /* Handle the last iteration */
    sum += 7;
    *result = sum;
}

/* Function 3: do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int n, int *result) {
    int sum = 0;
    int i = n;
    if (i != 0) {
        do {
            sum += i * 11;
            g_volatile_sink = i;
        } while (--i != 0);  /* Decrement and compare with 0 */
    }
    *result = sum;
}

/* Function 4: while loop with post-decrement */
NOOPT void loop_decrement_post(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Post-decrement pattern */
    while (i-- != 0) {
        sum += (i + 1) * 13;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 5: Complex counter but still decrement by 1 */
NOOPT void loop_decrement_complex(int n, int *result) {
    int sum = 0;
    /* Counter modified in loop body but still decrements by 1 */
    for (int i = n; i != 0; ) {
        sum += i * 17;
        g_volatile_sink = i;
        i--;  /* Separate decrement */
    }
    *result = sum;
}

/* Function 6: Unsigned counter (might generate different RTL) */
NOOPT void loop_decrement_unsigned(unsigned int n, int *result) {
    int sum = 0;
    for (unsigned int i = n; i != 0; i--) {
        sum += (int)i * 19;
        g_volatile_sink = (int)i;
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    int results[6] = {0};
    int total = 0;
    
    /* Make loop bound non-constant at compile time */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile to prevent constant propagation */
        loop_bound = g_volatile_source;
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 100;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while(loop_bound, &results[1]);
    loop_decrement_dowhile(loop_bound, &results[2]);
    loop_decrement_post(loop_bound, &results[3]);
    loop_decrement_complex(loop_bound, &results[4]);
    loop_decrement_unsigned((unsigned int)loop_bound, &results[5]);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 6; i++) {
        total += results[i];
        printf("Result %d: %d\n", i, results[i]);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero total\n");
    }
    
    return 0;
}
