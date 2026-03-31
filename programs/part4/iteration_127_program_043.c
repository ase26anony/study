/* doloop_coverage.c
 * Designed to trigger GCC's doloop_optimize pattern matching
 * for (reg - 1) != 0 comparison pattern
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function 1: Classic for loop with i != 0 condition */
NOOPT void loop_decrement_for(int iterations) {
    volatile int local_sink = 0;
    for (int i = iterations; i != 0; i--) {
        local_sink += i * 3;  /* Side effect depending on counter */
    }
    global_sink += local_sink;
}

/* Function 2: While loop with explicit decrement and != 0 check */
NOOPT void loop_decrement_while(int iterations) {
    volatile int local_sink = 0;
    int cnt = iterations;
    while (cnt != 0) {
        local_sink += cnt * 5;
        cnt--;  /* Decrement after use */
    }
    global_sink += local_sink;
}

/* Function 3: While loop with pre-decrement and != 0 check */
NOOPT void loop_decrement_while_pre(int iterations) {
    volatile int local_sink = 0;
    int cnt = iterations;
    while (--cnt != 0) {  /* Pre-decrement in condition */
        local_sink += cnt * 7;
    }
    /* Handle last iteration for exact count */
    if (iterations > 0) {
        local_sink += 7;
    }
    global_sink += local_sink;
}

/* Function 4: Do-while with decrementing counter */
NOOPT void loop_decrement_dowhile(int iterations) {
    volatile int local_sink = 0;
    int cnt = iterations;
    if (cnt > 0) {
        do {
            local_sink += cnt * 11;
            cnt--;
        } while (cnt != 0);
    }
    global_sink += local_sink;
}

/* Function 5: For loop with compound decrement in condition */
NOOPT void loop_decrement_for_compound(int iterations) {
    volatile int local_sink = 0;
    int i = iterations;
    for (; i != 0; ) {
        local_sink += i * 13;
        i--;  /* Separate decrement statement */
    }
    global_sink += local_sink;
}

/* Function 6: Unsigned counter variant */
NOOPT void loop_decrement_unsigned(unsigned int iterations) {
    volatile unsigned int local_sink = 0;
    for (unsigned int i = iterations; i != 0; i--) {
        local_sink += i * 17;
    }
    global_sink += local_sink;
}

/* Function 7: Counter in register with explicit register keyword */
NOOPT void loop_decrement_register(int iterations) {
    volatile int local_sink = 0;
    register int cnt asm("r12") = iterations;  /* Hint for register allocation */
    while (cnt != 0) {
        local_sink += cnt * 19;
        cnt--;
    }
    global_sink += local_sink;
}

/* Function 8: Nested loops to create more complex scenario */
NOOPT void loop_decrement_nested(int outer_iter, int inner_iter) {
    volatile int local_sink = 0;
    for (int i = outer_iter; i != 0; i--) {
        int inner = inner_iter;
        while (inner != 0) {
            local_sink += (i * inner) % 256;
            inner--;
        }
    }
    global_sink += local_sink;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int base_iterations = 1000;
    
    /* Also use command line argument if provided */
    int iterations = base_iterations;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = base_iterations;
        }
    }
    
    /* Use time-based variation to prevent pattern recognition */
    srand(time(NULL));
    int variation = rand() % 100;
    
    printf("Running doloop pattern tests with %d iterations...\n", iterations);
    
    /* Call all loop variants */
    loop_decrement_for(iterations + variation);
    loop_decrement_while(iterations + variation + 1);
    loop_decrement_while_pre(iterations + variation + 2);
    loop_decrement_dowhile(iterations + variation + 3);
    loop_decrement_for_compound(iterations + variation + 4);
    loop_decrement_unsigned(iterations + variation + 5);
    loop_decrement_register(iterations + variation + 6);
    loop_decrement_nested(10, iterations / 10 + variation);
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %d\n", global_sink);
    
    /* Additional anti-optimization: use result in unpredictable way */
    if (global_sink % 7 == 0) {
        printf("Result is divisible by 7\n");
    }
    
    return global_sink != 0 ? 0 : 1;
}
