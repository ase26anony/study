/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_seed;
static volatile int volatile_result;

/* ========== Helper Functions with Various Attributes ========== */

/* Function with hidden visibility, nothrow, and used attributes */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_attributed_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Noinline function to prevent optimization */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(int seed) {
    volatile int acc = 0;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (volatile int i = 0; i < 5; i++) {
        /* Use various arithmetic built-ins */
        int val = seed + i;
        acc += __builtin_abs(val);
        
        /* Use floating point built-in */
        float fval = (float)val;
        acc += (int)__builtin_sqrtf(fval);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return acc;
}

static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int result = 0;
    
    /* Bit manipulation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Use in conditional */
    if (__builtin_parity(seed)) {
        result ^= 0xFF;
    }
    
    volatile_result = result;
    return result;
}

static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    int result = 0;
    int overflow;
    
    /* Overflow checking built-ins */
    if (__builtin_add_overflow(a, b, &result)) {
        result = INT_MAX;
    }
    
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        result = INT_MAX;
    } else {
        result += mul_result;
    }
    
    /* Use __builtin_expect with overflow result */
    if (__builtin_expect(overflow, 0)) {
        result = -1;
    }
    
    return result;
}

/* Function with artificial control flow using builtins */
static void __attribute__((noinline))
test_builtin_control_flow(int x) {
    /* __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* __builtin_prefetch */
    int array[100];
    __builtin_prefetch(&array[x % 100], 0, 0);
    
    /* __builtin_trap in unreachable path */
    if (x > 1000) {
        __builtin_trap();
    }
}

/* ========== External Function Definitions ========== */

/* This simulates external linkage processing */
int external_builtin_user(int x) {
    /* Use multiple built-ins in complex expression */
    int result = __builtin_abs(x) + 
                 __builtin_popcount((unsigned int)x) +
                 (__builtin_expect(x > 0, 1) ? 1 : 0);
    
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        return 0;
    }
    
    return result;
}

void external_visibility_func(void) {
    /* Function with hidden visibility attribute */
    volatile int x = 10;
    int result = __builtin_bswap16(x);
    volatile_result = result;
}

/* ========== Main Test Function ========== */

int main(int argc, char *argv[]) {
    /* Initialize volatile seed from argc to prevent constant folding */
    volatile_seed = argc;
    int seed = volatile_seed;
    int checksum = 0;
    
    printf("Testing GCC built-in functions for targhooks.cc coverage\n");
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Attributed function with built-ins */
    checksum += hidden_attributed_func(seed);
    
    /* Test 5: Control flow built-ins */
    test_builtin_control_flow(seed);
    
    /* Test 6: External function with built-ins */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Visibility attributed function */
    external_visibility_func();
    
    /* Test 8: Direct built-in declarations and usage */
    {
        /* Declare built-in function prototype */
        int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
        
        volatile unsigned int v = seed * 7;
        checksum += __builtin_popcount(v);
        checksum += __builtin_ffs(v);
        
        /* Use __builtin_choose_expr */
        checksum += __builtin_choose_expr(
            sizeof(int) == 4,
            __builtin_clz(v | 1),
            0
        );
    }
    
    /* Test 9: String built-ins */
    {
        char buffer[100] = {0};
        const char *src = "test";
        __builtin_memcpy(buffer, src, __builtin_strlen(src) + 1);
        checksum += buffer[0];
    }
    
    /* Test 10: Type traits built-ins */
    {
        checksum += __builtin_types_compatible_p(int, unsigned int);
        checksum += __builtin_constant_p(seed) ? 0 : 1;
    }
    
    /* Ensure all results are used */
    printf("Final checksum: %d\n", checksum);
    
    /* Use __builtin_return_address */
    void *ra = __builtin_return_address(0);
    checksum += (unsigned long)ra & 0xFF;
    
    return checksum & 0xFF;
}

/* Additional static function that might trigger declaration processing */
static int __attribute__((unused, visibility("hidden")))
unused_static_with_builtin(int x) {
    return __builtin_abs(x) + __builtin_popcount(x);
}
