#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 1000;
static volatile int g_volatile_sink;

/* Different loop variants to increase hit probability */

/* Variant 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int bound) {
    volatile int sink = 0;
    for (int i = bound; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink = sink;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int bound) {
    volatile int sink = 0;
    int i = bound;
    while (--i != 0) {
        sink += (i & 0xFF);  /* Side effect */
    }
    g_volatile_sink = sink;
}

/* Variant 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int bound) {
    volatile int sink = 0;
    int i = bound;
    while (i-- != 0) {
        sink ^= i;  /* Side effect */
    }
    g_volatile_sink = sink;
}

/* Variant 4: do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int bound) {
    volatile int sink = 0;
    int i = bound;
    if (i > 0) {
        do {
            sink += i * i;  /* Side effect */
        } while (--i != 0);
    }
    g_volatile_sink = sink;
}

/* Variant 5: for loop with unsigned counter */
NOOPT void loop_decrement_unsigned(unsigned int bound) {
    volatile unsigned int sink = 0;
    for (unsigned int i = bound; i != 0; i--) {
        sink += i % 37;  /* Side effect */
    }
    g_volatile_sink = sink;
}

/* Variant 6: Nested loops to create more complex scenario */
NOOPT void loop_decrement_nested(int bound) {
    volatile int sink = 0;
    int outer = bound / 10;
    for (int i = outer; i != 0; i--) {
        int inner = 10;
        while (inner-- != 0) {
            sink += i * inner;  /* Side effect */
        }
    }
    g_volatile_sink = sink;
}

/* Variant 7: Loop with if condition inside */
NOOPT void loop_decrement_with_if(int bound) {
    volatile int sink = 0;
    for (int i = bound; i != 0; i--) {
        if (i % 3 == 0) {
            sink += i;  /* Conditional side effect */
        } else {
            sink -= 1;
        }
    }
    g_volatile_sink = sink;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line argument or volatile to prevent constant propagation */
    int base_bound = g_volatile_bound;
    if (argc > 1) {
        base_bound = atoi(argv[1]);
    }
    
    /* Seed random for variability */
    srand(time(NULL));
    
    /* Call all loop variants with different bounds */
    int checksum = 0;
    
    /* Variant 1 */
    loop_decrement_for(base_bound);
    checksum += g_volatile_sink;
    
    /* Variant 2 */
    loop_decrement_while_predec(base_bound + 1);
    checksum += g_volatile_sink;
    
    /* Variant 3 */
    loop_decrement_while_postdec(base_bound + 2);
    checksum += g_volatile_sink;
    
    /* Variant 4 */
    loop_decrement_dowhile(base_bound + 3);
    checksum += g_volatile_sink;
    
    /* Variant 5 */
    loop_decrement_unsigned((unsigned int)(base_bound + 4));
    checksum += g_volatile_sink;
    
    /* Variant 6 */
    loop_decrement_nested(base_bound + 5);
    checksum += g_volatile_sink;
    
    /* Variant 7 */
    loop_decrement_with_if(base_bound + 6);
    checksum += g_volatile_sink;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
