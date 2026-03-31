#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_source = 1000;

/* Different loop variants to increase hit probability */

NOINLINE void loop_decrement_for(int n) {
    volatile int sink = 0;
    /* for loop with i != 0 condition */
    for (int i = n; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink = sink;
}

NOINLINE void loop_decrement_while_predec(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sink += cnt * 7;
    }
    g_volatile_sink = sink;
}

NOINLINE void loop_decrement_while_postdec(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sink += cnt * 11;
    }
    g_volatile_sink = sink;
}

NOINLINE void loop_decrement_do_while(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* do-while with explicit comparison */
    if (cnt > 0) {
        do {
            sink += cnt * 13;
        } while (--cnt != 0);
    }
    g_volatile_sink = sink;
}

NOINLINE void loop_decrement_for_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    /* unsigned counter, != 0 condition */
    for (unsigned int i = n; i != 0; i--) {
        sink += i * 17;
    }
    g_volatile_sink = (int)sink;
}

NOINLINE void loop_decrement_complex(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* More complex but still (reg-1) != 0 pattern */
    while ((cnt - 1) != -1) {  /* Equivalent to cnt != 0 */
        sink += cnt * 19;
        cnt--;
    }
    g_volatile_sink = sink;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    /* Use volatile source to prevent constant propagation */
    int base_count = g_volatile_source;
    
    /* Also use command line argument if provided */
    if (argc > 1) {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 100;
    }
    
    /* Ensure count is positive and not trivially small */
    int count = (base_count & 0xFFF) + 100;  /* Range: 100-4195 */
    
    printf("Testing with count = %d\n", count);
    
    /* Call all loop variants */
    loop_decrement_for(count);
    loop_decrement_while_predec(count);
    loop_decrement_while_postdec(count);
    loop_decrement_do_while(count);
    loop_decrement_for_unsigned((unsigned int)count);
    loop_decrement_complex(count);
    
    /* Use results to prevent dead code elimination */
    int checksum = g_volatile_sink;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
