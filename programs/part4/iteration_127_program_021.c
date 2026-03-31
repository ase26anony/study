#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent optimization */
volatile int g_volatile_sink;
volatile int g_volatile_source = 1000;

/* Function prototypes */
NOINLINE void loop_decrement_for(int n, int *result);
NOINLINE void loop_decrement_while_predec(int n, int *result);
NOINLINE void loop_decrement_while_postdec(int n, int *result);
NOINLINE void loop_decrement_do_while(int n, int *result);
NOINLINE void loop_decrement_for_unsigned(unsigned int n, int *result);

/* Variant 1: for loop with i != 0 condition */
NOINLINE void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, exit when i != 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        sum += i * 2;
        /* Additional side effect to volatile */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Variant 2: while loop with --i != 0 */
NOINLINE void loop_decrement_while_predec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Pre-decrement pattern */
    while (--i != 0) {
        sum += i * 3;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Variant 3: while loop with i-- != 0 */
NOINLINE void loop_decrement_while_postdec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Post-decrement pattern */
    while (i-- != 0) {
        sum += i * 5;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Variant 4: do-while loop with explicit check */
NOINLINE void loop_decrement_do_while(int n, int *result) {
    int sum = 0;
    int i = n;
    if (i != 0) {
        do {
            sum += i * 7;
            g_volatile_sink = i;
        } while (--i != 0);
    }
    *result = sum;
}

/* Variant 5: unsigned counter to ensure != 0 comparison */
NOINLINE void loop_decrement_for_unsigned(unsigned int n, int *result) {
    int sum = 0;
    /* Unsigned counter ensures != 0 comparison */
    for (unsigned int i = n; i != 0; i--) {
        sum += (int)i * 11;
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
        if (loop_bound <= 0) loop_bound = 100;
    }
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
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
    
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile write to ensure side effects aren't optimized away */
    g_volatile_sink = total;
    
    return total != 0 ? 0 : 1;
}
