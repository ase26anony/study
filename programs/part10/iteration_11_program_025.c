/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
  #define ARCH_ARM 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
  #define ARCH_PPC 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define DOLOOP_SUPPORTED 1
#else
  #define DOLOOP_SUPPORTED 0
#endif

/* Test function 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 0x1234;  /* Arbitrary constant to prevent optimization */
        /* Prevent counter aliasing */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pre-decrement pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 0x2468;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Test function 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    return sum;
}

/* Test function 7: Unsigned counter (common in doloop patterns) */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0xABCD;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Counter in register variable (hint to keep in register) */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Architecture-specific register hint */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0x3333;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test variations */
#if DOLOOP_SUPPORTED

/* ARM-specific: Use register constraints that might trigger doloop */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subtract-and-set-flags patterns */
    while (__builtin_expect((n--) != 0, 1)) {
        sum += 0x7777;
        /* ARM-specific asm to prevent optimization */
        asm volatile("" : "+r"(sum), "+r"(n) : : "cc", "memory");
    }
    return sum;
}

/* AVR-specific: Small loops common on AVR */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);  /* AVR often uses 8-bit counters */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0x11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

#endif /* DOLOOP_SUPPORTED */

/* Main function with architecture-aware execution */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Get non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int loop_limit;
    
    if (argc > 1) {
        loop_limit = atoi(argv[1]);
    } else {
        /* Use volatile and system call to prevent compile-time computation */
        loop_limit = base_limit + (getpid() & 0x3F);  /* 0-63 variation */
    }
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    printf("Doloop supported: %s\n", DOLOOP_SUPPORTED ? "YES" : "NO");
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit_zero(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    
#if DOLOOP_SUPPORTED
    #ifdef ARCH_ARM
    total_sum += test_arm_specific(loop_limit);
    #elif defined(ARCH_AVR)
    total_sum += test_avr_specific(loop_limit);
    #endif
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return value depends on results to ensure all loops execute */
    return (total_sum > 0) ? 0 : 1;
}
