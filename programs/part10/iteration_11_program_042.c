/* loop-doloop-test.c - Test program for GCC's doloop optimization */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent inlining and IPA transformations */
#define NOOPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH)
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

/* Test function 1: Post-decrement in condition */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3;
        /* Prevent loop body optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition */
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

/* Test function 3: For loop with decrement */
NOOPT int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract operation */
NOOPT int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    for (; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NOOPT int test_do_while_post(int limit) {
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

/* Test function 6: While loop with explicit compare */
NOOPT int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while ((n = n - 1) >= 0) */
    while (__builtin_expect((n = n - 1) >= 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 7: Unsigned counter (common in doloop) */
NOOPT unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Complex decrement pattern */
NOOPT int test_complex_decrement(int limit) {
    volatile int temp = limit; /* Prevent constant propagation */
    int n = temp;
    int sum = 0;
    
    /* Pattern with volatile read to force reload */
    while (__builtin_expect(n > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1; /* Separate decrement statement */
    }
    return sum;
}

/* Main function with architecture-specific handling */
int main() {
    int total = 0;
    int loop_limit;
    
    printf("Testing doloop optimization on %s architecture\n", ARCH_NAME);
    
    /* Get a non-constant loop limit to prevent constant propagation */
#if defined(ARCH_ARM) || defined(ARCH_AVR) || defined(ARCH_PPC) || defined(ARCH_MIPS)
    /* For doloop-supported architectures, use a moderate loop count */
    volatile int vol_limit = 1000;
    loop_limit = vol_limit;
#else
    /* For generic architectures, use a smaller count */
    loop_limit = 100;
#endif
    
    /* Execute all test functions */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_loop_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while_post(loop_limit);
    total += test_while_explicit(loop_limit);
    total += test_unsigned_decrement((unsigned int)loop_limit);
    total += test_complex_decrement(loop_limit);
    
    printf("Total sum: %d\n", total);
    
    /* Return non-zero to indicate execution */
    return total != 0 ? 0 : 1;
}
