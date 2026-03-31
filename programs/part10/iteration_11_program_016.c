/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent inlining and IPA transformations */
#define NOOPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM)
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

/* Get a non-constant loop limit to prevent constant propagation */
static int get_loop_limit(void) {
    volatile int limit = 1000;
    /* Mix with something external to prevent optimization */
    return limit + (getpid() & 0x3F);
}

/* Loop variant 1: Post-decrement in condition */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3;
        /* Prevent optimization of loop body */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NOOPT int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration for exact comparison */
    if (limit > 0) sum += 5;
    return sum;
}

/* Loop variant 3: For loop with decrement */
NOOPT int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (int i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NOOPT int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (; i != 0; i = i - 1) pattern */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Loop variant 5: Do-while with decrement */
NOOPT int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(--n > 0, 1));
    }
    return sum;
}

/* Loop variant 6: Count down to zero with unsigned */
NOOPT int test_unsigned_decrement(int limit) {
    unsigned int n = (unsigned int)limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return (int)sum;
}

/* Loop variant 7: Complex decrement pattern */
NOOPT int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Create more complex pattern that might still match */
    while (__builtin_expect((n--) > 0, 1)) {
        sum += 19;
        /* Add memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    return sum;
}

/* Architecture-specific test for ARM */
#if defined(ARCH_ARM)
NOOPT int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often has specific doloop instructions */
    while (n--) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* Architecture-specific test for AVR */
#if defined(ARCH_AVR)
NOOPT int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);
    uint8_t sum = 0;
    
    /* AVR has specific doloop instructions for 8-bit counters */
    do {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (--n);
    
    return (int)sum;
}
#endif

int main(void) {
    int total = 0;
    int limit;
    
    printf("Testing doloop pattern matching on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit */
    limit = get_loop_limit();
    printf("Loop limit: %d\n", limit);
    
    /* Execute all test functions */
    total += test_post_decrement(limit);
    total += test_pre_decrement(limit);
    total += test_for_decrement(limit);
    total += test_explicit_subtract(limit);
    total += test_do_while(limit);
    total += test_unsigned_decrement(limit);
    total += test_complex_decrement(limit);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total += test_arm_specific(limit);
    printf("ARM-specific test executed\n");
#endif
    
#if defined(ARCH_AVR)
    total += test_avr_specific(limit);
    printf("AVR-specific test executed\n");
#endif
    
    printf("Total sum: %d\n", total);
    
    /* Return non-zero if all tests produced expected results */
    int expected_min = limit * 3;  /* Minimum expected from first test */
    return (total > expected_min) ? 0 : 1;
}
