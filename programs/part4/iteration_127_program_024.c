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
static volatile int g_volatile_source = 1000;

/* Function 1: Classic for loop with i != 0 condition */
NOOPT void loop_decrement_for(int iterations) {
    volatile int sink = 0;
    int i;
    
    /* for loop with explicit i != 0 comparison */
    for (i = iterations; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    
    g_volatile_sink = sink;  /* Ensure side effect is observable */
}

/* Function 2: While loop with post-decrement */
NOOPT void loop_decrement_while_post(int iterations) {
    volatile int sink = 0;
    int cnt = iterations;
    
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sink += (cnt + 1) * 5;  /* Use cnt+1 since cnt was decremented */
    }
    
    g_volatile_sink = sink;
}

/* Function 3: While loop with pre-decrement */
NOOPT void loop_decrement_while_pre(int iterations) {
    volatile int sink = 0;
    int cnt = iterations;
    
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
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
            cnt--;
        } while (cnt != 0);
    }
    
    g_volatile_sink = sink;
}

/* Function 5: Complex counter but still decrement by 1 */
NOOPT void loop_decrement_complex(int iterations) {
    volatile int sink = 0;
    int counter = iterations;
    int temp;
    
    while (counter != 0) {
        temp = counter - 1;  /* Decrement by 1 in separate step */
        sink += counter * 13;
        counter = temp;      /* Assign back */
    }
    
    g_volatile_sink = sink;
}

/* Function 6: Unsigned counter (might generate different but valid pattern) */
NOOPT void loop_decrement_unsigned(unsigned int iterations) {
    volatile unsigned int sink = 0;
    unsigned int i;
    
    for (i = iterations; i != 0; i--) {
        sink += i * 17;
    }
    
    g_volatile_sink = (int)sink;
}

/* Function 7: Counter with arithmetic in condition */
NOOPT void loop_decrement_cond_arith(int iterations) {
    volatile int sink = 0;
    int i = iterations;
    
    /* The comparison (i - 1) != 0 should match the pattern */
    while ((i - 1) != 0) {
        sink += i * 19;
        i--;
    }
    /* Handle last iteration */
    if (i > 0) {
        sink += i * 19;
    }
    
    g_volatile_sink = sink;
}

/* Function 8: Nested loops to create more complex CFG */
NOOPT void loop_decrement_nested(int outer_iter, int inner_iter) {
    volatile int sink = 0;
    int i, j;
    
    for (i = outer_iter; i != 0; i--) {
        for (j = inner_iter; j != 0; j--) {
            sink += (i * j) % 256;
        }
    }
    
    g_volatile_sink = sink;
}

/* Main driver that calls all variants */
int main(int argc, char *argv[]) {
    int base_iterations;
    
    /* Get iteration count from volatile source or command line */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) {
            base_iterations = g_volatile_source;
        }
    } else {
        base_iterations = g_volatile_source;
    }
    
    printf("Testing doloop optimization patterns with %d iterations\n", base_iterations);
    
    /* Call all loop variants */
    loop_decrement_for(base_iterations);
    loop_decrement_while_post(base_iterations);
    loop_decrement_while_pre(base_iterations);
    loop_decrement_dowhile(base_iterations);
    loop_decrement_complex(base_iterations);
    loop_decrement_unsigned((unsigned int)base_iterations);
    loop_decrement_cond_arith(base_iterations);
    loop_decrement_nested(base_iterations / 10, 10);
    
    /* Create checksum from volatile sink to prevent DCE */
    int checksum = g_volatile_sink;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
