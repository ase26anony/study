/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int vol_global = 0;
static volatile unsigned int vol_seed = 12345;

/* ========== Helper Functions with Various Attributes ========== */

/* Function with explicit hidden visibility and nothrow attribute */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Function marked as used to prevent elimination */
static int __attribute__((used, noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile float fval = (float)seed;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        /* Use math built-ins */
        result += __builtin_abs(seed + i);
        result += (int)__builtin_sqrtf(fval + i);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Function with bit operation built-ins */
static unsigned int __attribute__((noinline))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Use various bit manipulation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Use built-in with computed argument */
    unsigned int rotated = __builtin_rotateright32(seed, 3);
    result += __builtin_popcount(rotated);
    
    return result;
}

/* Function with overflow checking built-ins */
static int __attribute__((noinline))
test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    int overflow_result;
    bool overflow_flag;
    
    /* Test addition overflow */
    overflow_flag = __builtin_add_overflow(a, b, &overflow_result);
    result += overflow_result;
    result += overflow_flag ? 1 : 0;
    
    /* Test multiplication overflow */
    overflow_flag = __builtin_mul_overflow(a, b, &overflow_result);
    result += overflow_result;
    result += overflow_flag ? 1 : 0;
    
    /* Test subtraction overflow */
    overflow_flag = __builtin_sub_overflow(a, b, &overflow_result);
    result += overflow_result;
    result += overflow_flag ? 1 : 0;
    
    return result;
}

/* Function that uses __builtin_unreachable */
static int __attribute__((noinline))
test_builtin_unreachable(volatile int x) {
    switch (x & 3) {
        case 0: return x + 1;
        case 1: return x * 2;
        case 2: return x - 1;
        default: 
            /* This should never happen with x & 3 */
            __builtin_unreachable();
    }
}

/* Function with artificial control flow using __builtin_expect */
static int __attribute__((noinline))
test_builtin_expect(volatile int x) {
    int result = 0;
    
    /* Use __builtin_expect in conditionals */
    if (__builtin_expect(x > 100, 0)) {
        result = __builtin_abs(x);
    } else if (__builtin_expect(x < -100, 0)) {
        result = __builtin_abs(x) * 2;
    } else {
        result = x;
    }
    
    /* Use __builtin_expect_with_probability */
    if (__builtin_expect_with_probability(result != 0, 1, 0.9)) {
        result += 100;
    }
    
    return result;
}

/* Function that declares and uses built-in prototypes */
static int __attribute__((noinline))
test_builtin_prototypes(volatile int x) {
    /* Declare built-in function prototype */
    int __builtin_popcount(unsigned int);
    int __builtin_clz(unsigned int);
    int __builtin_ctz(unsigned int);
    
    int result = 0;
    unsigned int ux = (unsigned int)x;
    
    /* Call declared built-ins */
    result += __builtin_popcount(ux);
    result += __builtin_clz(ux | 1);
    result += __builtin_ctz(ux | 1);
    
    return result;
}

/* ========== External Function Definitions ========== */

/* External function that uses built-ins (simulates cross-file reference) */
int external_builtin_user(int x) {
    int result = 0;
    
    /* Use various built-ins */
    result += __builtin_abs(x);
    result += __builtin_parity((unsigned int)x);
    
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        result += 1000;
    } else {
        result += __builtin_clz((unsigned int)(x > 0 ? x : -x) | 1);
    }
    
    return result;
}

/* External function with hidden visibility attribute */
void __attribute__((visibility("hidden"), nothrow, used))
external_visibility_func(void) {
    /* Use built-in with side effect prevention */
    volatile int x = vol_global;
    int y = __builtin_abs(x);
    
    /* Ensure the value is used */
    asm volatile("" : : "r"(y) : "memory");
}

/* ========== Main Test Function ========== */

int main(int argc, char *argv[]) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int checksum = 0;
    
    /* Update volatile seed */
    vol_seed = seed;
    vol_global = seed;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow checking built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Hidden visibility function with built-ins */
    checksum += hidden_visibility_func(seed);
    
    /* Test 5: External function with built-ins */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Built-in unreachable */
    checksum += test_builtin_unreachable(seed);
    
    /* Test 7: Built-in expect */
    checksum += test_builtin_expect(seed);
    
    /* Test 8: Built-in prototypes */
    checksum += test_builtin_prototypes(seed);
    
    /* Call external visibility function */
    external_visibility_func();
    
    /* Additional built-in usage in main */
    checksum += __builtin_ffs(checksum | 1);
    
    /* Use __builtin_assume_aligned */
    int aligned_buffer[4] __attribute__((aligned(16)));
    int *ptr = __builtin_assume_aligned(aligned_buffer, 16);
    checksum += ptr[0];
    
    /* Final result output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional static function that might trigger the hook */
static void __attribute__((constructor, used))
init_function(void) {
    /* Use built-in in constructor */
    vol_global = __builtin_abs(-100);
}
