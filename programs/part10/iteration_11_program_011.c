/* loop-doloop-test.c
 * Test program for GCC's doloop optimization pattern matching
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
  #define ARCH_ARM 1
  #define ARCH_NAME "ARM"
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
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
    /* Use volatile to prevent constant propagation */
    static volatile int base_limit = 1000;
    /* Mix with something runtime-dependent */
    return base_limit + (getpid() & 0xFF);
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

/* Loop variant 4: Explicit subtract */
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

/* Loop variant 5: Do-while with post-decrement */
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

/* Loop variant 6: Unsigned counter */
NO_OPT static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Counter in register with volatile */
NO_OPT static int test_volatile_counter(int limit) {
    register int counter asm("r0") = limit;
    int sum = 0;
    
    /* Force counter to stay in register */
    asm volatile("" : "+r"(counter));
    
    while (__builtin_expect(counter-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific optimizations */
#if defined(ARCH_ARM)
/* ARM-specific: Use doloop-friendly patterns */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often optimizes: subs + bne pattern */
    do {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(--n != 0, 1));
    
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific: 8-bit counters for doloop */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (limit > 255) ? 255 : limit;
    int sum = 0;
    
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

int main(void) {
    int total = 0;
    int loop_limit;
    
    printf("Testing doloop optimization on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    printf("Loop limit: %d\n", loop_limit);
    
    /* Test all loop variants */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while(loop_limit);
    total += test_unsigned_decrement((unsigned int)loop_limit);
    total += test_volatile_counter(loop_limit);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total += test_arm_specific(loop_limit);
    printf("ARM-specific test included\n");
#endif
    
#if defined(ARCH_AVR)
    total += test_avr_specific(loop_limit);
    printf("AVR-specific test included\n");
#endif
    
    printf("Total sum: %d\n", total);
    
    /* Return non-zero to indicate success and prevent optimization */
    return (total > 0) ? 0 : 1;
}
