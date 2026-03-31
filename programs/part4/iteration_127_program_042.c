#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 1000;
static volatile int g_volatile_sink = 0;

/* Different loop variants to increase hit probability */

/* Variant 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n) {
    int i;
    volatile int sink = 0;
    
    /* Pattern: (reg - 1) != 0 */
    for (i = n; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    
    g_volatile_sink += sink;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n) {
    int i = n;
    volatile int sink = 0;
    
    /* Pattern: (reg - 1) != 0 with pre-decrement */
    while (--i != 0) {
        sink += (i & 0xFF);  /* Side effect */
    }
    
    g_volatile_sink += sink;
}

/* Variant 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n) {
    int i = n;
    volatile int sink = 0;
    
    /* Pattern: (reg - 1) != 0 with post-decrement */
    while (i-- != 0) {
        sink ^= i;  /* Side effect */
    }
    
    g_volatile_sink += sink;
}

/* Variant 4: do-while with explicit decrement and comparison */
NOOPT void loop_decrement_dowhile(int n) {
    int i = n;
    volatile int sink = 0;
    
    if (i > 0) {
        do {
            sink += (i % 256);
            i--;
        } while (i != 0);  /* Explicit comparison with 0 */
    }
    
    g_volatile_sink += sink;
}

/* Variant 5: for loop with complex decrement expression */
NOOPT void loop_decrement_complex(int n) {
    int i = n;
    volatile int sink = 0;
    
    /* Force counter into register with arithmetic */
    for (; i != 0; i = i - 1) {
        sink += (i * i) & 0xFF;
    }
    
    g_volatile_sink += sink;
}

/* Variant 6: unsigned counter to avoid signed overflow issues */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    unsigned int i = n;
    volatile unsigned int sink = 0;
    
    /* Pattern with unsigned counter */
    for (; i != 0; i--) {
        sink += i * 7;
    }
    
    g_volatile_sink += sink;
}

/* Variant 7: Nested loops to create more complex control flow */
NOOPT void loop_decrement_nested(int n) {
    int i, j;
    volatile int sink = 0;
    
    for (i = n; i != 0; i--) {
        for (j = 10; j != 0; j--) {
            sink += i * j;
        }
    }
    
    g_volatile_sink += sink;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        loop_bound = g_volatile_bound;
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_complex(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_nested(loop_bound / 10);  /* Smaller bound for nested */
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", g_volatile_sink);
    
    return 0;
}
