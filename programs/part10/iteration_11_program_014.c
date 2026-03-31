/* test-doloop-patterns.c
 * Target: Trigger specific RTL pattern matching in GCC's doloop optimization
 * Compile with: -O2 -fdump-rtl-loop2 -fdump-rtl-doloop2
 * For ARM: -O2 -march=armv7-a
 * For AVR: -O2 -mmcu=atmega328p
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone, optimize("O0")))

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

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPTIMIZE
static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This should generate: (compare (plus reg -1) (const_int 0)) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that can't be optimized away */
        asm volatile("" : "+r"(sum) : : "memory"); /* Prevent dead code elimination */
    }
    
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This might also generate the PLUS -1 pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPTIMIZE
static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPTIMIZE
static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NO_OPTIMIZE
static int test_do_while_post_decrement(int limit) {
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
NO_OPTIMIZE
static int test_while_compare_zero(int limit) {
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
NO_OPTIMIZE
static int test_unsigned_counter(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 8: Counter in register variable */
NO_OPTIMIZE
static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Architecture-specific test that uses doloop-friendly patterns */
#if ARCH_SUPPORTS_DOLOOP
NO_OPTIMIZE
static int test_arch_specific_doloop(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This pattern is most likely to trigger doloop optimization */
    for (; n != 0; n--) {
        sum += 29;
        /* Use architecture-specific asm to prevent optimization */
        #ifdef ARCH_ARM
            asm volatile("add %0, %0, #29" : "+r"(sum) : : "cc");
        #elif defined(ARCH_AVR)
            asm volatile("add %0, %A0" : "+r"(sum) : :);
        #else
            asm volatile("" : "+r"(sum) : : "memory");
        #endif
    }
    
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop patterns on ");
    
    #ifdef ARCH_ARM
        printf("ARM architecture\n");
    #elif defined(ARCH_AVR)
        printf("AVR architecture\n");
    #elif defined(ARCH_PPC)
        printf("PowerPC architecture\n");
    #elif defined(ARCH_MIPS)
        printf("MIPS architecture\n");
    #else
        printf("generic architecture (doloop may not be supported)\n");
    #endif
    
    printf("Loop limit: %d\n", loop_limit);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post_decrement(loop_limit);
    total_sum += test_while_compare_zero(loop_limit);
    total_sum += test_unsigned_counter(loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    #if ARCH_SUPPORTS_DOLOOP
        total_sum += test_arch_specific_doloop(loop_limit);
    #endif
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero to indicate execution */
    return total_sum != 0 ? 0 : 1;
}
