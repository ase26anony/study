#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural optimization */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;

/* Function 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink = sink;
}

/* Function 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n) {
    volatile int sink = 0;
    int i = n;
    while (--i != 0) {
        sink += (i & 0xFF) * 7;
    }
    g_volatile_sink = sink;
}

/* Function 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n) {
    volatile int sink = 0;
    int i = n;
    while (i-- != 0) {
        sink += i * 11;
    }
    g_volatile_sink = sink;
}

/* Function 4: do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int n) {
    volatile int sink = 0;
    int i = n;
    if (i > 0) {
        do {
            sink += i * 13;
        } while (--i != 0);
    }
    g_volatile_sink = sink;
}

/* Function 5: for loop with unsigned counter */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    for (unsigned int i = n; i != 0; i--) {
        sink += i * 17;
    }
    g_volatile_sink = sink;
}

/* Function 6: Nested loops to create more complex scenario */
NOOPT void loop_decrement_nested(int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i--) {
        for (int j = 10; j != 0; j--) {
            sink += i * j;
        }
    }
    g_volatile_sink = sink;
}

/* Function 7: Loop with if condition inside */
NOOPT void loop_decrement_with_if(int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i--) {
        if (i % 2 == 0) {
            sink += i * 19;
        } else {
            sink += i * 23;
        }
    }
    g_volatile_sink = sink;
}

/* Function 8: Loop that modifies array */
NOOPT void loop_decrement_array(int n) {
    volatile int sink = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Decrementing loop with array access */
    for (int i = n; i != 0; i--) {
        arr[i % 100] += i * 29;
        sink += arr[i % 100];
    }
    
    g_volatile_sink = sink;
}

int main(int argc, char *argv[]) {
    /* Use command-line argument or volatile read for non-constant loop bound */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile to prevent compile-time constant propagation */
        volatile int vol_bound = 1000;
        loop_bound = vol_bound;
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants to increase coverage probability */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_nested(loop_bound / 10);  /* Smaller bound for nested */
    loop_decrement_with_if(loop_bound);
    loop_decrement_array(loop_bound % 100 + 50);  /* Reasonable array size */
    
    /* Final checksum to prevent dead code elimination */
    volatile int final_sink = g_volatile_sink;
    printf("Final sink value: %d\n", final_sink);
    
    return 0;
}
