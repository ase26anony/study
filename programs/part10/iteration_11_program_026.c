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

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Test 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;  /* Prevent dead code elimination */
        /* Prevent counter aliasing with body */
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
NO_OPT static int test_for_loop_decrement(int limit) {
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
NO_OPT static int test_do_while_post(int limit) {
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
NO_OPT static int test_while_explicit(int limit) {
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
NO_OPT static int test_unsigned_counter(int limit) {
    unsigned int n = (unsigned int)limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return (int)sum;
}

/* Test 8: Counter in register variable (hint to keep in register) */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
#if ARCH_SUPPORTS_DOLOOP
    /* Use volatile or system call for doloop-supported architectures */
    volatile int vol_limit = 100;
    loop_limit = vol_limit;
    
    /* Architecture-specific hints */
    #ifdef ARCH_ARM
        asm volatile("" : : : "memory");
        /* ARM-specific: ensure we're not in Thumb mode if that matters */
        #ifdef __thumb__
            asm volatile(".code 32" : : : "memory");
        #endif
    #elif defined(ARCH_AVR)
        /* AVR: small loops work best */
        loop_limit = 100;
    #elif defined(ARCH_PPC)
        /* PowerPC: use typical loop counts */
        loop_limit = 1000;
    #elif defined(ARCH_MIPS)
        /* MIPS: standard loop */
        loop_limit = 500;
    #endif
#else
    /* Generic architecture fallback */
    loop_limit = get_loop_limit();
#endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    printf("Architecture: ");
    
#if defined(ARCH_ARM)
    printf("ARM\n");
#elif defined(ARCH_AVR)
    printf("AVR\n");
#elif defined(ARCH_PPC)
    printf("PowerPC\n");
#elif defined(ARCH_MIPS)
    printf("MIPS\n");
#else
    printf("Generic\n");
#endif
    
    /* Run all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_counter(loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to ensure all code paths are considered */
    return total_sum != 0 ? 0 : 1;
}
