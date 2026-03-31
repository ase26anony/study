#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural optimization */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_sink;
static volatile int g_volatile_source = 1000;

/* Function 1: for loop with i-- != 0 */
NOOPT void loop_decrement_for(int n, int *result) {
    int sum = 0;
    /* Counter in register, decrement by 1, compare with != 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect depending on counter */
        sum += i * 2;
        /* Volatile write to prevent dead code elimination */
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 2: while loop with --i != 0 */
NOOPT void loop_decrement_while_predec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Pre-decrement pattern */
    while (--i != 0) {
        sum += i * 3;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 3: while loop with i-- != 0 */
NOOPT void loop_decrement_while_postdec(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Post-decrement pattern */
    while (i-- != 0) {
        sum += i * 5;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 4: do-while with explicit comparison */
NOOPT void loop_decrement_dowhile(int n, int *result) {
    int sum = 0;
    int i = n;
    if (i > 0) {
        do {
            sum += i * 7;
            g_volatile_sink = i;
        } while (--i != 0);  /* Explicit --i != 0 comparison */
    }
    *result = sum;
}

/* Function 5: for loop with unsigned counter */
NOOPT void loop_decrement_unsigned(unsigned int n, unsigned int *result) {
    unsigned int sum = 0;
    /* Unsigned counter, still != 0 comparison */
    for (unsigned int i = n; i != 0; i--) {
        sum += i * 11;
        g_volatile_sink = (int)i;
    }
    *result = sum;
}

/* Function 6: Complex expression in loop condition */
NOOPT void loop_decrement_complex(int n, int *result) {
    int sum = 0;
    int i = n;
    /* Force (reg - 1) pattern in comparison */
    while ((i - 1) != -1) {  /* Equivalent to i != 0 */
        i--;
        sum += i * 13;
        g_volatile_sink = i;
    }
    *result = sum;
}

/* Function 7: Nested loops to create different contexts */
NOOPT void loop_decrement_nested(int n, int *result) {
    int sum = 0;
    int outer = n / 10;
    
    for (int j = 0; j < outer; j++) {
        int inner = n;
        /* Inner loop with decrementing counter */
        while (inner-- != 0) {
            sum += (j * inner) * 17;
            g_volatile_sink = inner;
        }
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    int results[7] = {0};
    int total = 0;
    
    /* Get loop bound from volatile source to prevent constant propagation */
    int loop_bound = g_volatile_source;
    
    /* Use command line argument if provided for more variability */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) loop_bound = g_volatile_source;
    }
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_dowhile(loop_bound, &results[3]);
    loop_decrement_unsigned((unsigned int)loop_bound, (unsigned int*)&results[4]);
    loop_decrement_complex(loop_bound, &results[5]);
    loop_decrement_nested(loop_bound, &results[6]);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 7; i++) {
        total += results[i];
        /* Additional volatile access */
        g_volatile_sink = results[i];
    }
    
    printf("Checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero checksum\n");
    }
    
    return 0;
}
