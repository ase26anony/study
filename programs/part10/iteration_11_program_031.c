/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM)
  #define ARCH_ARM 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
  #define ARCH_PPC 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define ARCH_SUPPORTS_DOLOOP 1
#else
  #define ARCH_GENERIC 1
  #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Test function 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that can't be optimized away */
        /* Prevent loop unrolling with a tiny memory barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pre-decrement pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration for comparison with zero */
    if (n == 0) {
        sum += 5;
    }
    return sum;
}

/* Test function 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
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

/* Test function 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit subtraction separate from comparison */
    }
    return sum;
}

/* Test function 7: Complex pattern with multiple decrements */
NO_OPT static int test_complex_pattern(int limit) {
    int n = limit;
    int m = limit / 2;
    int sum = 0;
    
    /* Outer loop */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Inner loop - might generate different patterns */
        for (int i = m; __builtin_expect(i > 0, 1); i--) {
            sum += 19;
            asm volatile("" : "+r"(sum) : : "memory");
        }
    }
    return sum;
}

/* Test function 8: Unsigned counter (common in doloop optimizations) */
NO_OPT static unsigned int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    
    #if ARCH_SUPPORTS_DOLOOP
        /* For doloop-supported architectures, use architecture-specific patterns */
        int loop_limit;
        
        #ifdef ARCH_ARM
            /* ARM-specific: ensure we're in a mode that supports doloop */
            asm volatile("" : "=r"(loop_limit) : "0"(base_limit));
            loop_limit = loop_limit & 0xFF;  /* Ensure reasonable limit */
            printf("Testing on ARM architecture (doloop supported)\n");
            
        #elif defined(ARCH_AVR)
            /* AVR-specific: smaller limits for 8-bit architecture */
            loop_limit = (base_limit & 0x3F) + 10;  /* 10-73 iterations */
            printf("Testing on AVR architecture (doloop supported)\n");
            
        #elif defined(ARCH_PPC)
            /* PowerPC-specific */
            loop_limit = base_limit;
            printf("Testing on PowerPC architecture (doloop supported)\n");
            
        #elif defined(ARCH_MIPS)
            /* MIPS-specific */
            loop_limit = base_limit;
            printf("Testing on MIPS architecture (doloop supported)\n");
            
        #endif
        
        printf("Using loop limit: %d\n", loop_limit);
        
    #else
        /* Generic fallback for architectures without doloop support */
        int loop_limit = base_limit;
        printf("Testing on generic architecture (doloop may not be supported)\n");
        printf("Using loop limit: %d\n", loop_limit);
    #endif
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_complex_pattern(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return the sum to prevent dead code elimination */
    return total_sum == 0 ? 1 : 0;
}
