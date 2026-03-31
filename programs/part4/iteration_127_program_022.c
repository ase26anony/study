/* doloop_coverage.c
 * Designed to trigger GCC's doloop_optimize pattern matching for (reg - 1) != 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int volatile_bound = 0;
static volatile int volatile_sink = 0;

/* Function 1: Classic for loop with i != 0 condition */
NOOPT void loop_decrement_for(int n) {
    int i;
    int sum = 0;
    
    /* Decrementing counter with explicit != 0 comparison */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        sum += i * 2;
        /* Additional volatile write to prevent dead code elimination */
        volatile_sink = i;
    }
    
    /* Use the result */
    volatile_sink = sum;
}

/* Function 2: While loop with pre-decrement and != 0 check */
NOOPT void loop_decrement_while_predec(int n) {
    int cnt = n;
    int acc = 0;
    
    while (--cnt != 0) {
        /* Different side effect pattern */
        acc ^= cnt;
        volatile_sink = cnt;
    }
    
    volatile_sink = acc;
}

/* Function 3: While loop with post-decrement and != 0 check */
NOOPT void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int prod = 1;
    
    while (cnt-- != 0) {
        /* Multiplication side effect */
        prod *= (cnt + 1) % 7 + 1;
        volatile_sink = prod;
    }
    
    volatile_sink = prod;
}

/* Function 4: Do-while with explicit decrement and comparison */
NOOPT void loop_decrement_dowhile(int n) {
    int i = n;
    int arr[256] = {0};
    
    if (i <= 0) return;
    
    do {
        /* Array access side effect */
        arr[i % 256] = i;
        volatile_sink = arr[i % 256];
        i--;
    } while (i != 0);
}

/* Function 5: Nested decrement pattern */
NOOPT void loop_decrement_nested(int n) {
    int outer = n / 2;
    int inner;
    int total = 0;
    
    while (outer != 0) {
        inner = outer;
        while (inner != 0) {
            total += inner + outer;
            volatile_sink = inner;
            inner--;
        }
        outer--;
    }
    
    volatile_sink = total;
}

/* Function 6: Counter in register with complex expression */
NOOPT void loop_decrement_complex(int n) {
    register int reg_counter asm("r12") = n; /* Hint for register allocation */
    int result = 0;
    
    for (; reg_counter != 0; reg_counter--) {
        /* Use rand() to create unpredictable side effect */
        result += rand() % 100;
        volatile_sink = reg_counter;
    }
    
    volatile_sink = result;
}

/* Main driver with non-constant loop bounds */
int main(int argc, char *argv[]) {
    int base_iterations;
    
    /* Make loop bound non-constant at compile time */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        base_iterations = volatile_bound;
        if (base_iterations == 0) {
            base_iterations = 1000; /* Default */
        }
    }
    
    /* Also use time to add runtime variability */
    srand(time(NULL));
    int iterations = base_iterations + (rand() % 100);
    
    printf("Testing doloop pattern with %d iterations\n", iterations);
    
    /* Call all loop variants */
    loop_decrement_for(iterations);
    loop_decrement_while_predec(iterations);
    loop_decrement_while_postdec(iterations);
    loop_decrement_dowhile(iterations);
    loop_decrement_nested(iterations);
    loop_decrement_complex(iterations);
    
    /* Final checksum to prevent elimination */
    printf("Final volatile sink: %d\n", volatile_sink);
    
    return 0;
}
