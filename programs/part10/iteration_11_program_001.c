/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

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

/* Different loop patterns to generate various decrement RTL sequences */

/* Pattern 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 0x1234;
        /* Prevent loop unrolling */
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
    return sum;
}

/* Pattern 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (int i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 4: Explicit subtract in loop */
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

/* Pattern 5: Do-while with post-decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 0x2468;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 1, 1));
    }
    return sum;
}

/* Pattern 6: While loop with explicit comparison */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Explicit compare to zero */
    while (__builtin_expect(n != 0, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Separate decrement */
    }
    return sum;
}

/* Pattern 7: Unsigned counter (important for some architectures) */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0xABCD;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 8: Counter in register variable (hint to compiler) */
NO_OPT static int test_register_var(int limit) {
    register int n asm("r0") = limit;  /* Architecture-specific register hint */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 0x2468;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Get non-constant loop limit to prevent constant propagation */
    int loop_limit;
    
    if (argc > 1) {
        loop_limit = atoi(argv[1]);
    } else {
        /* Use volatile or system call to prevent compile-time constant */
        volatile int volatile_limit = 100;
        loop_limit = volatile_limit;
        
        /* Alternative: use function call result */
        loop_limit = getpid() & 0xFF;  /* Non-constant but bounded */
        if (loop_limit == 0) loop_limit = 100;
    }
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    printf("Architecture: ");
    
#if ARCH_ARM
    printf("ARM (doloop supported)\n");
#elif ARCH_AVR
    printf("AVR (doloop supported)\n");
#elif ARCH_PPC
    printf("PowerPC (doloop supported)\n");
#elif ARCH_MIPS
    printf("MIPS (doloop supported)\n");
#else
    printf("Generic (doloop may not be supported)\n");
#endif
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_register_var(loop_limit);
    
    printf("Total sum: %d (0x%08x)\n", total_sum, total_sum);
    
    /* Return non-zero result to ensure all loops executed */
    return (total_sum != 0) ? 0 : 1;
}
