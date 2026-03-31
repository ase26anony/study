/* test-doloop.c
 * This program is designed to trigger the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) which matches the pattern (reg - 1) != 0.
 * Compile with: gcc -O2 -fdump-rtl-doloop -fdump-rtl-loop2 test-doloop.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Function 1: for loop with i-- and explicit != 0 condition */
NOOPT void loop_decrement_for(int n) {
    int i;
    volatile int local_sink = 0;
    
    /* Pattern: for (reg = n; reg != 0; reg--) */
    for (i = n; i != 0; i--) {
        local_sink += i * 3;
    }
    
    global_sink += local_sink;
}

/* Function 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    /* Pattern: while (--reg != 0) */
    while (--i != 0) {
        local_sink ^= i;
    }
    
    global_sink += local_sink;
}

/* Function 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    /* Pattern: while (reg-- != 0) */
    while (i-- != 0) {
        local_sink |= (1 << (i & 7));
    }
    
    global_sink += local_sink;
}

/* Function 4: do-while with explicit decrement and check */
NOOPT void loop_decrement_dowhile(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    if (i > 0) {
        do {
            local_sink = local_sink * 13 + i;
            i--;
        } while (i != 0);  /* Explicit (reg != 0) check */
    }
    
    global_sink += local_sink;
}

/* Function 5: for loop with compound decrement in condition */
NOOPT void loop_decrement_for_compound(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    /* Pattern: for (; (i - 1) != 0; ) with manual decrement */
    for (; i != 0; ) {
        local_sink += i;
        i--;
    }
    
    global_sink += local_sink;
}

/* Function 6: unsigned counter (may generate different but valid pattern) */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    unsigned int i = n;
    volatile int local_sink = 0;
    
    while (i-- != 0) {
        local_sink += (int)i * 7;
    }
    
    global_sink += local_sink;
}

/* Function 7: counter in register variable hint */
NOOPT void loop_decrement_register_var(int n) {
    register int i asm ("r12") = n;  /* Hint, but compiler may ignore */
    volatile int local_sink = 0;
    
    for (; i != 0; i--) {
        local_sink = (local_sink << 1) | (i & 1);
    }
    
    global_sink += local_sink;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int iterations;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        iterations = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        volatile int vol_iter = 1000;
        iterations = vol_iter;
    }
    
    /* Ensure reasonable bounds */
    if (iterations <= 0) iterations = 1000;
    if (iterations > 1000000) iterations = 1000000;
    
    printf("Testing with %d iterations\n", iterations);
    
    /* Call all loop variants */
    loop_decrement_for(iterations);
    loop_decrement_while_predec(iterations);
    loop_decrement_while_postdec(iterations);
    loop_decrement_dowhile(iterations);
    loop_decrement_for_compound(iterations);
    loop_decrement_unsigned((unsigned int)iterations);
    loop_decrement_register_var(iterations);
    
    /* Print checksum to prevent elimination */
    printf("Checksum: %d\n", global_sink);
    
    return 0;
}
