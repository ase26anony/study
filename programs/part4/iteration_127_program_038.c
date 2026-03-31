#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_counter;
static volatile int g_volatile_result;

/* Function prototypes */
NOOPT void loop_decrement_for(int n);
NOOPT void loop_decrement_while(int n);
NOOPT void loop_decrement_do_while(int n);
NOOPT void loop_decrement_predec(int n);
NOOPT void loop_decrement_postdec(int n);

/* Variant 1: for loop with i-- and != 0 condition */
NOOPT void loop_decrement_for(int n) {
    volatile int local_sum = 0;
    /* Counter must be in register, use plain int */
    int i;
    
    /* Pattern: (reg - 1) != 0 */
    for (i = n; i != 0; i--) {
        /* Side effect depending on counter to prevent dead code elimination */
        local_sum += i * 3;
        /* Additional volatile write to ensure side effect */
        g_volatile_result = local_sum;
    }
    
    /* Ensure result is used */
    g_volatile_counter = local_sum;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    
    /* Pattern: (reg - 1) != 0 with pre-decrement */
    while (--cnt != 0) {
        local_sum += cnt * 5;
        g_volatile_result = local_sum;
    }
    
    g_volatile_counter = local_sum;
}

/* Variant 3: do-while loop with explicit decrement and comparison */
NOOPT void loop_decrement_do_while(int n) {
    volatile int local_sum = 0;
    int counter = n;
    
    if (counter > 0) {
        do {
            local_sum += counter * 7;
            g_volatile_result = local_sum;
            counter--;
        } while (counter != 0);  /* Explicit != 0 comparison */
    }
    
    g_volatile_counter = local_sum;
}

/* Variant 4: while loop with post-decrement */
NOOPT void loop_decrement_postdec(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    
    /* Pattern: cnt-- != 0 should generate (reg - 1) != 0 */
    while (cnt-- != 0) {
        local_sum += cnt * 11;
        g_volatile_result = local_sum;
    }
    
    g_volatile_counter = local_sum;
}

/* Variant 5: Complex decrement pattern with multiple operations */
NOOPT void loop_decrement_complex(int n) {
    volatile int local_sum = 0;
    int i = n;
    
    /* Multiple decrement patterns in one function */
    while (i != 0) {
        local_sum += i * 13;
        g_volatile_result = local_sum;
        i--;  /* Separate decrement, should still match pattern */
    }
    
    g_volatile_counter = local_sum;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        g_volatile_counter = 1000;
        loop_bound = g_volatile_counter;
    }
    
    /* Ensure loop bound is reasonable */
    if (loop_bound <= 0) {
        loop_bound = 1000;
    }
    
    printf("Testing doloop optimization with bound: %d\n", loop_bound);
    
    /* Call all variants to increase coverage probability */
    loop_decrement_for(loop_bound);
    loop_decrement_while(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_postdec(loop_bound);
    loop_decrement_complex(loop_bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Final volatile result: %d\n", g_volatile_result);
    
    return 0;
}
