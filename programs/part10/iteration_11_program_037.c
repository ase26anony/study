/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

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

/* Different loop patterns to trigger the PLUS with -1 RTL pattern */

/* Pattern 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 0x1234;  /* Arbitrary constant */
        /* Prevent loop body optimization without affecting counter */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration for comparison with zero */
    if (n == 0) {
        sum += 0x5678;
    }
    return sum;
}

/* Pattern 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 4: Explicit subtract operation */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (; i != 0; i = i - 1) pattern */
    for (; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 5: Do-while with decrement */
NO_OPT static int test_do_while_decrement(int limit) {
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

/* Pattern 6: While loop with explicit comparison */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n) with explicit decrement */
    while (__builtin_expect(n, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit subtraction */
    }
    return sum;
}

/* Pattern 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int counter asm("r0") = limit;  /* Suggest register for ARM */
    int sum = 0;
    
    while (__builtin_expect(counter-- > 0, 1)) {
        sum += 0xFACE;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 8: Unsigned counter (might generate different RTL) */
NO_OPT static int test_unsigned_counter(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0xBEEF;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Helper to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int external_limit = 100;  /* Prevent constant propagation */
    
    /* Use various sources to make limit non-constant */
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop targets, use values that work well with the optimization */
    return external_limit + (getpid() & 0xF);  /* Small, non-constant value */
#else
    /* For generic targets, just return a reasonable value */
    return external_limit;
#endif
}

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get a non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    printf("Architecture: ");
    
#if defined(ARCH_ARM)
    printf("ARM (doloop supported)\n");
#elif defined(ARCH_AVR)
    printf("AVR (doloop supported)\n");
#elif defined(ARCH_PPC)
    printf("PowerPC (doloop supported)\n");
#elif defined(ARCH_MIPS)
    printf("MIPS (doloop supported)\n");
#else
    printf("Generic (doloop may not be supported)\n");
#endif
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_decrement(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_register_counter(loop_limit);
    total_sum += test_unsigned_counter(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to ensure all code paths are considered */
    return total_sum != 0 ? 0 : 1;
}
