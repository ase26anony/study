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

/* Test 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple operation that can't be optimized away */
        sum += 0x1234;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Start with n+1 to match the same number of iterations */
    n = limit + 1;
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtraction, not i-- */
    }
    return sum;
}

/* Test 5: Do-while with decrement at end */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 0x2468;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(--n > 0, 1));
    }
    return sum;
}

/* Test 6: Unsigned counter (common in doloop patterns) */
NO_OPT static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 7: Counter in register variable (hint to keep in register) */
NO_OPT static int test_register_var(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0xFACE;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 8: Nested loops - outer loop might trigger doloop */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer = outer_limit;
    int sum = 0;
    
    while (__builtin_expect(outer-- > 0, 1)) {
        int inner = inner_limit;
        while (__builtin_expect(inner-- > 0, 1)) {
            sum += 0xBEEF;
            asm volatile("" : "+r"(sum) : : "memory");
        }
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int limit = base_limit;
    
    /* Use function call for additional non-constancy */
    int pid = getpid();
    limit = (pid & 0xFF) + 50;  /* Limit between 50-305 */
    
    printf("Testing doloop pattern matching with limit=%d\n", limit);
    printf("DOLOOP_SUPPORTED=%d\n", DOLOOP_SUPPORTED);
    
#if DOLOOP_SUPPORTED
    printf("Testing on doloop-supported architecture\n");
    
    /* Call all test functions */
    total_sum += test_post_decrement(limit);
    total_sum += test_pre_decrement(limit);
    total_sum += test_for_decrement(limit);
    total_sum += test_explicit_subtract(limit);
    total_sum += test_do_while(limit);
    total_sum += test_unsigned_decrement((unsigned int)limit);
    total_sum += test_register_var(limit);
    total_sum += test_nested_loops(limit/10, 10);
    
    printf("Total sum: %d (0x%08x)\n", total_sum, total_sum);
    
    /* Architecture-specific variations */
    #ifdef ARCH_ARM
        /* ARM-specific: test with different data types */
        printf("ARM architecture detected\n");
        {
            uint32_t arm_limit = limit;
            while (arm_limit-- > 0) {
                asm volatile("nop" : : : "memory");
            }
        }
    #elif defined(ARCH_AVR)
        printf("AVR architecture detected\n");
    #elif defined(ARCH_PPC)
        printf("PowerPC architecture detected\n");
        /* PPC often uses special decrement-and-branch instructions */
    #elif defined(ARCH_MIPS)
        printf("MIPS architecture detected\n");
    #endif
    
#else
    /* Fallback for architectures without doloop support */
    printf("Testing generic loops (doloop not supported on this arch)\n");
    
    /* Still test the patterns, but they won't trigger doloop */
    total_sum += test_post_decrement(limit);
    total_sum += test_for_decrement(limit);
    
    printf("Total sum: %d\n", total_sum);
#endif
    
    return total_sum != 0 ? 0 : 1;
}
