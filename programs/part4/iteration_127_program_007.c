/* loop-doloop-coverage.c
 * 
 * This program is designed to exercise the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) which checks for the pattern:
 *   (reg - 1) != 0
 * in loop exit conditions.
 *
 * Compile with: gcc -O2 -fdump-rtl-doloop -fdump-rtl-loop2 loop-doloop-coverage.c
 * Or for more detailed analysis: gcc -O3 -funroll-loops -fdump-rtl-all loop-doloop-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent constant propagation and loop unrolling */
static volatile int g_volatile_bound = 1000;

/* External function to prevent optimization */
extern int rand(void);

/* ========== Loop Variant 1: for loop with i-- != 0 ========== */
__attribute__((noinline, noipa))
int loop_decrement_for(int bound) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Pattern: for (i = bound; i != 0; i--) */
    for (int i = bound; i != 0; i--) {
        sum += rand() % 100;
        sink = sum;  /* Side effect to prevent dead code elimination */
    }
    
    return sum + sink;
}

/* ========== Loop Variant 2: while loop with --i != 0 ========== */
__attribute__((noinline, noipa))
int loop_decrement_while_pre(int bound) {
    volatile int sink = 0;
    int sum = 0;
    int i = bound;
    
    /* Pattern: while (--i != 0) */
    while (--i != 0) {
        sum += rand() % 100;
        sink = sum;
    }
    
    return sum + sink + i;
}

/* ========== Loop Variant 3: while loop with i-- != 0 ========== */
__attribute__((noinline, noipa))
int loop_decrement_while_post(int bound) {
    volatile int sink = 0;
    int sum = 0;
    int i = bound;
    
    /* Pattern: while (i-- != 0) */
    while (i-- != 0) {
        sum += rand() % 100;
        sink = sum;
    }
    
    return sum + sink + i;
}

/* ========== Loop Variant 4: do-while with explicit check ========== */
__attribute__((noinline, noipa))
int loop_decrement_dowhile(int bound) {
    volatile int sink = 0;
    int sum = 0;
    int i = bound;
    
    if (i <= 0) return 0;
    
    /* Pattern: do { ... } while (--i != 0); */
    do {
        sum += rand() % 100;
        sink = sum;
    } while (--i != 0);
    
    return sum + sink;
}

/* ========== Loop Variant 5: Nested loops to complicate analysis ========== */
__attribute__((noinline, noipa))
int loop_decrement_nested(int bound) {
    volatile int sink = 0;
    int sum = 0;
    int outer = bound / 10;
    
    for (int j = 0; j < outer; j++) {
        int inner = bound;
        /* Inner loop with the target pattern */
        while (inner-- != 0) {
            sum += (j * inner) & 0xFF;
            sink = sum;
        }
    }
    
    return sum + sink;
}

/* ========== Loop Variant 6: Unsigned counter ========== */
__attribute__((noinline, noipa))
int loop_decrement_unsigned(unsigned int bound) {
    volatile int sink = 0;
    int sum = 0;
    unsigned int i = bound;
    
    /* Pattern with unsigned: while (i-- != 0) */
    while (i-- != 0) {
        sum += rand() % 100;
        sink = sum;
    }
    
    return sum + sink;
}

/* ========== Loop Variant 7: Counter in separate function ========== */
static int helper_counter = 0;
__attribute__((noinline, noipa))
int loop_decrement_global(int bound) {
    volatile int sink = 0;
    int sum = 0;
    
    helper_counter = bound;
    /* Pattern: while (helper_counter-- != 0) */
    while (helper_counter-- != 0) {
        sum += rand() % 100;
        sink = sum;
    }
    
    return sum + sink;
}

/* ========== Main function ========== */
int main(int argc, char *argv[]) {
    int bound;
    
    /* Make bound non-constant to prevent unrolling */
    if (argc > 1) {
        bound = atoi(argv[1]);
        if (bound <= 0) bound = 1000;
    } else {
        bound = g_volatile_bound;
    }
    
    printf("Testing doloop pattern with bound = %d\n", bound);
    
    /* Call all loop variants to increase coverage probability */
    int results[7];
    
    results[0] = loop_decrement_for(bound);
    results[1] = loop_decrement_while_pre(bound);
    results[2] = loop_decrement_while_post(bound);
    results[3] = loop_decrement_dowhile(bound);
    results[4] = loop_decrement_nested(bound);
    results[5] = loop_decrement_unsigned((unsigned int)bound);
    results[6] = loop_decrement_global(bound);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum ^= results[i];
        printf("Loop %d result: %d\n", i, results[i]);
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
