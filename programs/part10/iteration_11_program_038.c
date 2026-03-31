/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent inlining and IPA transformations */
#define NOOPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
#define ARCH_ARM 1
#define ARCH_NAME "ARM"
#elif defined(__AVR__)
#define ARCH_AVR 1
#define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__)
#define ARCH_PPC 1
#define ARCH_NAME "PowerPC"
#elif defined(__mips__)
#define ARCH_MIPS 1
#define ARCH_NAME "MIPS"
#else
#define ARCH_GENERIC 1
#define ARCH_NAME "Generic"
#endif

/* Get a non-constant loop limit to prevent constant propagation */
static volatile int get_loop_limit(void) {
    static volatile int seed = 0;
    return 1000 + (seed++ & 0xFF); /* Non-constant but bounded */
}

/* Loop variant 1: Post-decrement in condition */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;
        /* Prevent optimization of loop body */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NOOPT int test_pre_decrement(int limit) {
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
NOOPT int test_for_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NOOPT int test_explicit_subtract(int limit) {
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
NOOPT int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    /* Pattern: do { ... } while (--n > 0) */
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(--n > 0, 1));
    
    return sum;
}

/* Loop variant 6: Unsigned counter */
NOOPT int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Complex decrement pattern */
NOOPT int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern that might generate PLUS with -1 */
    while (__builtin_expect(n > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n + (-1);  /* Alternative form of decrement */
    }
    return sum;
}

/* Architecture-specific test harness */
#if defined(ARCH_ARM) || defined(ARCH_AVR) || defined(ARCH_PPC) || defined(ARCH_MIPS)
/* Optimized test for architectures with doloop support */
NOOPT int run_architecture_tests(void) {
    volatile int base_limit = get_loop_limit();
    int total = 0;
    
    printf("Running doloop optimization tests on %s architecture\n", ARCH_NAME);
    printf("Base loop limit: %d\n", base_limit);
    
    /* Run all loop variants */
    total += test_post_decrement(base_limit);
    total += test_pre_decrement(base_limit);
    total += test_for_decrement(base_limit);
    total += test_explicit_subtract(base_limit);
    total += test_do_while(base_limit);
    total += test_unsigned_counter((unsigned int)base_limit);
    total += test_complex_decrement(base_limit);
    
    return total;
}
#else
/* Fallback for generic architectures */
NOOPT int run_architecture_tests(void) {
    volatile int base_limit = get_loop_limit();
    int total = 0;
    
    printf("Running generic loop tests (no doloop optimization expected)\n");
    printf("Base loop limit: %d\n", base_limit);
    
    /* Still run tests to exercise the code */
    total += test_post_decrement(base_limit);
    total += test_pre_decrement(base_limit);
    total += test_for_decrement(base_limit);
    
    return total;
}
#endif

int main(void) {
    int result = run_architecture_tests();
    
    printf("Total accumulator result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result > 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Warning: No loop iterations performed.\n");
        return 1;
    }
}
