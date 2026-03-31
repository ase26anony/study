/* test-doloop-coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with doloop pattern matching */
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

/* Test function 1: Post-decrement in condition */
/* while (n-- > 0) pattern */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 0x1234;  /* Arbitrary constant */
        /* Prevent loop body from being optimized away */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition */
/* while (--n > 0) pattern */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration */
    sum += 0x5678;
    return sum;
}

/* Test function 3: For loop with decrement */
/* for (int i = limit; i > 0; i--) pattern */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract pattern */
/* for (int i = limit; i != 0; i = i - 1) pattern */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (i != 0) {
        sum += 0xDEF0;
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
    
    do {
        sum += 0x2468;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Test function 6: Unsigned counter pattern */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (n-- > 0) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 7: Complex decrement pattern with multiple uses */
NO_OPT static int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    int temp;
    
    while (n > 0) {
        temp = n;
        sum += temp * 0x1111;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Separate decrement statement */
    }
    return sum;
}

/* Main function with architecture-specific adaptations */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 1000;
    int loop_limit;
    
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use a volatile read 
       to prevent constant folding */
    loop_limit = base_limit;
    
    /* Add architecture-specific tweaks */
    #ifdef ARCH_ARM
        /* ARM-specific: ensure we're not in Thumb mode for certain patterns */
        asm volatile("" : "=r"(loop_limit) : "0"(loop_limit) : "memory");
    #elif defined(ARCH_AVR)
        /* AVR: smaller loops are better */
        loop_limit = 100;
    #elif defined(ARCH_PPC)
        /* PowerPC: use typical values */
        loop_limit = 500;
    #endif
#else
    /* Generic fallback - still try to exercise the code */
    loop_limit = 100;
#endif
    
    /* Also try with a function return value to prevent constant propagation */
    int dynamic_limit = getpid() & 0xFF;
    if (dynamic_limit < 10) dynamic_limit = 100;
    
    printf("Testing doloop patterns with limit=%d, dynamic=%d\n", 
           loop_limit, dynamic_limit);
    
    /* Run all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_complex_decrement(loop_limit);
    
    /* Run some tests with dynamic limit */
    total_sum += test_post_decrement(dynamic_limit);
    total_sum += test_for_loop_decrement(dynamic_limit);
    
    /* Use the result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum == 0) ? 1 : 0;
}
