/* test_targhooks.c - Comprehensive built-in function test to trigger flag-setting hooks */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Helper function with attributes that may interact with built-in processing */
static int __attribute__((noinline, noclone, used))
test_builtin_arithmetic(volatile int seed) {
    volatile int result = 0;
    
    /* Use math built-ins with volatile variables to prevent constant folding */
    for (volatile int i = 0; i < 3; i++) {
        int val = seed + i * 100;
        
        /* __builtin_abs - arithmetic built-in */
        int abs_val = __builtin_abs(val);
        
        /* __builtin_sqrtf - math built-in (using float version) */
        float sqrt_val = __builtin_sqrtf((float)(abs_val > 0 ? abs_val : 1));
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        result += (int)sqrt_val + abs_val;
    }
    
    return result;
}

/* Function with explicit visibility and nothrow attributes */
static int __attribute__((visibility("hidden"), nothrow, noinline, used))
test_attributed_function(volatile int x) {
    /* Use __builtin_expect for branch prediction */
    if (__builtin_expect(x > 100, 0)) {
        return __builtin_clz(x);  /* Count leading zeros */
    }
    
    /* Use __builtin_popcount for bit operations */
    return __builtin_popcount(x);
}

/* Function using overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    bool overflow;
    
    /* __builtin_add_overflow */
    int sum;
    if (__builtin_add_overflow(a, b, &sum)) {
        result += 1;
    } else {
        result += sum;
    }
    
    /* __builtin_mul_overflow */
    int product;
    if (__builtin_mul_overflow(a, b, &product)) {
        result += 2;
    } else {
        result += product;
    }
    
    /* __builtin_sub_overflow */
    int diff;
    if (__builtin_sub_overflow(a, b, &diff)) {
        result += 4;
    } else {
        result += diff;
    }
    
    return result;
}

/* Function with static linkage using various built-ins */
static int __attribute__((noinline))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int x = seed;
    int result = 0;
    
    /* Chain multiple built-in calls */
    result += __builtin_popcount(x);
    result += __builtin_clz(x | 1);  /* Ensure non-zero */
    result += __builtin_ctz(x | 1);  /* Count trailing zeros */
    result += __builtin_ffs(x);      /* Find first set bit */
    
    /* __builtin_bswap32 for byte swapping */
    result += __builtin_bswap32(x) & 0xFF;
    
    return result;
}

/* External-like function defined later in the same file */
int __attribute__((visibility("default")))
external_builtin_user(int x) {
    /* Use __builtin_unreachable for unreachable code */
    if (x < 0) {
        __builtin_unreachable();  /* Compiler hint */
    }
    
    /* Use __builtin_constant_p to check if value is constant */
    if (__builtin_constant_p(x)) {
        return __builtin_abs(x);
    }
    
    /* __builtin_return_address for debugging built-in */
    void* ra = __builtin_return_address(0);
    return x + ((long)ra & 0xF);  /* Mix in some bits from return address */
}

/* Function using built-ins with assembly barriers */
static int __attribute__((noinline, used))
test_builtin_with_barriers(volatile int x) {
    int result = x;
    
    /* Memory barrier between built-in calls */
    asm volatile("" : : : "memory");
    
    result = __builtin_abs(result);
    
    asm volatile("" : : : "memory");
    
    /* __builtin_expect with result */
    if (__builtin_expect(result > 1000, 0)) {
        result = __builtin_clz(result);
    }
    
    asm volatile("" : : : "memory");
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    volatile int seed = argc;
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(seed * 13 + 7);
    
    /* Test 3: Overflow checking built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Attributed function with visibility control */
    checksum += test_attributed_function(seed * 3);
    
    /* Test 5: External-like function with built-ins */
    checksum += external_builtin_user(seed * 5);
    
    /* Test 6: Built-ins with optimization barriers */
    checksum += test_builtin_with_barriers(seed * 11);
    
    /* Additional direct built-in usage in main */
    volatile int direct_result = 0;
    
    /* __builtin_constant_p usage */
    if (!__builtin_constant_p(seed)) {
        direct_result += __builtin_popcount(checksum);
    }
    
    /* __builtin_expect for main's control flow */
    if (__builtin_expect(checksum > 0, 1)) {
        direct_result += __builtin_abs(checksum);
    }
    
    /* Final result that can't be optimized away */
    printf("Result: %d (checksum: %d)\n", direct_result, checksum);
    
    /* Use __builtin_trap for abnormal exit condition */
    if (direct_result < 0) {
        __builtin_trap();
    }
    
    return direct_result > 1000 ? 0 : 1;
}

/* Additional static function using built-in in unreachable code */
static void __attribute__((used, noinline))
unreachable_builtin_test(void) {
    /* This code is meant to be unreachable but declared used */
    volatile int x = 0;
    
    if (x) {  /* Always false, but compiler might not know */
        /* Use various built-ins in "unreachable" code */
        int y = __builtin_abs(-10);
        y += __builtin_clz(1);
        y += __builtin_popcount(0xFF);
        
        /* Force emission with asm */
        asm volatile("" : "+r" (y) : : "memory");
    }
}
