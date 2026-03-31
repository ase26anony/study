/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection for doloop support */
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

/* Variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body that can't be optimized away */
        sum += 0x1234;
        /* Prevent compiler from moving operations across iterations */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; i != 0; i = i - 1) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 0x2468;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 1, 1));
    }
    return sum;
}

/* Variant 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 1000;
    int loop_limit;
    
    /* Use different methods to get non-constant values */
#if DOLOOP_SUPPORTED
    /* For doloop-supported architectures, use a value that can't be constant-folded */
    loop_limit = base_limit + (getpid() & 0x3F);  /* Add some entropy */
#else
    /* For other architectures, still use a non-constant value */
    loop_limit = base_limit + (clock() & 0x1F);
#endif
    
    printf("Testing doloop pattern matching with limit = %d\n", loop_limit);
    printf("Doloop supported on this architecture: %s\n", 
           DOLOOP_SUPPORTED ? "YES" : "NO");
    
    /* Execute all test variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    
    printf("Total sum from all loops: %d (0x%08x)\n", total_sum, total_sum);
    
    /* Return the sum to prevent dead code elimination */
    return total_sum == 0 ? 1 : 0;
}

/* Additional architecture-specific variants for maximum coverage */
#if defined(ARCH_ARM)
/* ARM-specific: Use register variables to encourage specific register allocation */
NO_OPT static int test_arm_specific(int limit) {
    register int n asm("r4") = limit;  /* Suggest register r4 */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0xDEAD;
        asm volatile("" : "+r"(sum) : "r"(n) : "memory");
    }
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific: Use 8-bit counters which are common for doloop */
NO_OPT static int test_avr_specific(uint8_t limit) {
    uint8_t n = limit;
    int sum = 0;
    
    do {
        sum += 0xBE;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 0, 1));
    
    return sum;
}
#endif
