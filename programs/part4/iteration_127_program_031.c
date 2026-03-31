#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_counter;
static volatile int g_volatile_sink;

/* Function prototypes */
NOOPT void loop_decrement_for(int n);
NOOPT void loop_decrement_while(int n);
NOOPT void loop_decrement_do_while(int n);
NOOPT void loop_decrement_postdec(int n);
NOOPT void loop_decrement_predec(int n);

/* Variant 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink = sink;
}

/* Variant 2: while loop with explicit decrement */
NOOPT void loop_decrement_while(int n) {
    volatile int sink = 0;
    int cnt = n;
    while (cnt != 0) {
        sink += cnt * 7;
        cnt--;
    }
    g_volatile_sink = sink;
}

/* Variant 3: do-while with pre-decrement check */
NOOPT void loop_decrement_do_while(int n) {
    volatile int sink = 0;
    int cnt = n + 1;  /* Start one higher */
    do {
        cnt--;
        sink += cnt * 11;
    } while (cnt != 0);
    g_volatile_sink = sink;
}

/* Variant 4: while loop with post-decrement in condition */
NOOPT void loop_decrement_postdec(int n) {
    volatile int sink = 0;
    int cnt = n;
    while (cnt-- != 0) {
        sink += (cnt + 1) * 13;  /* Adjust for post-decrement */
    }
    g_volatile_sink = sink;
}

/* Variant 5: while loop with pre-decrement in condition */
NOOPT void loop_decrement_predec(int n) {
    volatile int sink = 0;
    int cnt = n + 1;  /* Start one higher */
    while (--cnt != 0) {
        sink += cnt * 17;
    }
    g_volatile_sink = sink;
}

/* Variant 6: for loop with --i != 0 */
NOOPT void loop_decrement_for_predec(int n) {
    volatile int sink = 0;
    for (int i = n + 1; --i != 0; ) {
        sink += i * 19;
    }
    g_volatile_sink = sink;
}

/* Variant 7: Complex exit condition to ensure pattern matching */
NOOPT void loop_decrement_complex(int n) {
    volatile int sink = 0;
    int cnt = n;
    while (1) {
        sink += cnt * 23;
        cnt--;
        if (cnt == 0) break;
    }
    g_volatile_sink = sink;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    /* Use command-line argument or volatile read for non-constant bound */
    int base_count;
    if (argc > 1) {
        base_count = atoi(argv[1]);
    } else {
        /* Use volatile to prevent compile-time constant */
        g_volatile_counter = 1000;
        base_count = g_volatile_counter;
    }
    
    /* Ensure count is positive and reasonable */
    if (base_count <= 0) base_count = 1000;
    
    /* Call all loop variants with different counts to increase coverage */
    int results[7] = {0};
    
    /* Different counts to test various patterns */
    loop_decrement_for(base_count);
    results[0] = g_volatile_sink;
    
    loop_decrement_while(base_count + 1);
    results[1] = g_volatile_sink;
    
    loop_decrement_do_while(base_count + 2);
    results[2] = g_volatile_sink;
    
    loop_decrement_postdec(base_count + 3);
    results[3] = g_volatile_sink;
    
    loop_decrement_predec(base_count + 4);
    results[4] = g_volatile_sink;
    
    loop_decrement_for_predec(base_count + 5);
    results[5] = g_volatile_sink;
    
    loop_decrement_complex(base_count + 6);
    results[6] = g_volatile_sink;
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Loop iterations completed successfully.\n");
    
    return 0;
}
