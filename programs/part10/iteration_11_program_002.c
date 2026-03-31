/* doloop_test.c - Test program for GCC's doloop optimization pattern matching */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

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

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) - should generate (plus n -1) compare with 0 */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory"); /* Memory barrier */
    }
    
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
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

/* Loop variant 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    /* Pattern: for (i = limit; i != 0; i = i - 1) */
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 5: Do-while with decrement - do { ... } while (n-- > 0) */
NO_OPT static int test_do_while_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    
    return sum;
}

/* Loop variant 6: While loop with explicit comparison */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n != 0) { n = n - 1; ... } */
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    
    return sum;
}

/* Architecture-specific loop variants */

#if defined(ARCH_ARM)
/* ARM-specific: Use register variables to encourage specific register allocation */
NO_OPT static int test_arm_specific(int limit) {
    register int n asm("r4") = limit;  /* Suggest register r4 for counter */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        /* ARM-specific asm to prevent optimization */
        asm volatile("add %0, %0, #19" : "+r"(sum) : : "cc");
    }
    
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific: Use 8-bit counters which are common for doloop on AVR */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (limit > 255) ? 255 : limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}
#endif

#if defined(ARCH_PPC)
/* PowerPC-specific: Use CTR-like pattern */
NO_OPT static int test_ppc_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern that might generate bdnz-like RTL */
    do {
        if (n <= 0) break;
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(--n > 0, 1));
    
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop optimization on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    /* Execute all generic loop variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_decrement(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    
    /* Execute architecture-specific variants */
#if defined(ARCH_ARM)
    total_sum += test_arm_specific(loop_limit);
    printf("ARM-specific test executed\n");
#endif

#if defined(ARCH_AVR)
    total_sum += test_avr_specific(loop_limit);
    printf("AVR-specific test executed\n");
#endif

#if defined(ARCH_PPC)
    total_sum += test_ppc_specific(loop_limit);
    printf("PowerPC-specific test executed\n");
#endif

#if defined(ARCH_MIPS)
    printf("MIPS architecture detected - using generic tests\n");
#endif

    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to indicate execution */
    return total_sum != 0 ? 0 : 1;
}
