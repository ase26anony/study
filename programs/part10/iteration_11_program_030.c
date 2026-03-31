/* test-doloop-pattern.c
 * Targets GCC's doloop optimization pattern matching
 * Specifically aims to trigger lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

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

/* Test function 1: Post-decrement in condition */
/* Should generate: (plus reg -1) compare with 0 */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract pattern */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (i = limit; i != 0; i = i - 1) pattern */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Test function 6: Unsigned counter (common in doloop) */
NO_OPT static int test_unsigned_counter(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r12") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific variants */
#if DOLOOP_SUPPORTED

/* ARM-specific: Use register constraints that might help */
#ifdef ARCH_ARM
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use asm to encourage specific register usage */
    asm volatile("" : "=r"(n) : "0"(n));
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        /* ARM-specific asm to prevent optimization */
        asm volatile("add %0, %0, #23" : "+r"(sum) : : "cc");
    }
    return sum;
}
#endif

/* AVR-specific: Small loops are common */
#ifdef ARCH_AVR
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);  /* AVR has 8-bit loops often */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

#endif /* DOLOOP_SUPPORTED */

int main(void) {
    int total = 0;
    
    /* Get non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int limit = base_limit;
    
    /* Also use function call to get dynamic value */
    int dynamic_limit = getpid() & 0x3F;  /* 0-63 */
    if (dynamic_limit == 0) dynamic_limit = 10;
    
    printf("Testing doloop pattern matching\n");
    printf("Architecture: ");
    
#if DOLOOP_SUPPORTED
    #ifdef ARCH_ARM
    printf("ARM (doloop supported)\n");
    #elif defined(ARCH_AVR)
    printf("AVR (doloop supported)\n");
    #elif defined(ARCH_PPC)
    printf("PowerPC (doloop supported)\n");
    #elif defined(ARCH_MIPS)
    printf("MIPS (doloop supported)\n");
    #endif
    
    /* Run all test functions */
    total += test_post_decrement(limit);
    total += test_pre_decrement(limit);
    total += test_for_decrement(limit);
    total += test_explicit_subtract(limit);
    total += test_do_while(limit);
    total += test_unsigned_counter(limit);
    total += test_register_counter(limit);
    
    #ifdef ARCH_ARM
    total += test_arm_specific(dynamic_limit);
    #endif
    
    #ifdef ARCH_AVR
    total += test_avr_specific(dynamic_limit);
    #endif
    
#else
    printf("Generic (doloop may not be supported)\n");
    /* Still run tests but with different expectations */
    total += test_post_decrement(limit);
    total += test_pre_decrement(limit);
    total += test_for_decrement(limit);
#endif
    
    printf("Total sum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        printf("Pattern test completed (high total indicates loops ran)\n");
    }
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
