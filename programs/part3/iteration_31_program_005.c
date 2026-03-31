/* test-loop-doloop.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c */

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

/* Variant 3: nested loops - inner loop should generate pattern */
void test_nested_loops(unsigned int outer, unsigned int inner) {
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        do {
            global_sum += i * counter;
        } while (--counter != 0);
    }
}

/* Variant 4: mixed signed/unsigned */
void test_mixed_types(int n) {
    unsigned int counter = (unsigned int)n;
    while (counter != 0) {
        global_sum += 1;
        counter--;
    }
}

/* Variant 5: complex but still decrement pattern */
void test_complex_decrement(unsigned int n) {
    unsigned int counter = n;
    unsigned int temp = 0;
    
    do {
        temp += counter;
        global_sum += temp;
    } while (--counter != 0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_count>\n", argv[0]);
        return 1;
    }
    
    int base_count = atoi(argv[1]);
    if (base_count <= 0) base_count = 100;
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all variants with different counts */
    test_do_while_predec(base_count);
    test_while_postdec(base_count / 2);
    test_nested_loops(5, base_count / 5);
    test_mixed_types(base_count / 3);
    test_complex_decrement(base_count / 4);
    
    printf("Final sum: %d\n", global_sum);
    return 0;
}
