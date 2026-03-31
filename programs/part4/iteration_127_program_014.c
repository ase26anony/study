#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Variant 1: for loop with i-- and != 0 condition */
NOOPT void loop_decrement_for(int n) {
    volatile int local_sink = 0;
    for (int i = n; i != 0; i--) {
        local_sink += i * 3;
    }
    global_sink += local_sink;
}

/* Variant 2: while loop with pre-decrement and != 0 condition */
NOOPT void loop_decrement_while_predec(int n) {
    volatile int local_sink = 0;
    int i = n;
    while (--i != 0) {
        local_sink += i * 5;
    }
    global_sink += local_sink;
}

/* Variant 3: while loop with post-decrement and != 0 condition */
NOOPT void loop_decrement_while_postdec(int n) {
    volatile int local_sink = 0;
    int i = n;
    while (i-- != 0) {
        local_sink += i * 7;
    }
    global_sink += local_sink;
}

/* Variant 4: do-while with explicit decrement and != 0 check */
NOOPT void loop_decrement_dowhile(int n) {
    volatile int local_sink = 0;
    int i = n;
    if (i > 0) {
        do {
            local_sink += i * 11;
        } while (--i != 0);
    }
    global_sink += local_sink;
}

/* Variant 5: for loop with explicit decrement in body */
NOOPT void loop_decrement_for_explicit(int n) {
    volatile int local_sink = 0;
    int i;
    for (i = n; i != 0; ) {
        local_sink += i * 13;
        i--;
    }
    global_sink += local_sink;
}

/* Variant 6: unsigned counter to avoid signed overflow issues */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    volatile unsigned int local_sink = 0;
    for (unsigned int i = n; i != 0; i--) {
        local_sink += i * 17;
    }
    global_sink += local_sink;
}

/* Variant 7: counter in separate variable with complex exit */
NOOPT void loop_decrement_complex(int n) {
    volatile int local_sink = 0;
    int counter = n;
    int result;
    
    while (1) {
        result = counter - 1;
        if (result == 0) break;
        local_sink += counter * 19;
        counter = result;
    }
    global_sink += local_sink;
}

/* Main function with non-constant loop bound */
int main(int argc, char *argv[]) {
    /* Use command-line argument or volatile to prevent constant propagation */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile to make bound unknown at compile time */
        volatile int volatile_bound = 1000;
        loop_bound = volatile_bound;
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    
    /* Initialize global sink */
    global_sink = 0;
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_for_explicit(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_complex(loop_bound);
    
    /* Print checksum to prevent elimination */
    printf("Checksum: %d\n", global_sink);
    
    return 0;
}
