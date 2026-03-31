#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_bound = 1000;
static volatile int g_volatile_sink;

/* Function prototypes */
NOOPT void loop_decrement_for(int n, int *result);
NOOPT void loop_decrement_while_pre(int n, int *result);
NOOPT void loop_decrement_while_post(int n, int *result);
NOOPT void loop_decrement_do_while(int n, int *result);
NOOPT void loop_decrement_for_complex(int n, int *result);

/* Variant 1: for loop with i != 0 condition */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, exit when not equal to 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect that depends on counter to prevent dead code elimination */
        sum += (i & 0xFF);
        /* Additional volatile store to prevent optimization */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Variant 2: while loop with pre-decrement */
NOOPT void loop_decrement_while_pre(int n, int *result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: (reg - 1) != 0 */
    while (--cnt != 0) {
        sum += (cnt * 3) & 0xFF;
        g_volatile_sink = cnt;
    }
    *result = sum;
}

/* Variant 3: while loop with post-decrement */
NOOPT void loop_decrement_while_post(int n, int *result) {
    int sum = 0;
    int cnt = n;
    /* Different syntax, same pattern */
    while (cnt != 0) {
        sum += (cnt * 5) & 0xFF;
        g_volatile_sink = cnt;
        cnt--;
    }
    *result = sum;
}

/* Variant 4: do-while loop with explicit check */
NOOPT void loop_decrement_do_while(int n, int *result) {
    int sum = 0;
    int cnt = n;
    if (cnt > 0) {
        do {
            sum += (cnt * 7) & 0xFF;
            g_volatile_sink = cnt;
        } while (--cnt != 0);  /* Critical pattern here */
    }
    *result = sum;
}

/* Variant 5: for loop with more complex expression but same pattern */
NOOPT void loop_decrement_for_complex(int n, int *result) {
    int sum = 0;
    /* Force counter into register with register keyword */
    register int counter = n;
    for (; counter != 0; counter = counter - 1) {
        /* This should generate (counter - 1) in RTL */
        sum += (counter * 11) & 0xFF;
        g_volatile_sink = counter;
    }
    *result = sum;
}

/* Variant 6: Unsigned counter to test different RTL patterns */
NOOPT void loop_decrement_unsigned(unsigned int n, int *result) {
    unsigned int sum = 0;
    unsigned int cnt = n;
    while (cnt-- != 0) {  /* Post-decrement with != 0 check */
        sum += cnt & 0xFF;
        g_volatile_sink = (int)cnt;
    }
    *result = (int)sum;
}

/* Variant 7: Counter in different scope */
NOOPT void loop_decrement_nested(int n, int *result) {
    int sum = 0;
    {
        int local_counter = n;
        while (local_counter != 0) {
            sum += (local_counter * 13) & 0xFF;
            g_volatile_sink = local_counter;
            local_counter--;
        }
    }
    *result = sum;
}

/* Main function that drives all variants */
int main(int argc, char *argv[]) {
    int bound;
    
    /* Make loop bound non-constant at compile time */
    if (argc > 1) {
        bound = atoi(argv[1]);
    } else {
        /* Use volatile to prevent constant propagation */
        bound = g_volatile_bound;
    }
    
    /* Ensure bound is positive and reasonable */
    if (bound <= 0) bound = 1000;
    if (bound > 1000000) bound = 1000000;
    
    int results[8] = {0};
    int total = 0;
    
    /* Call all loop variants */
    loop_decrement_for(bound, &results[0]);
    loop_decrement_while_pre(bound, &results[1]);
    loop_decrement_while_post(bound, &results[2]);
    loop_decrement_do_while(bound, &results[3]);
    loop_decrement_for_complex(bound, &results[4]);
    loop_decrement_unsigned((unsigned int)bound, &results[5]);
    loop_decrement_nested(bound, &results[6]);
    
    /* Additional variant with different starting value */
    loop_decrement_for(bound + 7, &results[7]);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        total += results[i];
        /* Use result to prevent optimization */
        g_volatile_sink = results[i];
    }
    
    /* Print checksum to ensure all loops executed */
    printf("Checksum: %d\n", total);
    
    return 0;
}
