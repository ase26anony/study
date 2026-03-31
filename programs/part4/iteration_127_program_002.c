/* test-doloop.c
 * 
 * This program is designed to trigger the specific pattern-matching logic
 * in GCC's doloop_optimize pass (loop-doloop.cc lines 136-150).
 * The pattern is: (reg - 1) != 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
static volatile int g_volatile_bound = 1000;

/* Side effect accumulator */
static volatile int g_side_effect = 0;

/* Array to create memory side effects */
static int g_array[10000];

/* ========== Loop Variant 1: for loop with i-- != 0 ========== */
NOINLINE
int loop_variant1(int bound) {
    int sum = 0;
    /* Classic decrementing for loop with != 0 comparison */
    for (int i = bound; i != 0; i--) {
        sum += i;
        g_array[i % 10000] = i;  /* Side effect */
    }
    g_side_effect += sum;
    return sum;
}

/* ========== Loop Variant 2: while loop with --i != 0 ========== */
NOINLINE
int loop_variant2(int bound) {
    int i = bound;
    int sum = 0;
    /* Pre-decrement in while condition */
    while (--i != 0) {
        sum += i;
        g_side_effect ^= i;  /* Different side effect */
    }
    return sum;
}

/* ========== Loop Variant 3: while loop with i-- != 0 ========== */
NOINLINE
int loop_variant3(int bound) {
    int i = bound;
    int sum = 0;
    /* Post-decrement in while condition */
    while (i-- != 0) {
        sum += i + 1;  /* Adjust for post-decrement */
        g_array[sum % 10000] = i;
    }
    g_side_effect += sum;
    return sum;
}

/* ========== Loop Variant 4: do-while with explicit check ========== */
NOINLINE
int loop_variant4(int bound) {
    int i = bound;
    int sum = 0;
    if (i <= 0) return 0;
    
    do {
        sum += i;
        g_side_effect |= i;  /* Another side effect */
    } while (--i != 0);  /* Decrement and compare with 0 */
    
    return sum;
}

/* ========== Loop Variant 5: for loop with complex counter ========== */
NOINLINE
int loop_variant5(int bound) {
    int sum = 0;
    /* Counter in register, decrement by 1 */
    register int counter = bound;
    for (; counter != 0; counter--) {
        sum += counter;
        /* Call external function to prevent optimization */
        sum ^= rand() & 0xFF;
    }
    g_side_effect += sum;
    return sum;
}

/* ========== Loop Variant 6: nested loops to create complexity ========== */
NOINLINE
int loop_variant6(int bound) {
    int sum = 0;
    int outer = bound / 10;
    if (outer == 0) outer = 1;
    
    for (int j = 0; j < outer; j++) {
        /* Inner loop with decrementing counter */
        int inner = bound;
        while (inner-- != 0) {
            sum += (j * inner);
            g_array[(j * inner) % 10000] = sum;
        }
    }
    g_side_effect ^= sum;
    return sum;
}

/* ========== Main function ========== */
int main(int argc, char *argv[]) {
    int bound;
    
    /* Make bound non-constant at compile time */
    if (argc > 1) {
        bound = atoi(argv[1]);
        if (bound <= 0) bound = 1000;
    } else {
        /* Use volatile to prevent constant propagation */
        bound = g_volatile_bound;
    }
    
    /* Initialize array */
    memset(g_array, 0, sizeof(g_array));
    
    /* Run all loop variants */
    int results[6];
    results[0] = loop_variant1(bound);
    results[1] = loop_variant2(bound);
    results[2] = loop_variant3(bound);
    results[3] = loop_variant4(bound);
    results[4] = loop_variant5(bound);
    results[5] = loop_variant6(bound);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum ^= results[i];
        printf("Loop variant %d: result = %d\n", i + 1, results[i]);
    }
    
    /* Use side effects */
    checksum ^= g_side_effect;
    checksum ^= g_array[bound % 10000];
    
    printf("Final checksum: %d\n", checksum);
    printf("Side effect accumulator: %d\n", g_side_effect);
    
    return checksum != 0 ? 0 : 1;
}
