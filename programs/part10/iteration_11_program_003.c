/* loop-doloop-test.c
 * Test program for GCC's doloop optimization pass
 * Targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

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

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that won't be optimized away */
        /* Prevent loop unrolling with a small asm barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 1, 1));
    }
    
    return sum;
}

/* Loop variant 6: Unsigned counter (common in embedded systems) */
NO_OPT static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 7: Counter in register with volatile to force reload */
NO_OPT static int test_volatile_counter(int limit) {
    register int counter asm("r0") = limit;  /* Suggest register for counter */
    int sum = 0;
    
    while (__builtin_expect(counter-- > 0, 1)) {
        sum += 19;
        /* Force counter to be reloaded from register */
        asm volatile("" : "+r"(counter), "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Architecture-specific test functions */
#if defined(ARCH_ARM)
/* ARM-specific: Use register constraints that might trigger doloop */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use inline asm to force specific register usage */
    asm volatile("" : "+r"(n) : : "cc");
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific: 8-bit counters often trigger doloop */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);  /* 8-bit counter */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}
#endif

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop optimization on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    
    /* Execute all test functions */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_volatile_counter(loop_limit);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total_sum += test_arm_specific(loop_limit);
    printf("ARM-specific test included\n");
#endif
    
#if defined(ARCH_AVR)
    total_sum += test_avr_specific(loop_limit);
    printf("AVR-specific test included\n");
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return total_sum == 0 ? 1 : 0;
}
