#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Function 1: Classic for loop with i-- != 0 */
NOINLINE void loop_decrement_for(int n) {
    int i;
    volatile int local_sink = 0;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        local_sink += i * 3;
    }
    
    global_sink += local_sink;
}

/* Function 2: While loop with --i != 0 */
NOINLINE void loop_decrement_while_predec(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    while (--i != 0) {
        local_sink += i * 5;
    }
    
    global_sink += local_sink;
}

/* Function 3: While loop with i-- != 0 */
NOINLINE void loop_decrement_while_postdec(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    while (i-- != 0) {
        local_sink += i * 7;
    }
    
    global_sink += local_sink;
}

/* Function 4: Do-while with pre-decrement check */
NOINLINE void loop_decrement_dowhile(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    if (i > 0) {
        do {
            local_sink += i * 11;
        } while (--i != 0);
    }
    
    global_sink += local_sink;
}

/* Function 5: For loop with explicit decrement in body */
NOINLINE void loop_decrement_for_explicit(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    for (;;) {
        if (i == 0) break;
        local_sink += i * 13;
        i--;
    }
    
    global_sink += local_sink;
}

/* Function 6: Loop with unsigned counter (may generate different RTL) */
NOINLINE void loop_decrement_unsigned(unsigned int n) {
    unsigned int i = n;
    volatile int local_sink = 0;
    
    while (i-- != 0) {
        local_sink += (int)i * 17;
    }
    
    global_sink += local_sink;
}

/* Function 7: Loop with counter in register variable hint */
NOINLINE void loop_decrement_register(int n) {
    register int i asm ("r12") = n; /* Suggest register, but compiler may ignore */
    volatile int local_sink = 0;
    
    for (; i != 0; i--) {
        local_sink += i * 19;
    }
    
    global_sink += local_sink;
}

/* Function 8: Nested loops to create more complex scenario */
NOINLINE void loop_decrement_nested(int n) {
    int i, j;
    volatile int local_sink = 0;
    
    for (i = n; i != 0; i--) {
        for (j = 3; j != 0; j--) {
            local_sink += i * j;
        }
    }
    
    global_sink += local_sink;
}

/* Main function with non-constant loop bound */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        volatile int seed = time(NULL);
        loop_bound = (seed % 100) + 50;  /* 50-149 iterations */
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_for_explicit(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_register(loop_bound);
    loop_decrement_nested(loop_bound);
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
