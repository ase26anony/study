/* loop-doloop-test.c
 * Test program to trigger GCC's doloop optimization pattern matching
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
#define ARCH_ARM 1
#define ARCH_NAME "ARM"
#elif defined(__AVR__)
#define ARCH_AVR 1
#define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
#define ARCH_PPC 1
#define ARCH_NAME "PowerPC"
#elif defined(__mips__)
#define ARCH_MIPS 1
#define ARCH_NAME "MIPS"
#else
#define ARCH_GENERIC 1
#define ARCH_NAME "Generic"
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    static volatile int seed = 0;
    /* Use volatile to prevent constant propagation */
    seed = seed + 1;
    return 100 + (seed & 0xFF);
}

/* Loop variant 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract by 1 */
    }
    return sum;
}

/* Loop variant 5: Do-while with decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: do { ... } while (n-- > 0) */
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    return sum;
}

/* Loop variant 6: Unsigned counter (common in doloop) */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int i asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    /* Pattern: while (i-- > 0) */
    while (__builtin_expect(i-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 8: Nested loops to stress pattern matching */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer = outer_limit;
    int total = 0;
    
    while (__builtin_expect(outer-- > 0, 1)) {
        int inner = inner_limit;
        while (__builtin_expect(inner-- > 0, 1)) {
            total += 23;
            asm volatile("" : "+r"(total) : : "memory");
        }
    }
    return total;
}

/* Main test driver */
int main(void) {
    int total_result = 0;
    int loop_limit;
    
    printf("Testing doloop optimization on %s architecture\n", ARCH_NAME);
    printf("Targeting GCC loop-doloop.cc lines 136-150\n\n");
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    printf("Using loop limit: %d\n", loop_limit);
    
    /* Execute all test variants */
    printf("\nRunning loop variants:\n");
    
    printf("1. Post-decrement: ");
    int r1 = test_post_decrement(loop_limit);
    printf("result = %d\n", r1);
    total_result += r1;
    
    printf("2. Pre-decrement: ");
    int r2 = test_pre_decrement(loop_limit);
    printf("result = %d\n", r2);
    total_result += r2;
    
    printf("3. For loop decrement: ");
    int r3 = test_for_decrement(loop_limit);
    printf("result = %d\n", r3);
    total_result += r3;
    
    printf("4. Explicit subtract: ");
    int r4 = test_explicit_subtract(loop_limit);
    printf("result = %d\n", r4);
    total_result += r4;
    
    printf("5. Do-while: ");
    int r5 = test_do_while(loop_limit);
    printf("result = %d\n", r5);
    total_result += r5;
    
    printf("6. Unsigned counter: ");
    int r6 = test_unsigned_counter((unsigned int)loop_limit);
    printf("result = %d\n", r6);
    total_result += r6;
    
    printf("7. Register counter: ");
    int r7 = test_register_counter(loop_limit);
    printf("result = %d\n", r7);
    total_result += r7;
    
    printf("8. Nested loops: ");
    int r8 = test_nested_loops(loop_limit / 10, 10);
    printf("result = %d\n", r8);
    total_result += r8;
    
    printf("\nTotal result: %d\n", total_result);
    
    /* Return non-zero to indicate success with all loops executed */
    return total_result != 0 ? 0 : 1;
}
