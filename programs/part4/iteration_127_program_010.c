#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink;

/* Different loop variants to increase coverage probability */

NOINLINE void loop_decrement_for(int n) {
    int i;
    int sum = 0;
    
    /* for loop with i != 0 condition */
    for (i = n; i != 0; i--) {
        sum += i * 3;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_while_predec(int n) {
    int cnt = n;
    int sum = 0;
    
    /* while loop with --cnt != 0 */
    while (--cnt != 0) {
        sum += cnt * 5;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_while_postdec(int n) {
    int cnt = n;
    int sum = 0;
    
    /* while loop with cnt-- != 0 */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 7;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_do_while(int n) {
    int cnt = n;
    int sum = 0;
    
    /* do-while with pre-decrement check */
    if (cnt > 0) {
        do {
            sum += cnt * 11;
        } while (--cnt != 0);
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_for_complex(int n) {
    unsigned int i;
    int sum = 0;
    
    /* for loop with unsigned counter */
    for (i = (unsigned int)n; i != 0; i--) {
        sum += (int)i * 13;
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_with_array(int n) {
    int i;
    int arr[100];
    int sum = 0;
    
    /* Initialize array to prevent optimization */
    for (int j = 0; j < 100; j++) {
        arr[j] = j * 2;
    }
    
    /* Loop with array access */
    for (i = n; i != 0; i--) {
        sum += arr[i % 100];
    }
    
    volatile_sink = sum;
}

NOINLINE void loop_decrement_mixed_ops(int n) {
    int cnt = n;
    int sum = 0;
    
    /* Loop with more complex operations but same pattern */
    while (cnt != 0) {
        sum = sum * 17 + cnt;
        cnt--;
    }
    
    volatile_sink = sum;
}

/* Main function with non-constant loop bound */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile to prevent constant propagation */
        volatile int seed = time(NULL);
        loop_bound = (seed % 1000) + 100;  /* 100-1099 iterations */
    }
    
    printf("Loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_for_complex(loop_bound);
    loop_decrement_with_array(loop_bound);
    loop_decrement_mixed_ops(loop_bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", volatile_sink);
    
    return 0;
}
