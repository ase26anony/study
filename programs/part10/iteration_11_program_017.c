/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

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
  #define ARCH_GENERIC 1
  #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Test function 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 0x1234;  /* Arbitrary constant */
        /* Prevent loop body from being optimized away */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Test function 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Copy to local to ensure we don't modify the parameter directly */
    int counter = n;
    
    while (__builtin_expect(--counter > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Test function 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Test function 4: Explicit subtract pattern - i = i - 1 */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- or --i */
    }
    
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 0x2468;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Test function 6: Unsigned counter (common in doloop patterns) */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Test function 7: Counter in register variable (hint to compiler) */
NO_OPT static int test_register_counter(int limit) {
    register int counter asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(counter-- > 0, 1)) {
        sum += 0xFACE;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Test function 8: Complex but valid decrement pattern */
NO_OPT static int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This might generate PLUS with -1 in different forms */
    while (__builtin_expect((n--) != 0, 1)) {
        sum += 0xBEEF;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    int total = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;  /* volatile prevents compile-time computation */
    int loop_limit = base_limit;
    
    /* Alternative: use system call for truly non-constant value */
    #if ARCH_SUPPORTS_DOLOOP
    int pid_limit = getpid() & 0xFF;  /* Non-constant, but small enough */
    if (pid_limit < 10) pid_limit = 100;  /* Ensure minimum iterations */
    loop_limit = pid_limit;
    #endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    printf("Architecture: ");
    
    #ifdef ARCH_ARM
    printf("ARM\n");
    #elif defined(ARCH_AVR)
    printf("AVR\n");
    #elif defined(ARCH_PPC)
    printf("PowerPC\n");
    #elif defined(ARCH_MIPS)
    printf("MIPS\n");
    #else
    printf("Generic\n");
    #endif
    
    /* Run all test functions */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_loop_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while(loop_limit);
    total += test_unsigned_decrement((unsigned int)loop_limit);
    total += test_register_counter(loop_limit);
    total += test_complex_decrement(loop_limit);
    
    printf("Total accumulator value: %d\n", total);
    
    /* Return something based on the result to prevent dead code elimination */
    return (total > 0) ? 0 : 1;
}

/* Additional architecture-specific variants */
#if defined(ARCH_ARM)
/* ARM-specific test with inline assembly to ensure RTL patterns */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subtract and compare instructions */
    while (n--) {
        sum += 0xCAFE;
        /* ARM-specific assembly barrier */
        asm volatile("" : "+r"(sum) : : "cc", "memory");
    }
    
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific - often uses 8-bit counters for doloop */
NO_OPT static uint8_t test_avr_specific(uint8_t limit) {
    uint8_t n = limit;
    uint8_t sum = 0;
    
    while (n--) {
        sum += 0x37;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}
#endif
