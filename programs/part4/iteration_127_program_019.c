/* doloop_coverage.c
 * Designed to trigger GCC's doloop_optimize pattern matching for (reg - 1) != 0
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_source = 1000;

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
    int cnt = n;
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sink += (cnt % 7) + 1;  /* Side effect */
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_while_postdec(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sink ^= cnt;  /* Side effect */
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_do_while(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* do-while with pre-decrement check */
    if (cnt > 0) {
        do {
            sink = sink * 13 + cnt;  /* Side effect */
        } while (--cnt != 0);
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_for_unsigned(unsigned int n) {
    volatile unsigned int sink = 0;
    /* unsigned counter, != 0 condition */
    for (unsigned int i = n; i != 0; i--) {
        sink += i & 0xFF;  /* Side effect */
    }
    g_volatile_sink += (int)sink;
}

NOOPT void loop_decrement_complex_expr(int n) {
    volatile int sink = 0;
    int cnt = n;
    /* More complex expression but still (reg - 1) != 0 pattern */
    while ((cnt - 1) != -1) {  /* Equivalent to cnt != 0 */
        sink += cnt * cnt;
        cnt--;
    }
    g_volatile_sink += sink;
}

NOOPT void loop_decrement_with_array(int n) {
    volatile int sink = 0;
    int cnt = n;
    int arr[100];
    
    /* Initialize array to prevent optimization */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Loop with counter in register */
    while (cnt != 0) {
        sink += arr[cnt % 100];  /* Side effect with array access */
        cnt--;
    }
    g_volatile_sink += sink;
}

NOOPT void loop_nested_decrement(int n) {
    volatile int sink = 0;
    int outer = n / 10;
    
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
    /* Use volatile to prevent constant propagation */
    int bound = g_volatile_source;
    
    /* Add some computation to make it non-trivial */
    bound = (bound % 997) + 100;  /* Ensure at least 100 iterations */
    
    return bound;
}

int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Get non-constant loop bound to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) {
            loop_bound = get_loop_bound();
        }
    } else {
        loop_bound = get_loop_bound();
    }
    
    printf("Running loops with bound = %d\n", loop_bound);
    
    /* Reset volatile sink */
    g_volatile_sink = 0;
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_for_unsigned((unsigned int)loop_bound);
    loop_decrement_complex_expr(loop_bound);
    loop_decrement_with_array(loop_bound);
    loop_nested_decrement(loop_bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", g_volatile_sink);
    
    return 0;
}
