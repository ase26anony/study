#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_counter;
static volatile int g_volatile_sink;

/* Function prototypes */
NOINLINE void loop_decrement_for(int n);
NOINLINE void loop_decrement_while(int n);
NOINLINE void loop_decrement_do_while(int n);
NOINLINE void loop_decrement_complex(int n);
NOINLINE void loop_decrement_unsigned(unsigned int n);

/* Variant 1: Classic for loop with i-- and != 0 condition */
NOINLINE void loop_decrement_for(int n) {
    volatile int sink = 0;
    /* Counter must be in register, use simple int */
    int i;
    
    /* Pattern: (reg - 1) != 0 */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        sink += i * 3;
        /* Additional side effect to prevent dead code elimination */
        g_volatile_sink = sink;
    }
    
    g_volatile_counter = sink;
}

/* Variant 2: While loop with explicit decrement and comparison */
NOINLINE void loop_decrement_while(int n) {
    volatile int sink = 0;
    int cnt = n;
    
    /* Pattern: (reg - 1) != 0 */
    while (cnt != 0) {
        sink += cnt * 7;
        g_volatile_sink = sink;
        cnt--;
    }
    
    g_volatile_counter = sink;
}

/* Variant 3: While loop with pre-decrement */
NOINLINE void loop_decrement_do_while(int n) {
    volatile int sink = 0;
    int cnt = n;
    
    if (cnt > 0) {
        do {
            sink += cnt * 11;
            g_volatile_sink = sink;
            cnt--;
        } while (cnt != 0);  /* != 0 comparison */
    }
    
    g_volatile_counter = sink;
}

/* Variant 4: More complex but still matching the pattern */
NOINLINE void loop_decrement_complex(int n) {
    volatile int sink = 0;
    int counter = n;
    int temp;
    
    /* Force counter into register with computation */
    temp = counter + 1;
    
    while (counter != 0) {
        sink += (counter * 13) ^ temp;
        g_volatile_sink = sink;
        counter--;
        temp ^= counter;
    }
    
    g_volatile_counter = sink;
}

/* Variant 5: Using unsigned int to ensure != 0 comparison */
NOINLINE void loop_decrement_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int i;
    
    for (i = n; i != 0; i--) {
        sink += i * 17;
        g_volatile_sink = sink;
    }
    
    g_volatile_counter = (int)sink;
}

/* Helper to make loop bound non-constant */
static int get_loop_bound(void) {
    /* Use volatile and external input to prevent constant propagation */
    volatile int base = 1000;
    int r = rand() & 0xFF;  /* Random component */
    return base + r;
}

int main(int argc, char *argv[]) {
    int loop_bound;
    int checksum = 0;
    
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Get non-constant loop bound */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) loop_bound = 1000;
    } else {
        loop_bound = get_loop_bound();
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    checksum += g_volatile_counter;
    
    loop_decrement_while(loop_bound);
    checksum += g_volatile_counter;
    
    loop_decrement_do_while(loop_bound);
    checksum += g_volatile_counter;
    
    loop_decrement_complex(loop_bound);
    checksum += g_volatile_counter;
    
    loop_decrement_unsigned((unsigned int)loop_bound);
    checksum += g_volatile_counter;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
