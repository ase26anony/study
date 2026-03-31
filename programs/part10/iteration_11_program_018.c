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
  #define DOLOOP_SUPPORTED 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
  #define ARCH_PPC 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define DOLOOP_SUPPORTED 1
#else
  #define DOLOOP_SUPPORTED 0
#endif

/* Test function 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;
        /* Prevent loop unrolling */
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
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
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
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- or --i */
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    /* Pattern: do { ... } while (n-- > 0); */
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 0, 1));
    
    return sum;
}

/* Test function 6: Unsigned counter (common in embedded) */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int i asm("r0") = limit;  /* Suggest register for counter */
    int sum = 0;
    
    /* Pattern: while (i-- > 0) with register variable */
    while (__builtin_expect(i-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Nested loops to stress pattern matching */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer = outer_limit;
    int sum = 0;
    
    while (__builtin_expect(outer-- > 0, 1)) {
        int inner = inner_limit;
        
        /* Inner loop with different pattern */
        for (int j = inner; __builtin_expect(j > 0, 1); j--) {
            sum += 23;
            asm volatile("" : "+r"(sum) : : "memory");
        }
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total = 0;
    
    /* Get non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int limit = base_limit;
    
    /* Use system call to add more variability if needed */
    #if DOLOOP_SUPPORTED
    limit += getpid() & 0xF;  /* Small variation based on PID */
    #endif
    
    printf("Testing doloop patterns with limit = %d\n", limit);
    
    /* Run all test functions */
    total += test_post_decrement(limit);
    total += test_pre_decrement(limit);
    total += test_for_loop_decrement(limit);
    total += test_explicit_subtract(limit);
    total += test_do_while_post(limit);
    total += test_unsigned_decrement((unsigned int)limit);
    total += test_register_counter(limit);
    total += test_nested_loops(limit / 10, 10);
    
    printf("Total sum from all loops: %d\n", total);
    
    /* Return non-zero to indicate success and prevent optimization */
    return total != 0 ? 0 : 1;
}

/* Architecture-specific variants */
#if defined(ARCH_ARM)
/* ARM-specific loop patterns that might generate different RTL */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subtract-and-compare instructions */
    while (n--) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific - often uses 8-bit counters */
NO_OPT static uint8_t test_avr_specific(uint8_t limit) {
    uint8_t n = limit;
    uint8_t sum = 0;
    
    do {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (n--);
    
    return sum;
}
#endif
