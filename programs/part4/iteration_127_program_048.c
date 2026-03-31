#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
volatile int global_seed;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n, volatile int *result) {
    int sum = 0;
    /* Pattern: for (reg = n; reg != 0; reg--) */
    for (int i = n; i != 0; i--) {
        sum += i * 2;
        /* Side effect to prevent dead code elimination */
        *result = sum;
    }
    *result = sum;
}

NOOPT void loop_decrement_while_predec(int n, volatile int *result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: while (--cnt != 0) */
    while (--cnt != 0) {
        sum += cnt * 3;
        *result = sum;
    }
    *result = sum;
}

NOOPT void loop_decrement_while_postdec(int n, volatile int *result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: while (cnt-- != 0) */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 5;
        *result = sum;
    }
    *result = sum;
}

NOOPT void loop_decrement_do_while(int n, volatile int *result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: do {...} while (--cnt != 0) */
    if (cnt > 0) {
        do {
            sum += cnt * 7;
            *result = sum;
        } while (--cnt != 0);
    }
    *result = sum;
}

NOOPT void loop_decrement_for_complex(int n, volatile int *result) {
    int sum = 0;
    /* Pattern with arithmetic in condition: for (int i = n; (i - 1) != -1; i--) */
    for (int i = n; i != 0; ) {
        sum += i * 11;
        *result = sum;
        i--;
    }
    *result = sum;
}

NOOPT void loop_decrement_unsigned(int n, volatile int *result) {
    unsigned int sum = 0;
    unsigned int cnt = (unsigned int)n;
    /* Unsigned decrement pattern */
    while (cnt-- != 0) {
        sum += cnt * 13;
        *result = (int)sum;
    }
    *result = (int)sum;
}

/* Helper to make loop bound non-constant at compile time */
int get_loop_bound(void) {
    /* Use volatile read to prevent constant propagation */
    volatile int bound = global_seed;
    if (bound <= 0) bound = 1000;  /* Ensure positive bound */
    return bound % 1000 + 100;     /* Return 100-1099 iterations */
}

int main(int argc, char *argv[]) {
    /* Initialize with non-constant value */
    global_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    
    volatile int results[6] = {0};
    int loop_bound = get_loop_bound();
    
    printf("Testing doloop pattern with bound = %d\n", loop_bound);
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_do_while(loop_bound, &results[3]);
    loop_decrement_for_complex(loop_bound, &results[4]);
    loop_decrement_unsigned(loop_bound, &results[5]);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum = %d\n", checksum);
    
    /* Use checksum to affect return value */
    return (checksum != 0) ? 0 : 1;
}
