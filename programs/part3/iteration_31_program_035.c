/* test-doloop-pattern.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
void test_do_while_predec(unsigned int n) {
    unsigned int counter = n;
    do {
        global_sum += counter * 2;
    } while (--counter != 0);
}

/* Variant 2: while with post-decrement */
void test_while_postdec(int n) {
    int counter = n;
    while (counter-- != 0) {
        global_sum += counter + 1;
    }
}

/* Variant 3: nested loops - inner loop with decrement pattern */
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
    unsigned int counter = (unsigned int)n;
    while (counter != 0) {
        global_sum += (int)counter;
        counter--;
    }
}

/* Variant 5: simple decrement in condition with different comparison */
void test_decrement_compare(unsigned int n) {
    unsigned int counter = n;
    do {
        global_sum += 1;
    } while (--counter > 0);  /* Should still generate similar pattern */
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_bound>\n", argv[0]);
        return 1;
    }
    
    int base_bound = atoi(argv[1]);
    if (base_bound <= 0) base_bound = 10;
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all variants with different bounds derived from input */
    test_do_while_predec(base_bound);
    test_while_postdec(base_bound + 1);
    test_nested_loops(3, base_bound);
    test_mixed_types(base_bound + 2);
    test_decrement_compare(base_bound + 3);
    
    printf("Result: %d\n", global_sum);
    return 0;
}
