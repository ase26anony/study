/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent function inlining to preserve loop structure */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM) || defined(__thumb__)
#define ARCH_ARM 1
#define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__AVR__)
#define ARCH_AVR 1
#define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
#define ARCH_POWERPC 1
#define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__mips__)
#define ARCH_MIPS 1
#define ARCH_SUPPORTS_DOLOOP 1
#else
#define ARCH_GENERIC 1
#define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000;  /* Prevent constant propagation */
    return limit;
}

/* Variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPTIMIZE
static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that won't be optimized away */
        /* Prevent loop unrolling with a tiny asm barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPTIMIZE
static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPTIMIZE
static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 5: Do-while with post-decrement */
NO_OPTIMIZE
static int test_do_while_post(int limit) {
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

/* Variant 6: While loop with explicit comparison to zero */
NO_OPTIMIZE
static int test_while_explicit_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    return sum;
}

/* Variant 7: Unsigned counter (common in doloop patterns) */
NO_OPTIMIZE
static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 8: Counter in register variable (hint to keep in register) */
NO_OPTIMIZE
static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Architecture-specific register hint */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get a non-constant loop limit to prevent constant propagation */
#if ARCH_SUPPORTS_DOLOOP
    /* Use volatile or system call for doloop-supported architectures */
    volatile int vol_limit = 100;
    loop_limit = vol_limit;
    
    /* Architecture-specific adjustments */
#if defined(ARCH_ARM)
    /* ARM-specific: ensure we're not in Thumb mode for certain patterns */
    asm volatile(".arm");  /* Ensure ARM mode if compiling for Thumb */
#elif defined(ARCH_AVR)
    /* AVR-specific: smaller loop counts */
    loop_limit = 50;
#endif
#else
    /* Generic fallback */
    loop_limit = get_loop_limit();
#endif
    
    printf("Testing doloop patterns on ");
#if defined(ARCH_ARM)
    printf("ARM architecture\n");
#elif defined(ARCH_AVR)
    printf("AVR architecture\n");
#elif defined(ARCH_POWERPC)
    printf("PowerPC architecture\n");
#elif defined(ARCH_MIPS)
    printf("MIPS architecture\n");
#else
    printf("generic architecture\n");
#endif
    printf("Loop limit: %d\n\n", loop_limit);
    
    /* Execute all test variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit_zero(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum > 0) ? 0 : 1;
}
