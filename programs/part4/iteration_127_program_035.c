#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n) {
    int sum = 0;
    /* for loop with i != 0 condition */
    for (int i = n; i != 0; i--) {
        sum += i * 2;
    }
    sink = sum;  /* Side effect */
}

NOOPT void loop_decrement_while_predec(int n) {
    int sum = 0;
    int i = n;
    /* while with --i != 0 pattern */
    while (--i != 0) {
        sum += i * 3;
    }
    sink = sum;
}

NOOPT void loop_decrement_while_postdec(int n) {
    int sum = 0;
    int i = n;
    /* while with i-- != 0 pattern */
    while (i-- != 0) {
        sum += i * 5;
    }
    sink = sum;
}

NOOPT void loop_decrement_do_while(int n) {
    int sum = 0;
    int i = n;
    /* do-while with explicit decrement and != 0 check */
    if (i > 0) {
        do {
            sum += i * 7;
        } while (--i != 0);
    }
    sink = sum;
}

NOOPT void loop_decrement_for_unsigned(unsigned int n) {
    unsigned int sum = 0;
    /* unsigned counter with != 0 condition */
    for (unsigned int i = n; i != 0; i--) {
        sum += i * 11;
    }
    sink = (int)sum;
}

NOOPT void loop_decrement_complex(int n) {
    int sum = 0;
    int i = n;
    /* More complex pattern with multiple operations */
    while (1) {
        sum += i * 13;
        i--;
        if (i == 0) break;
    }
    sink = sum;
}

NOOPT void loop_decrement_with_if(int n) {
    int sum = 0;
    int i = n;
    /* Loop with if condition that should still generate (reg-1) != 0 */
    while (i != 0) {
        if (sum < 1000000) {
            sum += i * 17;
        }
        i--;
    }
    sink = sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile or external input to prevent constant propagation */
    volatile int base_iterations = 1000;
    int iterations;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    } else {
        iterations = base_iterations;
    }
    
    /* Ensure iterations is positive and not trivially small */
    if (iterations < 100) iterations = 100;
    if (iterations > 10000) iterations = 10000;
    
    printf("Testing with %d iterations\n", iterations);
    
    /* Call all loop variants */
    loop_decrement_for(iterations);
    loop_decrement_while_predec(iterations);
    loop_decrement_while_postdec(iterations);
    loop_decrement_do_while(iterations);
    loop_decrement_for_unsigned(iterations);
    loop_decrement_complex(iterations);
    loop_decrement_with_if(iterations);
    
    /* Use sink value to prevent optimization */
    printf("Final sink value: %d\n", sink);
    
    return 0;
}
