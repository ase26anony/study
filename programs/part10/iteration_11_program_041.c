/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

/* Architecture detection for doloop support */
#if defined(__arm__) || defined(__ARM_ARCH)
  #define ARCH_ARM 1
  #define ARCH_DOLOOP 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_DOLOOP 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
  #define ARCH_PPC 1
  #define ARCH_DOLOOP 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define ARCH_DOLOOP 1
#else
  #define ARCH_GENERIC 1
  #define ARCH_DOLOOP 0
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3; /* Simple arithmetic that can't be optimized away */
        asm volatile("" : "+r"(sum) : : "memory"); /* Prevent optimization */
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
    for (int i = limit; i > 0; i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    for (; i != 0; i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
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

/* Loop variant 6: While with explicit compare to zero */
NO_OPT static int test_while_compare_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n != 0) { n = n - 1; ... } */
    while (__builtin_expect(n != 0, 1)) {
        n = n - 1;  /* Explicit decrement separate from compare */
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Complex decrement pattern that might generate PLUS with -1 */
NO_OPT static int test_complex_decrement(int limit) {
    unsigned int n = (unsigned int)limit;
    unsigned int sum = 0;
    
    /* Pattern using unsigned to avoid signed overflow issues */
    while (__builtin_expect(n--, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return (int)sum;
}

/* Loop variant 8: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit; /* Suggest register for counter */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test functions */
#ifdef ARCH_ARM
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often has specific doloop instructions */
    asm volatile("" : "+r"(n) : : "memory");
    while (n--) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

#ifdef ARCH_AVR
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF); /* AVR often uses 8-bit counters */
    int sum = 0;
    
    /* AVR has specific decrement and branch instructions */
    do {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (n--);
    
    return sum;
}
#endif

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop pattern matching on ");
    
#if ARCH_DOLOOP
    #ifdef ARCH_ARM
    printf("ARM architecture\n");
    #elif defined(ARCH_AVR)
    printf("AVR architecture\n");
    #elif defined(ARCH_PPC)
    printf("PowerPC architecture\n");
    #elif defined(ARCH_MIPS)
    printf("MIPS architecture\n");
    #endif
#else
    printf("generic architecture (doloop may not be supported)\n");
#endif
    
    /* Execute all test functions */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_while_compare_zero(loop_limit);
    total_sum += test_complex_decrement(loop_limit);
    total_sum += test_register_counter(loop_limit);
    
#if ARCH_DOLOOP
    #ifdef ARCH_ARM
    total_sum += test_arm_specific(loop_limit);
    #elif defined(ARCH_AVR)
    total_sum += test_avr_specific(loop_limit);
    #endif
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return total_sum == 0 ? 1 : 0;
}
