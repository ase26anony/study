/* loop-doloop-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA to preserve loop structure */
__attribute__((noinline, noipa))
int test_for_loop(int limit) {
    int sum = 0;
    /* Classic decrementing for loop compared to zero */
    for (int i = limit; i > 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* While loop with decrement in condition */
    while (i-- > 0) {
        sum += (i + 1);  /* Adjust for post-decrement */
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    /* Do-while with pre-check - may already be optimal but still tested */
    if (i > 0) {
        do {
            sum += i;
        } while (--i > 0);
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_unsigned_loop(unsigned int limit) {
    unsigned int sum = 0;
    /* Unsigned version - different type but same pattern */
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

__attribute__((noinline, noipa))
int test_nested_loop(int limit) {
    int sum = 0;
    /* Nested loop to test pattern in inner loop */
    for (int i = limit; i > 0; i--) {
        for (int j = 5; j > 0; j--) {
            sum += i * j;
        }
    }
    return sum;
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int iterations = 100;
    int total = 0;
    
    /* Call all test functions */
    total += test_for_loop(iterations);
    total += test_while_loop(iterations);
    total += test_do_while_loop(iterations);
    total += test_unsigned_loop((unsigned int)iterations);
    total += test_nested_loop(iterations / 10);
    
    printf("Total sum: %d\n", total);
    
    /* Also test with random value to avoid compile-time optimization */
    srand(42);
    int random_iter = rand() % 1000 + 100;
    printf("Random test: %d\n", test_for_loop(random_iter));
    
    return total > 0 ? 0 : 1;
}
