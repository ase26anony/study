#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimizations and inlining */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_source = 1000;

/* Function prototypes */
NOOPT void loop_decrement_for(int n, int *result);
NOOPT void loop_decrement_while_predec(int n, int *result);
NOOPT void loop_decrement_while_postdec(int n, int *result);
NOOPT void loop_decrement_do_while(int n, int *result);
NOOPT void loop_decrement_for_unsigned(unsigned int n, int *result);

/* Variant 1: for loop with i != 0 condition */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, exit when i != 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect that depends on counter to prevent dead code elimination */
        sum += (i & 0xFF);
        /* Additional volatile write to prevent optimization */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Variant 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Pre-decrement pattern: --i != 0 */
    while (--i != 0) {
        sum += (i & 0x3F);
        g_volatile_sink = i * 2;
    }
    /* Handle the case when n might be 0 */
    if (n > 0) {
        sum += n;  /* Account for the skipped iteration */
    }
    *result = sum;
}

/* Variant 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Post-decrement pattern: i-- != 0 */
    while (i-- != 0) {
        sum += (i & 0x7F);
        g_volatile_sink = i + 1;
    }
    *result = sum;
}

/* Variant 4: do-while loop with explicit decrement */
NOOPT void loop_decrement_do_while(int n, int *result) {
    int sum = 0;
    int i = n;
    if (i > 0) {
        do {
            sum += (i & 0x1F);
            g_volatile_sink = i * 3;
        } while (--i != 0);  /* Decrement and compare with 0 */
    }
    *result = sum;
}

/* Variant 5: unsigned counter (might generate different RTL) */
NOOPT void loop_decrement_for_unsigned(unsigned int n, int *result) {
    int sum = 0;
    /* Unsigned counter, still using != 0 condition */
    for (unsigned int i = n; i != 0; i--) {
        sum += (int)(i & 0xFF);
        g_volatile_sink = (int)i;
    }
    *result = sum;
}

/* Main function with non-constant loop bounds */
int main(int argc, char *argv[]) {
    int results[5] = {0};
    int total = 0;
    
    /* Use volatile source to prevent constant propagation */
    int loop_bound = g_volatile_source;
    
    /* Also use command line argument if provided */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) {
            loop_bound = 100;
        }
    }
    
    /* Limit loop bound to avoid excessive runtime */
    if (loop_bound > 1000000) {
        loop_bound = 1000000;
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_do_while(loop_bound, &results[3]);
    loop_decrement_for_unsigned((unsigned int)loop_bound, &results[4]);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 5; i++) {
        total += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Total checksum = %d\n", total);
    
    /* Use the results to affect program output */
    if (total > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure - all loops optimized away */
    }
}
