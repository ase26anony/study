/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone))

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
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3; /* Simple arithmetic that can't be optimized away */
        /* Prevent counter aliasing with asm barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pre-decrement pattern */
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

/* Loop variant 5: Do-while with post-decrement - do { ... } while (n-- > 0) */
NO_OPTIMIZE
static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    return sum;
}

/* Loop variant 6: While with explicit comparison - while ((n = n - 1) >= 0) */
NO_OPTIMIZE
static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect((n = n - 1) >= 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Unsigned counter to avoid signed overflow issues */
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

/* Loop variant 8: Counter in register with volatile to force reload */
NO_OPTIMIZE
static int test_volatile_counter(int limit) {
    register int n asm("r0") = limit; /* Suggest register for counter */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        /* Force memory barrier to prevent reordering */
        asm volatile("" : "+r"(sum), "+r"(n) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
#if DOLOOP_SUPPORTED
    printf("Testing on doloop-supported architecture\n");
#if defined(ARCH_ARM)
    printf("ARM architecture detected\n");
#elif defined(ARCH_AVR)
    printf("AVR architecture detected\n");
#elif defined(ARCH_PPC)
    printf("PowerPC architecture detected\n");
#elif defined(ARCH_MIPS)
    printf("MIPS architecture detected\n");
#endif
    /* Use volatile or system call to prevent constant propagation */
    loop_limit = getpid() & 0xFFF; /* Non-constant but reasonable value */
    if (loop_limit <= 0) loop_limit = 1000;
#else
    printf("Generic architecture - doloop may not be supported\n");
    loop_limit = get_loop_limit();
#endif
    
    printf("Loop limit: %d\n", loop_limit);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_counter(loop_limit);
    total_sum += test_volatile_counter(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to indicate success and prevent dead code elimination */
    return total_sum != 0 ? 0 : 1;
}
