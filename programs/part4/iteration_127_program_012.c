#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 1000;
static volatile int g_volatile_sink;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n) {
    volatile int sink = 0;
    /* for loop with i != 0 condition */
    for (int i = n; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_while_predec(int n) {
    volatile int sink = 0;
    int i = n;
    /* while loop with --i != 0 */
    while (--i != 0) {
        sink += (i % 7) * 5;
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_while_postdec(int n) {
    volatile int sink = 0;
    int i = n;
    /* while loop with i-- != 0 */
    while (i-- != 0) {
        sink += i * 11;
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_do_while(int n) {
    volatile int sink = 0;
    int i = n;
    /* do-while with explicit check */
    if (i > 0) {
        do {
            sink += (i & 0xFF) * 13;
        } while (--i != 0);
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    /* unsigned counter, != 0 condition */
    for (unsigned int i = n; i != 0; i--) {
        sink += i * 17;
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_with_if(int n) {
    volatile int sink = 0;
    /* More complex but still (reg-1) != 0 pattern */
    for (int i = n; i != 0; ) {
        sink += i * 19;
        if (--i == 0) break;
        sink += i * 23;
        if (--i == 0) break;
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_nested(int n) {
    volatile int sink = 0;
    /* Nested loops, outer uses decrement pattern */
    for (int i = n; i != 0; i--) {
        for (int j = 10; j > 0; j--) {
            sink += (i * j) % 256;
        }
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_array(int n) {
    volatile int sink = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 7;
    }
    
    /* Decrementing loop accessing array */
    for (int i = n; i != 0; i--) {
        sink += arr[i % 100];
    }
    g_volatile_sink += sink;
}

int main(int argc, char *argv[]) {
    /* Use volatile or command-line to prevent constant propagation */
    int loop_bound;
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Mix of volatile and computation for non-constant bound */
        loop_bound = g_volatile_bound + (time(NULL) % 100);
    }
    
    /* Ensure bound is positive and reasonable */
    if (loop_bound <= 0) loop_bound = 100;
    if (loop_bound > 1000000) loop_bound = 1000000;
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_with_if(loop_bound);
    loop_decrement_nested(loop_bound);
    loop_decrement_array(loop_bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", g_volatile_sink);
    
    return 0;
}
