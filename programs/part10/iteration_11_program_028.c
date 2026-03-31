/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent function inlining to preserve loop structure */
#define NO_OPT __attribute__((noinline, noipa, noclone))

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

/* Different loop patterns to trigger the PLUS with -1 RTL pattern */

/* Pattern 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;  /* Prevent optimization */
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
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 3: For loop with decrement */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* for (int i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 4: Explicit subtract operation */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (int i = limit; i != 0; i = i - 1) pattern */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Pattern 5: Do-while with post-decrement */
NO_OPT static int test_do_while_postdec(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Pattern 6: Count down to zero with unsigned */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    /* while (n-- != 0) pattern */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 7: Complex decrement pattern that might generate PLUS with -1 */
NO_OPT static int test_complex_decrement(int limit) {
    volatile int counter = limit;  /* volatile to prevent constant propagation */
    int n = counter;
    int sum = 0;
    
    /* This might generate different RTL patterns */
    for (; n > 0; n = n + (-1)) {  /* Explicit PLUS with -1 */
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 8: Nested loops to increase complexity */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer_sum = 0;
    
    for (int i = outer_limit; __builtin_expect(i > 0, 1); i--) {
        int inner_sum = 0;
        int j = inner_limit;
        
        /* Inner loop with post-decrement */
        while (__builtin_expect(j-- > 0, 1)) {
            inner_sum += 2;
            asm volatile("" : "+r"(inner_sum) : : "memory");
        }
        
        outer_sum += inner_sum;
    }
    
    return outer_sum;
}

/* Helper to get non-constant loop limit */
static int get_loop_limit(void) {
    /* Use various methods to get non-constant values */
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop architectures, use values that won't be optimized away */
    volatile int base = 1000;
    
    #ifdef ARCH_ARM
        /* ARM-specific: use values that work well with doloop */
        return base + (getpid() & 0xF);  /* Small variation */
    #elif defined(ARCH_AVR)
        /* AVR: smaller loops */
        return 100 + (getpid() & 0x7);
    #elif defined(ARCH_PPC)
        /* PowerPC */
        return 500 + (getpid() & 0xF);
    #elif defined(ARCH_MIPS)
        /* MIPS */
        return 300 + (getpid() & 0xF);
    #else
        return 200;
    #endif
#else
    /* Generic fallback */
    return 100;
#endif
}

int main(void) {
    int total_sum = 0;
    
    /* Get non-constant loop limits to prevent constant propagation */
    int limit1 = get_loop_limit();
    int limit2 = get_loop_limit() / 2;
    unsigned int ulimit = (unsigned int)get_loop_limit();
    
    printf("Testing doloop patterns on ");
    
#if defined(ARCH_ARM)
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
    
    printf("Loop limit: %d\n", limit1);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(limit1);
    total_sum += test_pre_decrement(limit1);
    total_sum += test_for_loop_decrement(limit1);
    total_sum += test_explicit_subtract(limit1);
    total_sum += test_do_while_postdec(limit1);
    total_sum += test_unsigned_decrement(ulimit);
    total_sum += test_complex_decrement(limit1);
    total_sum += test_nested_loops(limit2, limit2 / 2);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return total_sum == 0 ? 1 : 0;
}
