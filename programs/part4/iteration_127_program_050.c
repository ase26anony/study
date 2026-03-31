#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n) {
    int i;
    for (i = n; i != 0; i--) {
        sink = i * 3;  /* Side effect depending on counter */
    }
}

NOOPT void loop_decrement_while_predec(int n) {
    int cnt = n;
    while (--cnt != 0) {
        sink = cnt + 7;
    }
}

NOOPT void loop_decrement_while_postdec(int n) {
    int cnt = n;
    while (cnt-- != 0) {
        sink = cnt * 2;
    }
}

NOOPT void loop_decrement_do_while(int n) {
    int cnt = n;
    if (cnt > 0) {
        do {
            sink = cnt | 0x55;
        } while (--cnt != 0);
    }
}

NOOPT void loop_decrement_for_complex(int n) {
    int i = n;
    for (; i != 0; ) {
        sink = i ^ 0xFF;
        i--;
    }
}

NOOPT void loop_decrement_mixed(int n) {
    int counter = n;
    while (counter != 0) {
        sink = counter * counter;
        counter = counter - 1;  /* Explicit subtraction */
    }
}

NOOPT void loop_decrement_unsigned(unsigned int n) {
    unsigned int i;
    for (i = n; i != 0; i--) {
        sink = (int)i + 100;
    }
}

/* Main function with non-constant loop bound */
int main(int argc, char *argv[]) {
    int iterations;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        iterations = atoi(argv[1]);
    } else {
        /* Use volatile to prevent constant propagation */
        volatile int vol_bound = 1000;
        iterations = vol_bound;
    }
    
    /* Ensure reasonable bounds */
    if (iterations <= 0) iterations = 1000;
    if (iterations > 1000000) iterations = 1000000;
    
    /* Execute all loop variants */
    loop_decrement_for(iterations);
    loop_decrement_while_predec(iterations);
    loop_decrement_while_postdec(iterations);
    loop_decrement_do_while(iterations);
    loop_decrement_for_complex(iterations);
    loop_decrement_mixed(iterations);
    loop_decrement_unsigned((unsigned int)iterations);
    
    /* Print something to prevent optimization */
    printf("Result: %d\n", sink);
    
    return 0;
}
