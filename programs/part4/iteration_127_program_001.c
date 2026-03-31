#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 0;
static volatile int g_volatile_sink = 0;

/* Different loop variants to increase coverage probability */

NOINLINE void loop_decrement_for(int n) {
    volatile int sink = 0;
    /* Pattern: for (reg != 0; reg--) */
    for (int i = n; i != 0; i--) {
        sink += i * 3;  /* Side effect depending on counter */
    }
    g_volatile_sink += sink;
}

NOINLINE void loop_decrement_while_predec(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* Pattern: while (--cnt != 0) */
    while (--cnt != 0) {
        sink ^= cnt;  /* Side effect */
    }
    g_volatile_sink += sink;
}

NOINLINE void loop_decrement_while_postdec(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* Pattern: while (cnt-- != 0) */
    while (cnt-- != 0) {
        sink |= cnt;  /* Side effect */
    }
    g_volatile_sink += sink;
}

NOINLINE void loop_decrement_do_while(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* Pattern: do { ... } while (--cnt != 0) */
    if (cnt > 0) {
        do {
            sink = sink * 7 + cnt;  /* Side effect */
        } while (--cnt != 0);
    }
    g_volatile_sink += sink;
}

NOINLINE void loop_decrement_for_complex(int n) {
    volatile int sink = 0;
    /* Pattern with explicit decrement in condition check */
    for (int i = n; (i - 1) != -1; i--) {
        sink += i & 0xFF;  /* Side effect */
    }
    g_volatile_sink += sink;
}

NOINLINE void loop_decrement_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    /* Unsigned variant */
    for (unsigned int i = n; i != 0; i--) {
        sink += i % 256;  /* Side effect */
    }
    g_volatile_sink += sink;
}

NOINLINE void loop_with_array(int n) {
    /* Use array to create memory side effects */
    static volatile int arr[1024];
    int sum = 0;
    
    for (int i = n; i != 0; i--) {
        arr[i % 1024] = i;  /* Side effect to volatile array */
        sum += i;
    }
    g_volatile_sink += sum;
}

NOINLINE void loop_nested_decrement(int n) {
    volatile int sink = 0;
    int outer = n / 2;
    
    /* Outer loop */
    for (int i = outer; i != 0; i--) {
        int inner = 10;
        /* Inner loop with decrement pattern */
        for (int j = inner; j != 0; j--) {
            sink += i * j;  /* Side effect */
        }
    }
    g_volatile_sink += sink;
}

/* Function to get non-constant loop bound */
static int get_loop_bound(void) {
    /* Use multiple sources to prevent constant propagation */
    int bound = g_volatile_bound;
    if (bound == 0) {
        bound = 1000;  /* Default if volatile is 0 */
    }
    
    /* Add some randomness to prevent pattern recognition */
    return bound + (clock() & 0xF);
}

int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Get non-constant loop bound from command line or volatile */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) loop_bound = 1000;
    } else {
        loop_bound = get_loop_bound();
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_for_complex(loop_bound);
    loop_decrement_unsigned(loop_bound);
    loop_with_array(loop_bound);
    loop_nested_decrement(loop_bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", g_volatile_sink);
    
    return 0;
}
