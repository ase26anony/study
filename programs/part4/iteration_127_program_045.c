#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_counter;
static volatile int g_volatile_sink;

/* Function to get non-constant loop bound */
static int get_loop_bound(void) {
    return g_volatile_counter ? g_volatile_counter : 1000;
}

/* Variant 1: for loop with i-- and explicit != 0 comparison */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Critical: counter decrements by 1, comparison is != 0 */
    for (int i = n; i != 0; i--) {
        sum += i * 2;  /* Side effect depending on counter */
        g_volatile_sink = i;  /* Additional volatile side effect */
    }
    *result = sum;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Critical: pre-decrement with explicit != 0 comparison */
    while (--i != 0) {
        sum += (i % 7) + 1;
        g_volatile_sink = sum;
    }
    *result = sum;
}

/* Variant 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Critical: post-decrement with explicit != 0 comparison */
    while (i-- != 0) {
        sum ^= i;  /* XOR operation for variety */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Variant 4: do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int n, int *result) {
    int sum = 0;
    int i = n;
    if (i > 0) {
        do {
            sum += i * i;
            g_volatile_sink = sum;
        } while (--i != 0);  /* Critical: --i != 0 in do-while */
    }
    *result = sum;
}

/* Variant 5: for loop with explicit decrement in body */
NOOPT void loop_decrement_for_explicit(int n, int *result) {
    int sum = 0;
    int i;
    /* Critical: decrement in body, comparison is != 0 */
    for (i = n; i != 0; ) {
        sum += (i & 0xFF);
        g_volatile_sink = i;
        i--;  /* Decrement in body */
    }
    *result = sum;
}

/* Variant 6: unsigned counter (may generate different RTL) */
NOOPT void loop_decrement_unsigned(unsigned int n, unsigned int *result) {
    unsigned int sum = 0;
    /* Critical: unsigned decrement with != 0 comparison */
    for (unsigned int i = n; i != 0; i--) {
        sum += i * 3;
        g_volatile_sink = (int)i;
    }
    *result = sum;
}

/* Variant 7: counter in register variable hint */
NOOPT void loop_decrement_register(int n, int *result) {
    register int i asm("r12") = n;  /* Suggest register allocation */
    int sum = 0;
    /* Critical: register variable with != 0 comparison */
    while (i != 0) {
        sum += i * 5;
        g_volatile_sink = sum;
        i--;
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    int results[8] = {0};
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) loop_bound = 1000;
    } else {
        /* Use volatile to prevent compile-time constant */
        g_volatile_counter = 1000;
        loop_bound = get_loop_bound();
    }
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_dowhile(loop_bound, &results[3]);
    loop_decrement_for_explicit(loop_bound, &results[4]);
    loop_decrement_unsigned((unsigned int)loop_bound, (unsigned int*)&results[5]);
    loop_decrement_register(loop_bound, &results[6]);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum ^= results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    /* Use checksum in output to ensure all loops execute */
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile use to ensure side effects aren't optimized away */
    g_volatile_sink = checksum;
    
    return checksum != 0 ? 0 : 1;
}
