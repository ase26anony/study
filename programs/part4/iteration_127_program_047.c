/* doloop_coverage.c
 * Designed to trigger GCC's doloop_optimize pattern matching for (reg - 1) != 0
 * Compile with: gcc -O2 -fdump-rtl-doloop -S doloop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 1000;
static volatile int g_volatile_sink = 0;

/* Different loop variants to increase coverage probability */

/* Variant 1: for loop with i-- and explicit != 0 comparison */
NOOPT void loop_variant1(int bound) {
    volatile int sink = 0;
    /* for (i = bound; i != 0; i--) */
    for (int i = bound; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink += sink;
}

/* Variant 2: while loop with pre-decrement and != 0 comparison */
NOOPT void loop_variant2(int bound) {
    volatile int sink = 0;
    int i = bound;
    /* while (--i != 0) */
    while (--i != 0) {
        sink += i * 5;
    }
    g_volatile_sink += sink;
}

/* Variant 3: while loop with post-decrement and != 0 comparison */
NOOPT void loop_variant3(int bound) {
    volatile int sink = 0;
    int i = bound;
    /* while (i-- != 0) */
    while (i-- != 0) {
        sink += i * 7;
    }
    g_volatile_sink += sink;
}

/* Variant 4: do-while with explicit decrement and comparison */
NOOPT void loop_variant4(int bound) {
    volatile int sink = 0;
    int i = bound;
    if (i > 0) {
        do {
            sink += i * 11;
        } while (--i != 0);
    }
    g_volatile_sink += sink;
}

/* Variant 5: for loop with separate decrement, explicit != 0 */
NOOPT void loop_variant5(int bound) {
    volatile int sink = 0;
    int i;
    for (i = bound; i != 0; ) {
        sink += i * 13;
        i--;
    }
    g_volatile_sink += sink;
}

/* Variant 6: unsigned counter to avoid signed overflow issues */
NOOPT void loop_variant6(unsigned int bound) {
    volatile unsigned int sink = 0;
    /* for (u = bound; u != 0; u--) */
    for (unsigned int u = bound; u != 0; u--) {
        sink += u * 17;
    }
    g_volatile_sink += sink;
}

/* Variant 7: counter in register with complex exit condition */
NOOPT void loop_variant7(int bound) {
    volatile int sink = 0;
    register int r asm("r12") = bound; /* Hint for register allocation */
    while (1) {
        int temp = r - 1;
        if (temp == -1) break; /* Equivalent to r != 0 after decrement */
        sink += r * 19;
        r = temp;
    }
    g_volatile_sink += sink;
}

/* Main driver that makes bound non-constant */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to make bound non-constant */
    int bound = g_volatile_bound;
    if (argc > 1) {
        bound = atoi(argv[1]);
        if (bound <= 0) bound = 100;
    }
    
    unsigned int u_bound = (unsigned int)bound;
    
    /* Call all variants to maximize coverage chance */
    loop_variant1(bound);
    loop_variant2(bound);
    loop_variant3(bound);
    loop_variant4(bound);
    loop_variant5(bound);
    loop_variant6(u_bound);
    loop_variant7(bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", g_volatile_sink);
    
    return 0;
}
