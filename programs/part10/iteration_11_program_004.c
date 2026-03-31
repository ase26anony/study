/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent inlining and IPA transformations */
#define NOOPT __attribute__((noinline, noipa, noclone))

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

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile to prevent constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that can't be optimized away */
        /* Use asm to prevent optimization without affecting RTL pattern */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NOOPT int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NOOPT int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NOOPT int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; i != 0; i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NOOPT int test_do_while_post(int limit) {
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

/* Loop variant 6: While loop with explicit comparison to zero */
NOOPT int test_while_explicit_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    return sum;
}

/* Loop variant 7: Unsigned counter (common in doloop patterns) */
NOOPT int test_unsigned_counter(int limit) {
    unsigned int n = (unsigned int)limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return (int)sum;
}

/* Loop variant 8: Counter in register variable (hint to keep in register) */
NOOPT int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test functions */
#ifdef ARCH_ARM
/* ARM-specific: Use register constraints that might encourage doloop */
NOOPT int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses decrement-and-branch instructions */
    while (n-- > 0) {
        sum += 29;
        /* ARM-specific asm to prevent optimization */
        asm volatile("add %0, %0, #29" : "+r"(sum) : : "cc");
    }
    return sum;
}
#endif

#ifdef ARCH_AVR
/* AVR-specific: Small loops are common on AVR */
NOOPT int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);  /* AVR has 8-bit registers */
    uint8_t sum = 0;
    
    while (n-- > 0) {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return (int)sum;
}
#endif

#ifdef ARCH_PPC
/* PowerPC-specific: Use CTR register pattern */
NOOPT int test_ppc_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern that might encourage use of decrement-and-branch CTR */
    for (int i = n; i > 0; --i) {
        sum += 37;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    int total = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop pattern matching on ");
    
#if defined(ARCH_ARM)
    printf("ARM architecture\n");
#elif defined(ARCH_AVR)
    printf("AVR architecture\n");
#elif defined(ARCH_PPC)
    printf("PowerPC architecture\n");
#elif defined(ARCH_MIPS)
    printf("MIPS architecture\n");
#else
    printf("generic architecture\n");
#endif
    
    printf("Loop limit: %d\n", loop_limit);
    
    /* Run all test variants */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_loop_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while_post(loop_limit);
    total += test_while_explicit_zero(loop_limit);
    total += test_unsigned_counter(loop_limit);
    total += test_register_counter(loop_limit);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total += test_arm_specific(loop_limit);
#elif defined(ARCH_AVR)
    total += test_avr_specific(loop_limit);
#elif defined(ARCH_PPC)
    total += test_ppc_specific(loop_limit);
#endif
    
    printf("Total sum from all loops: %d\n", total);
    
    /* Return non-zero if any test failed (simplified check) */
    return total == 0 ? 1 : 0;
}
