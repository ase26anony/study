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
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
  #define ARCH_PPC 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define DOLOOP_SUPPORTED 1
#else
  #define DOLOOP_SUPPORTED 0
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* Use volatile to prevent constant propagation */
    return limit;
}

/* Variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3; /* Prevent dead code elimination */
        /* Use asm to prevent optimization without affecting RTL pattern */
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
    
    /* Pattern: for (i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Variant 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (i = limit; i != 0; i = i - 1) */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1; /* Explicit subtraction by 1 */
    }
    
    return sum;
}

/* Variant 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
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

/* Variant 6: Count down to zero with unsigned counter */
NO_OPT static int test_unsigned_decrement(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) with unsigned */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Variant 7: Complex expression in compare */
NO_OPT static int test_complex_compare(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while ((n = n - 1) >= 0) */
    while (__builtin_expect((n = n - 1) >= 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Architecture-specific test functions */
#if defined(ARCH_ARM)
/* ARM-specific: Use register variables to encourage specific RTL patterns */
NO_OPT static int test_arm_specific(int limit) {
    register int n asm("r4") = limit; /* Suggest specific register */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        /* ARM-specific asm to prevent optimization */
        asm volatile("add %0, %0, #23" : "+r"(sum) : : "cc");
    }
    
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific: Use 8-bit counters which are common for doloop */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 29;
        /* AVR-specific nop to prevent optimization */
        asm volatile("nop" : : :);
    }
    
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop pattern matching with limit = %d\n", loop_limit);
    printf("Doloop supported on this architecture: %s\n", 
           DOLOOP_SUPPORTED ? "YES" : "NO");
    
    /* Execute all test variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_unsigned_decrement(loop_limit);
    total_sum += test_complex_compare(loop_limit);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total_sum += test_arm_specific(loop_limit);
    printf("ARM-specific test executed\n");
#endif
    
#if defined(ARCH_AVR)
    total_sum += test_avr_specific(loop_limit);
    printf("AVR-specific test executed\n");
#endif
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return total_sum == 0 ? 1 : 0;
}
