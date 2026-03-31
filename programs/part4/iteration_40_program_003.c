/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_func(void) __attribute__((visibility("hidden")));

/* Prevent optimization of helper functions */
#define NO_OPT __attribute__((noinline, noclone))

/* Test 1: Arithmetic built-ins with volatile barriers */
NO_OPT static int test_builtin_arithmetic(volatile int seed) {
    volatile int result = 0;
    
    /* Use various arithmetic built-ins */
    for (volatile int i = 0; i < 5; i++) {
        int val = seed + i * 100;
        
        /* __builtin_abs with volatile input */
        int abs_val = __builtin_abs(val);
        
        /* __builtin_sqrtf with float conversion */
        float sqrt_val = __builtin_sqrtf((float)(abs_val + 1));
        
        /* __builtin_expect to influence branching */
        if (__builtin_expect(val > 0, 1)) {
            result += (int)sqrt_val;
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Test 2: Bit manipulation built-ins */
NO_OPT static int test_builtin_bitops(unsigned int seed) {
    volatile unsigned int accum = 0;
    
    /* __builtin_popcount on varying values */
    accum += __builtin_popcount(seed);
    accum += __builtin_popcount(seed ^ 0xAAAAAAAA);
    
    /* __builtin_clz with zero check */
    if (seed != 0) {
        accum += __builtin_clz(seed);
    }
    
    /* __builtin_ctz */
    accum += __builtin_ctz(seed | 1);  // Ensure non-zero
    
    /* __builtin_ffs */
    accum += __builtin_ffs(seed);
    
    /* __builtin_parity */
    accum += __builtin_parity(seed);
    
    return accum;
}

/* Test 3: Overflow checking built-ins */
NO_OPT static int test_builtin_overflow(int a, int b) {
    volatile int result = 0;
    int overflow;
    
    /* __builtin_add_overflow */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result += 1;
    } else {
        result += overflow;
    }
    
    /* __builtin_mul_overflow */
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += 2;
    } else {
        result += overflow;
    }
    
    /* __builtin_sub_overflow */
    if (__builtin_sub_overflow(a, b, &overflow)) {
        result += 4;
    } else {
        result += overflow;
    }
    
    return result;
}

/* Test 4: Function with explicit attributes that calls built-ins */
__attribute__((visibility("hidden"), nothrow, used))
NO_OPT static int attributed_builtin_user(int x) {
    /* This function should be marked with DECL_ARTIFICIAL and other flags */
    volatile int y = x;
    
    /* Use __builtin_expect within attributed function */
    if (__builtin_expect(y > 100, 0)) {
        return __builtin_abs(y);
    }
    
    /* Use __builtin_unreachable for impossible paths */
    if (y < 0) {
        __builtin_unreachable();
    }
    
    return y * 2;
}

/* Test 5: External linkage simulation */
/* First declare it extern */
extern int external_builtin_helper(int x) __attribute__((visibility("hidden")));

/* Then define it (simulating separate compilation unit) */
__attribute__((visibility("hidden"), nothrow))
NO_OPT int external_builtin_helper(int x) {
    volatile int result = 0;
    
    /* Use multiple built-ins */
    result += __builtin_popcount(x);
    
    /* __builtin_assume_aligned */
    int* ptr = &x;
    ptr = (int*)__builtin_assume_aligned(ptr, 4);
    
    /* __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        result += 10;
    }
    
    /* Conditional __builtin_unreachable */
    if (x == INT_MAX) {
        __builtin_unreachable();
    }
    
    return result;
}

/* Another externally visible function using built-ins */
__attribute__((used, visibility("hidden")))
NO_OPT void external_visibility_func(void) {
    volatile int x = 42;
    
    /* __builtin_trap under condition */
    if (x < 0) {
        __builtin_trap();
    }
    
    /* __builtin_prefetch */
    __builtin_prefetch(&x, 0, 3);
    
    asm volatile("" : : : "memory");
}

/* Test 6: Complex expression with built-ins */
NO_OPT static int test_complex_expressions(volatile int seed) {
    int a = seed;
    int b = seed * 2;
    int c = seed / 2;
    
    /* Nested built-in calls in complex expressions */
    int result = __builtin_abs(a) + 
                 __builtin_popcount((unsigned int)b) * 
                 (__builtin_expect(c > 0, 1) ? 1 : -1);
    
    /* __builtin_add_overflow in loop */
    for (volatile int i = 0; i < 3; i++) {
        int sum;
        if (__builtin_add_overflow(result, i, &sum)) {
            result = __builtin_abs(result - i);
        } else {
            result = sum;
        }
    }
    
    return result;
}

/* Test 7: Built-ins with floating point */
NO_OPT static float test_float_builtins(float seed) {
    volatile float result = seed;
    
    /* __builtin_fabsf */
    result = __builtin_fabsf(result);
    
    /* __builtin_copysignf */
    result = __builtin_copysignf(result, -1.0f);
    
    /* __builtin_isinf and __builtin_isnan */
    if (!__builtin_isinf(result) && !__builtin_isnan(result)) {
        /* __builtin_floorf */
        result = __builtin_floorf(result + 0.5f);
    }
    
    return result;
}

/* Main function that ties everything together */
int main(int argc, char* argv[]) {
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)argc;
    volatile float fseed = (float)argc;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(useed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed + 100);
    
    /* Test 4: Attributed function with built-ins */
    checksum += attributed_builtin_user(seed);
    
    /* Test 5: External linkage functions */
    checksum += external_builtin_helper(seed);
    external_visibility_func();
    
    /* Test 6: Complex expressions */
    checksum += test_complex_expressions(seed);
    
    /* Test 7: Float built-ins */
    checksum += (int)test_float_builtins(fseed);
    
    /* Additional direct built-in usage in main */
    volatile int direct_result = 0;
    
    /* __builtin_constant_p with argc (non-constant) */
    if (!__builtin_constant_p(argc)) {
        direct_result += __builtin_popcount(useed);
    }
    
    /* __builtin_expect for branch prediction */
    if (__builtin_expect(argc > 1, 0)) {
        direct_result += __builtin_clz(useed | 1);
    }
    
    /* __builtin_unreachable for impossible case */
    if (argc < 0) {
        __builtin_unreachable();
    }
    
    checksum += direct_result;
    
    /* Ensure result is used */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional global declaration with attributes */
__attribute__((constructor, visibility("hidden"), used))
static void init_builtin_test(void) {
    volatile int x = 1;
    /* Use built-in in constructor */
    if (__builtin_expect(x > 0, 1)) {
        x = __builtin_abs(x);
    }
}
