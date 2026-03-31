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
  #define ARCH_NAME "ARM"
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
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
    volatile int limit = 1000; /* Use volatile to prevent constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
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

/* Loop variant 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract operation */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (int i = limit; i != 0; i = i - 1) */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: do { ... } while (n-- > 0) */
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    return sum;
}

/* Loop variant 6: Unsigned counter (common in doloop) */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit; /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test for ARM */
#if defined(ARCH_ARM)
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subs instruction for decrement-and-compare */
    while (n--) {
        if (n <= 0) break;
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* Architecture-specific test for AVR */
#if defined(ARCH_AVR)
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF); /* AVR often uses 8-bit counters */
    int sum = 0;
    
    while (n--) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    int total = 0;
    int loop_limit;
    
    printf("Testing doloop pattern matching on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    /* Run all applicable test variants */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while(loop_limit);
    total += test_unsigned_counter((unsigned int)loop_limit);
    total += test_register_counter(loop_limit);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total += test_arm_specific(loop_limit);
    printf("ARM-specific test included\n");
#endif
    
#if defined(ARCH_AVR)
    total += test_avr_specific(loop_limit);
    printf("AVR-specific test included\n");
#endif
    
    printf("Total sum from all loops: %d\n", total);
    
    /* Return non-zero to indicate success and prevent optimization away */
    return total != 0 ? 0 : 1;
}
