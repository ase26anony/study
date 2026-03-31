/* test-doloop-pattern.c
 * Target: PowerPC (or other architecture with condition code registers)
 * Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;

/* Variant 1: do-while with pre-decrement */
void test_do_while_predec(unsigned int n) {
    unsigned int counter = n;
    do {
        global_sum += counter;  /* Simple non-empty body */
    } while (--counter != 0);
}

/* Variant 2: while with post-decrement */
void test_while_postdec(int n) {
    int counter = n;
    while (counter-- != 0) {
        global_sum += counter;
    }
}

/* Variant 3: nested loops - inner loop has decrement pattern */
void test_nested_loops(unsigned int outer, unsigned int inner) {
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        do {
            global_sum += i * counter;
        } while (--counter != 0);
    }
}

/* Variant 4: mixed signed/unsigned counters */
void test_mixed_types(int n) {
    /* Force use of different comparison patterns */
    unsigned int ucounter = (unsigned int)n;
    int scounter = n;
    
    /* First loop: unsigned decrement */
    while (ucounter-- != 0) {
        global_sum += 1;
    }
    
    /* Second loop: signed decrement */
    do {
        global_sum -= 1;
    } while (--scounter != 0);
}

/* Variant 5: complex control flow with decrement */
void test_complex_control(unsigned int n) {
    unsigned int counter = n;
    unsigned int temp = 0;
    
    while (counter != 0) {
        temp += counter;
        counter--;  /* Decrement in body, but condition checks counter != 0 */
        
        /* Small conditional to prevent over-optimization */
        if (temp > 1000) {
            temp = 0;
        }
    }
    global_sum += temp;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_bound>\n", argv[0]);
        return 1;
    }
    
    int base_bound = atoi(argv[1]);
    if (base_bound <= 0) base_bound = 100;
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all variants with different bounds derived from input */
    test_do_while_predec(base_bound);
    test_while_postdec(base_bound / 2);
    test_nested_loops(base_bound / 10, base_bound / 5);
    test_mixed_types(base_bound / 3);
    test_complex_control(base_bound);
    
    /* Print predictable result for verification */
    printf("Result: %d\n", global_sum);
    
    /* Also compute expected value for quick verification */
    int expected = 0;
    int n = base_bound;
    
    /* Manually compute what test_do_while_predec should produce */
    for (unsigned int i = n; i > 0; --i) expected += i;
    
    /* Add other contributions... (simplified) */
    printf("Expected minimum: %d\n", expected);
    
    return 0;
}
