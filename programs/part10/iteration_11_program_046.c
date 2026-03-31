/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM)
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
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque operation - cannot be optimized away */
        sum += 3;
        /* Prevent compiler from moving operations across loop boundary */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; i != 0; i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 5: Do-while with decrement (should also trigger the pattern) */
NO_OPT static int test_do_while_decrement(int limit) {
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

/* Loop variant 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    while (n) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;
    }
    return sum;
}

/* Loop variant 7: Counter in register variable (hint to keep in register) */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit; /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (n--) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 8: Unsigned counter (common in doloop patterns) */
NO_OPT static int test_unsigned_counter(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    while (n-- != 0) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test wrappers */
#if DOLOOP_SUPPORTED
/* For doloop-supported architectures, use more aggressive patterns */
NO_OPT static int test_arch_specific_1(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This pattern often generates the (plus reg -1) RTL */
    while ((n = n - 1) >= 0) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

NO_OPT static int test_arch_specific_2(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Alternative pattern that might generate PLUS with -1 */
    for (; n; n--) {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop pattern matching with limit = %d\n", loop_limit);
    
    /* Execute all test functions */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_decrement(loop_limit);
    total_sum += test_while_explicit_zero(loop_limit);
    total_sum += test_register_counter(loop_limit);
    total_sum += test_unsigned_counter(loop_limit);
    
#if DOLOOP_SUPPORTED
    printf("Running architecture-specific doloop tests...\n");
    total_sum += test_arch_specific_1(loop_limit);
    total_sum += test_arch_specific_2(loop_limit);
#endif
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum > 0) ? 0 : 1;
}
