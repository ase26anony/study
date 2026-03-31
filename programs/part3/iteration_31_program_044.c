/* test-doloop-pattern.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
void test_do_while_predec(unsigned int n) {
    unsigned int counter = n;
    do {
        global_sum += counter;  /* Simple body to avoid elimination */
    } while (--counter != 0);
}

/* Variant 2: while with post-decrement */
void test_while_postdec(int n) {
    int counter = n;
    while (counter-- != 0) {
        global_sum += counter;
    }
}

/* Variant 3: nested loops - inner loop uses decrement pattern */
void test_nested_loops(unsigned int outer, unsigned int inner) {
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        do {
            global_sum += i + counter;
        } while (--counter != 0);
    }
}

/* Variant 4: mixed signed/unsigned types */
void test_mixed_types(int n) {
    unsigned int counter = (unsigned int)n;
    while (counter != 0) {
        global_sum += 1;
        counter--;
    }
}

/* Variant 5: complex condition but still decrement pattern */
void test_complex_condition(unsigned int n) {
    unsigned int counter = n;
    do {
        global_sum += (counter & 0xFF);
        /* The decrement and compare should still be recognizable */
    } while (--counter != 0);
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
    test_nested_loops(5, base_bound / 5);
    test_mixed_types(base_bound);
    test_complex_condition(base_bound * 2);
    
    printf("Result: %d\n", global_sum);
    return 0;
}
