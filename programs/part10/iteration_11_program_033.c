/* loop-doloop-test.c - Test program for GCC's doloop optimization */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent inlining and IPA transformations */
#define NOOPT __attribute__((noinline,noipa,noclone))

/* Get a non-constant loop limit to prevent constant propagation */
static volatile int external_limit = 1000;

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
  #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Loop variant 1: Post-decrement in condition */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NOOPT int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) */
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

/* Loop variant 3: For loop with decrement */
NOOPT int test_for_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract operation */
NOOPT int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- or --i */
    }
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NOOPT int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    /* Pattern: do { ... } while (n-- > 0); */
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 0, 1));
    
    return sum;
}

/* Loop variant 6: While loop with explicit compare */
NOOPT int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while ((n = n - 1) >= 0) */
    while (__builtin_expect((n = n - 1) >= 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Unsigned counter (common in doloop patterns) */
NOOPT unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 8: Counter in register variable */
NOOPT int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register for counter */
    int sum = 0;
    
    /* Pattern: while (n-- > 0) with register counter */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test wrappers */
#if ARCH_SUPPORTS_DOLOOP
/* Test specifically for ARM doloop optimization */
NOOPT int test_arm_doloop(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subtract and compare in one instruction */
    while (__builtin_expect(n--, 1)) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

int main(void) {
    int total = 0;
    int loop_limit;
    
    /* Get a non-constant loop limit to prevent constant propagation */
#if ARCH_SUPPORTS_DOLOOP
    /* Use a volatile read or system call for doloop-supported architectures */
    loop_limit = external_limit;
    /* Mix with a small non-constant value */
    loop_limit = (getpid() & 0x3F) + 50;  /* 50-113 iterations */
#else
    /* For non-doloop architectures, still test but with simpler pattern */
    loop_limit = 100;
#endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    
    /* Execute all test functions */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while(loop_limit);
    total += test_while_explicit(loop_limit);
    total += test_unsigned_decrement((unsigned int)loop_limit);
    total += test_register_counter(loop_limit);
    
#if ARCH_SUPPORTS_DOLOOP
    total += test_arm_doloop(loop_limit);
    printf("Running on doloop-supported architecture\n");
#else
    printf("Running on generic architecture (doloop may not be supported)\n");
#endif
    
    printf("Total sum from all loops: %d\n", total);
    
    /* Return non-zero if any test failed (simplified check) */
    return total == 0 ? 1 : 0;
}
