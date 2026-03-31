/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection for doloop support */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM) || defined(__thumb__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "ARM"
#elif defined(__AVR__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "PowerPC"
#elif defined(__mips__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "MIPS"
#else
#define ARCH_SUPPORTS_DOLOOP 0
#define ARCH_NAME "Generic"
#endif

/* Variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 2: Pre-decrement in condition - while (--n > 0) */
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

/* Variant 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i != 0; i = i - 1) */
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    /* Pattern: do { ... } while (n-- > 0) */
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 0, 1));
    
    return sum;
}

/* Variant 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n != 0) { n = n - 1; ... } */
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;
    }
    return sum;
}

/* Variant 7: Complex pattern with multiple decrements */
NO_OPT static int test_complex_pattern(int limit) {
    int n = limit;
    int m = limit / 2;
    int sum = 0;
    
    /* Nested loops to create more complex patterns */
    while (__builtin_expect(n-- > 0, 1)) {
        int inner = m;
        while (__builtin_expect(inner-- > 0, 1)) {
            sum += 19;
            asm volatile("" : "+r"(sum) : : "memory");
        }
    }
    return sum;
}

/* Get a non-constant loop limit to prevent constant propagation */
static int get_loop_limit(void) {
    volatile int limit;
    
#if ARCH_SUPPORTS_DOLOOP
    /* Use architecture-specific methods to get non-constant values */
    #if defined(__arm__)
        asm volatile("mov %0, #100" : "=r"(limit));
    #elif defined(__AVR__)
        limit = 50;  /* Smaller for AVR */
    #elif defined(__powerpc__)
        limit = 75;
    #elif defined(__mips__)
        limit = 80;
    #else
        limit = 100;
    #endif
#else
    /* Generic fallback - use system call or volatile */
    limit = getpid() & 0x7F;  /* Limit to 127 for safety */
    if (limit < 10) limit = 10;
#endif
    
    return limit;
}

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop optimization on %s architecture\n", ARCH_NAME);
    
    /* Get a non-constant loop limit */
    loop_limit = get_loop_limit();
    printf("Using loop limit: %d\n", loop_limit);
    
    /* Execute all test variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_complex_pattern(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (all should succeed) */
    return (total_sum > 0) ? 0 : 1;
}
