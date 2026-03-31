#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n) {
    int i;
    int sum = 0;
    /* for loop with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        sum += i * 2;
    }
    sink = sum; /* Side effect */
}

NOOPT void loop_decrement_while_predec(int n) {
    int cnt = n;
    int sum = 0;
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sum += cnt * 3;
    }
    sink = sum; /* Side effect */
}

NOOPT void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int sum = 0;
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 5;
    }
    sink = sum; /* Side effect */
}

NOOPT void loop_decrement_do_while(int n) {
    int cnt = n;
    int sum = 0;
    /* do-while with pre-decrement check */
    if (cnt > 0) {
        do {
            sum += cnt * 7;
        } while (--cnt != 0);
    }
    sink = sum; /* Side effect */
}

NOOPT void loop_decrement_for_unsigned(unsigned int n) {
    unsigned int i;
    int sum = 0;
    /* unsigned counter with != 0 comparison */
    for (i = n; i != 0; i--) {
        sum += (int)i * 11;
    }
    sink = sum; /* Side effect */
}

NOOPT void loop_decrement_complex_body(int n) {
    int i = n;
    int arr[100];
    int sum = 0;
    
    /* Initialize array to prevent optimization */
    for (int j = 0; j < 100; j++) {
        arr[j] = j;
    }
    
    /* Loop with counter in register and != 0 comparison */
    while (i != 0) {
        arr[i % 100] += i;  /* Complex array access */
        sum += arr[i % 100];
        i--;
    }
    sink = sum; /* Side effect */
}

NOOPT void loop_decrement_nested(int n) {
    int outer = n / 2;
    int inner = n;
    int sum = 0;
    
    /* Outer loop */
    while (outer-- != 0) {
        int temp = inner;
        /* Inner loop with != 0 comparison */
        while (temp != 0) {
            sum += outer * temp;
            temp--;
        }
    }
    sink = sum; /* Side effect */
}

int main(int argc, char *argv[]) {
    /* Use volatile or argument to prevent constant propagation */
    volatile int base_iterations = 1000;
    int iterations;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    } else {
        iterations = base_iterations;
    }
    
    /* Ensure iterations is positive and non-zero */
    if (iterations <= 0) {
        iterations = 1000;
    }
    
    printf("Running loops with %d iterations\n", iterations);
    
    /* Call all loop variants */
    loop_decrement_for(iterations);
    loop_decrement_while_predec(iterations);
    loop_decrement_while_postdec(iterations);
    loop_decrement_do_while(iterations);
    loop_decrement_for_unsigned(iterations);
    loop_decrement_complex_body(iterations);
    loop_decrement_nested(iterations);
    
    /* Use results to prevent dead code elimination */
    printf("Final sink value: %d\n", sink);
    
    return 0;
}
