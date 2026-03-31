#include <stdio.h>
#include <stdlib.h>

// Prevent inlining and interprocedural optimizations
__attribute__((noinline, noipa))
int test_for_loop(int limit) {
    int sum = 0;
    // Classic for loop with decrementing counter compared to zero
    for (int i = limit; i > 0; i--) {
        sum += i;  // Simple non-empty body
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_loop(int limit) {
    int sum = 0;
    int i = limit;
    // While loop with explicit decrement and zero comparison
    while (i > 0) {
        sum += i;
        i--;
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_while_postdecrement(int limit) {
    int sum = 0;
    int i = limit;
    // Alternative while pattern with post-decrement
    while (i-- > 0) {
        sum += i + 1;  // Adjust for post-decrement
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_do_while_with_check(int limit) {
    int sum = 0;
    int i = limit;
    // do-while with pre-check (should still trigger pattern matching)
    if (i > 0) {
        do {
            sum += i;
        } while (--i > 0);
    }
    return sum;
}

__attribute__((noinline, noipa))
int test_unsigned_counter(unsigned int limit) {
    unsigned int sum = 0;
    // Using unsigned counter with != 0 comparison
    for (unsigned int i = limit; i != 0; i--) {
        sum += i;
    }
    return (int)sum;
}

int main() {
    volatile int iterations = 100;  // Prevent constant propagation
    int total_sum = 0;
    
    // Call all test functions with the volatile-bound iteration count
    total_sum += test_for_loop(iterations);
    total_sum += test_while_loop(iterations);
    total_sum += test_while_postdecrement(iterations);
    total_sum += test_do_while_with_check(iterations);
    total_sum += test_unsigned_counter((unsigned int)iterations);
    
    printf("Total sum: %d\n", total_sum);
    
    // Also test with random value to prevent pattern recognition
    srand(42);
    int random_iter = rand() % 1000 + 1;
    printf("Random test sum: %d\n", test_for_loop(random_iter));
    
    return total_sum > 0 ? 0 : 1;  // Ensure all loops executed
}
