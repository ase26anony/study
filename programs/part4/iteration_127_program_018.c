/* doloop_coverage.c
 * Designed to trigger GCC's doloop optimization pattern matching
 * for (reg - 1) != 0 comparison pattern
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;

/* Function 1: Classic for loop with i-- != 0 */
NOOPT void loop_decrement_for(int iterations) {
    volatile int sink = 0;
    for (int i = iterations; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink = sink;
}

/* Function 2: While loop with pre-decrement */
NOOPT void loop_decrement_while_pre(int iterations) {
    volatile int sink = 0;
    int cnt = iterations;
    while (--cnt != 0) {
        sink += cnt * 5;
    }
    g_volatile_sink = sink;
}

/* Function 3: While loop with post-decrement */
NOOPT void loop_decrement_while_post(int iterations) {
    volatile int sink = 0;
    int cnt = iterations;
    while (cnt-- != 0) {
        sink += cnt * 7;
    }
    g_volatile_sink = sink;
}

/* Function 4: Do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int iterations) {
    volatile int sink = 0;
    int cnt = iterations;
    if (cnt > 0) {
        do {
            sink += cnt * 11;
        } while (--cnt != 0);
    }
    g_volatile_sink = sink;
}

/* Function 5: For loop with separate decrement */
NOOPT void loop_decrement_for_separate(int iterations) {
    volatile int sink = 0;
    int i;
    for (i = iterations; i != 0; ) {
        sink += i * 13;
        i--;
    }
    g_volatile_sink = sink;
}

/* Function 6: Unsigned counter variant */
NOOPT void loop_decrement_unsigned(unsigned int iterations) {
    volatile unsigned int sink = 0;
    unsigned int i;
    for (i = iterations; i != 0; i--) {
        sink += i * 17;
    }
    g_volatile_sink = sink;
}

/* Function 7: Counter in register with complex body */
NOOPT void loop_decrement_complex(int iterations) {
    volatile int sink = 0;
    int reg_counter = iterations;  /* Force into register */
    
    /* Prevent other optimizations */
    asm volatile("" : "+r" (reg_counter));
    
    while (reg_counter != 0) {
        /* Multiple side effects to prevent dead code elimination */
        sink += reg_counter;
        sink ^= (reg_counter << 3);
        reg_counter--;
    }
    g_volatile_sink = sink;
}

/* Function 8: Nested decrement pattern */
NOOPT void loop_decrement_nested(int outer_iter, int inner_iter) {
    volatile int sink = 0;
    
    for (int i = outer_iter; i != 0; i--) {
        int j = inner_iter;
        while (j-- != 0) {
            sink += (i * j) ^ 0x55;
        }
    }
    g_volatile_sink = sink;
}

/* Main driver with non-constant iteration count */
int main(int argc, char *argv[]) {
    /* Make iteration count non-constant to prevent unrolling */
    int base_iterations;
    
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        volatile int v = 1000;
        base_iterations = v;
    }
    
    /* Ensure minimum iterations for loop formation */
    if (base_iterations < 10) {
        base_iterations = 100;
    }
    
    printf("Testing doloop pattern with %d iterations\n", base_iterations);
    
    /* Call all loop variants */
    loop_decrement_for(base_iterations);
    loop_decrement_while_pre(base_iterations);
    loop_decrement_while_post(base_iterations);
    loop_decrement_dowhile(base_iterations);
    loop_decrement_for_separate(base_iterations);
    loop_decrement_unsigned((unsigned int)base_iterations);
    loop_decrement_complex(base_iterations);
    loop_decrement_nested(base_iterations / 10, 10);
    
    /* Final side effect to prevent entire program elimination */
    printf("Result checksum: %d\n", g_volatile_sink);
    
    return g_volatile_sink != 0 ? 0 : 1;
}
