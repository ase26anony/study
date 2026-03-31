/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
  #define ARCH_ARM 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
  #define ARCH_PPC 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define ARCH_SUPPORTS_DOLOOP 1
#else
  #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Test function 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 0x1234;  /* Arbitrary constant */
        /* Prevent loop body from being optimized away */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract in loop */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    while (__builtin_expect(i != 0, 1)) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: do { ... } while (n-- > 0) */
    if (n > 0) {
        do {
            sum += 0x2468;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    return sum;
}

/* Test function 6: While loop with explicit comparison */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n) { ...; n--; } */
    while (__builtin_expect(n, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
        n--;
    }
    return sum;
}

/* Test function 7: Unsigned counter (common in doloop patterns) */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 0xFACE;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register for counter */
    int sum = 0;
    
    /* Pattern: while (n-- > 0) with register counter */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0xBEEF;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    /* Get non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int loop_limit;
    
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use a non-constant value */
    loop_limit = base_limit + (getpid() & 0xF);  /* Small variation */
    
    /* Architecture-specific optimizations */
#if defined(ARCH_ARM)
    /* ARM-specific: ensure we're in a mode that supports doloop */
    asm volatile(".arch armv7-a\n\t"
                 ".syntax unified\n\t" : : : "memory");
#elif defined(ARCH_AVR)
    /* AVR-specific: small loops work best */
    loop_limit = 10 + (getpid() & 0x7);
#endif
    
    printf("Testing on doloop-supported architecture\n");
    printf("Loop limit: %d\n", loop_limit);
#else
    /* Fallback for non-doloop architectures */
    loop_limit = 50;
    printf("Testing on generic architecture (doloop may not apply)\n");
#endif
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    printf("Total sum: %d (0x%08x)\n", total_sum, total_sum);
    
    /* Return non-zero if any test returned zero (unlikely) */
    return total_sum == 0 ? 1 : 0;
}
