/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations with various attributes */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));
static int static_builtin_wrapper(int x) __attribute__((used, nothrow));

/* Global volatile variables to prevent optimization */
volatile int global_seed = 0;
volatile int global_result = 0;

/* Barrier to prevent optimization */
#define OPT_BARRIER() asm volatile("" : : : "memory")

/* Test 1: Arithmetic built-ins with volatile context */
__attribute__((noinline, noclone))
int test_builtin_arithmetic(volatile int seed) {
    volatile int acc = 0;
    
    /* Use built-ins in a loop with volatile counter */
    for (volatile int i = 0; i < 5; i++) {
        int val = seed + i * 17;
        
        /* Multiple arithmetic built-ins */
        acc += __builtin_abs(val);
        acc += (int)__builtin_sqrtf((float)(val > 0 ? val : -val));
        acc += __builtin_ffs(val | 1);  /* Ensure non-zero */
        
        /* Use __builtin_expect to influence branching */
        if (__builtin_expect((val & 1) != 0, 0)) {
            acc += __builtin_clz((unsigned int)val);
        }
    }
    
    OPT_BARRIER();
    return acc;
}

/* Test 2: Bit manipulation built-ins */
__attribute__((noinline, noclone))
int test_builtin_bitops(volatile unsigned int seed) {
    volatile unsigned int result = 0;
    
    /* Chain multiple bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_ctz(seed | 1);  /* Avoid zero */
    result += __builtin_parity(seed);
    result += __builtin_bswap32(seed);
    
    /* Use __builtin_choose_expr */
    int choice = __builtin_choose_expr(
        (seed & 0x100) != 0,
        __builtin_clz(seed),
        __builtin_ctz(seed | 1)
    );
    result += choice;
    
    OPT_BARRIER();
    return (int)result;
}

/* Test 3: Overflow checking built-ins */
__attribute__((noinline, noclone))
int test_builtin_overflow(volatile int a, volatile int b) {
    volatile int total = 0;
    int overflow;
    
    /* Test various overflow built-ins */
    if (__builtin_add_overflow(a, b, &overflow)) {
        total += __builtin_abs(overflow);
    } else {
        total += overflow;
    }
    
    if (__builtin_mul_overflow(a, b, &overflow)) {
        total += __builtin_clz((unsigned int)overflow);
    }
    
    if (__builtin_sub_overflow(a, b, &overflow)) {
        total += __builtin_ctz((unsigned int)(overflow & ~0U));
    }
    
    /* Use __builtin_expect with overflow results */
    if (__builtin_expect(__builtin_add_overflow_p(a, b, (int)0), 0)) {
        total += 1000;
    }
    
    OPT_BARRIER();
    return total;
}

/* Test 4: Static function with multiple attributes using built-ins */
__attribute__((visibility("hidden"), nothrow, used, noinline))
static int static_builtin_wrapper(int x) {
    /* Use builtin inside attributed static function */
    int result = __builtin_expect(x > 0, 1) ? 
                 __builtin_popcount((unsigned int)x) : 
                 __builtin_abs(x);
    
    /* Use __builtin_unreachable in a controlled way */
    if (result < 0) {
        __builtin_unreachable();  /* Should never happen with our inputs */
    }
    
    OPT_BARRIER();
    return result;
}

/* Test 5: External linkage simulation with built-ins */
__attribute__((visibility("hidden"), noinline))
int external_builtin_user(int x) {
    volatile int acc = 0;
    
    /* Use multiple built-ins in external-like function */
    acc += __builtin_abs(x);
    acc += __builtin_ffs(x | 1);
    
    /* Complex expression with built-in */
    int temp = __builtin_add_overflow_p(x, 42, (int)0) ? 
               __builtin_clz((unsigned int)x) : 
               __builtin_ctz((unsigned int)(x | 1));
    acc += temp;
    
    /* Conditional __builtin_unreachable */
    if (x == 0xDEADBEEF) {  /* Unlikely value */
        __builtin_unreachable();
    }
    
    OPT_BARRIER();
    return acc;
}

/* Test 6: Built-ins with assembly barriers */
__attribute__((noinline, noclone))
int test_builtin_with_asm(volatile int seed) {
    int result = 0;
    
    /* Mix built-ins with inline assembly */
    asm volatile("# Assembly barrier before builtin" : : : "memory");
    result += __builtin_bswap32((unsigned int)seed);
    asm volatile("# Assembly barrier after builtin" : : : "memory");
    
    result += __builtin_popcount((unsigned int)seed);
    asm volatile("" : : : "memory");
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(seed)) {
        result += __builtin_clz((unsigned int)seed | 1);
    }
    
    return result;
}

/* Test 7: Built-in in a macro expansion */
#define USE_BUILTIN_MACRO(x) \
    (__builtin_expect((x) > 100, 0) ? \
     __builtin_sqrtf((float)(x)) : \
     __builtin_abs(x))

__attribute__((noinline, noclone))
int test_builtin_macro(volatile int x) {
    return (int)USE_BUILTIN_MACRO(x);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed from multiple sources */
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    seed += (int)time(NULL);
    seed ^= (int)(long)main;  /* Use function address as entropy */
    
    int checksum = 0;
    
    /* Run all test functions */
    checksum += test_builtin_arithmetic(seed);
    checksum += test_builtin_bitops((unsigned int)seed);
    checksum += test_builtin_overflow(seed, seed * 2 + 1);
    checksum += static_builtin_wrapper(seed);
    checksum += external_builtin_user(seed);
    checksum += test_builtin_with_asm(seed);
    checksum += test_builtin_macro(seed);
    
    /* Additional direct built-in usage in main */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(checksum);
    direct_result += __builtin_popcount((unsigned int)checksum);
    
    /* Use __builtin_expect for final condition */
    if (__builtin_expect(direct_result > 0, 1)) {
        checksum += direct_result;
    }
    
    /* Prevent dead code elimination */
    global_result = checksum;
    
    /* Use __builtin_return_address */
    void *ra = __builtin_return_address(0);
    checksum ^= (int)(long)ra;
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero result */
}

/* Additional function to increase declaration count */
__attribute__((constructor, visibility("hidden")))
static void init_builtin_test(void) {
    /* Use built-in in constructor */
    volatile int init_val = __builtin_abs(-42);
    global_seed = init_val;
}

/* Another static function with different attributes */
__attribute__((unused, noinline, nothrow))
static int unused_builtin_func(int x) {
    /* This function may be ignored but still processed */
    return __builtin_ffs(x) + __builtin_clz((unsigned int)x);
}
