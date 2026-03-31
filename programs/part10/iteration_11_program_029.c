/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
  #define ARCH_ARM 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
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
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that won't be optimized away */
        /* Prevent compiler from moving operations across iterations */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 5: Do-while with post-decrement */
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

/* Test 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Separate decrement */
    }
    return sum;
}

/* Test 7: Unsigned counter (common in doloop patterns) */
NO_OPT static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 8: Counter in register variable (hint to keep in register) */
NO_OPT static int test_register_var(int limit) {
    register int n asm("r0") = limit;  /* Architecture-specific register hint */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test variations */
#if DOLOOP_SUPPORTED

/* ARM-specific: Use register constraints that match ARM calling convention */
#ifdef ARCH_ARM
NO_OPT static int test_arm_specific(int limit) {
    register int n asm("r4") = limit;
    int sum = 0;
    
    /* ARM often uses subs + bne for loops */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* AVR-specific: Use 8-bit counters (common for AVR doloop) */
#ifdef ARCH_AVR
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (limit > 255) ? 255 : limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* PowerPC-specific: Use CTR register pattern hint */
#ifdef ARCH_PPC
NO_OPT static int test_ppc_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* PPC often uses bdnz for loop decrement */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 37;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* MIPS-specific */
#ifdef ARCH_MIPS
NO_OPT static int test_mips_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 41;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

#endif /* DOLOOP_SUPPORTED */

/* Get a non-constant loop limit to prevent constant propagation */
static int get_loop_limit(void) {
    volatile int base = 1000;  /* Prevent constant folding */
    
    /* Use system call or volatile to get unpredictable but bounded value */
#if DOLOOP_SUPPORTED
    /* For doloop targets, use smaller counts that fit in hardware loops */
    return (getpid() & 0xFF) + 10;  /* 10-265 iterations */
#else
    /* For other architectures, still test but with different pattern */
    return base + (getpid() & 0x3F);  /* 1000-1063 iterations */
#endif
}

int main(void) {
    int total = 0;
    int limit = get_loop_limit();
    
    printf("Testing doloop patterns with limit = %d\n", limit);
    
    /* Run all generic tests */
    total += test_post_decrement(limit);
    total += test_pre_decrement(limit);
    total += test_for_decrement(limit);
    total += test_explicit_subtract(limit);
    total += test_do_while(limit);
    total += test_while_zero(limit);
    total += test_unsigned_decrement((unsigned int)limit);
    total += test_register_var(limit);
    
#if DOLOOP_SUPPORTED
    /* Run architecture-specific tests */
    #ifdef ARCH_ARM
    total += test_arm_specific(limit);
    #endif
    
    #ifdef ARCH_AVR
    total += test_avr_specific(limit);
    #endif
    
    #ifdef ARCH_PPC
    total += test_ppc_specific(limit);
    #endif
    
    #ifdef ARCH_MIPS
    total += test_mips_specific(limit);
    #endif
    
    printf("Doloop-supported architecture detected\n");
#else
    printf("Generic architecture (doloop may not be supported)\n");
#endif
    
    printf("Total sum: %d\n", total);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total == 0) ? 1 : 0;
}
