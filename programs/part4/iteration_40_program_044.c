/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* Prevent optimization of helper functions */
#define NOOPT __attribute__((noinline, noclone))

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void);

/* Volatile variables to prevent optimization */
static volatile int vol_seed = 42;
static volatile int vol_result = 0;

/* ====== Pattern 1: Direct built-in usage with volatile barriers ====== */
NOOPT int test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int barrier;
    
    /* Use various arithmetic built-ins */
    result += __builtin_abs(seed - 100);
    barrier = result;
    
    /* Use in loop with volatile counter */
    for (volatile int i = 0; i < 3; i++) {
        result += __builtin_abs(seed + i * 10);
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    /* Floating point built-in */
    float fval = (float)seed * 0.5f;
    result += (int)__builtin_sqrtf(fval);
    
    return result;
}

/* ====== Pattern 2: Bit manipulation built-ins ====== */
NOOPT unsigned test_builtin_bitops(unsigned seed) {
    unsigned result = 0;
    volatile unsigned vseed = seed;
    
    /* Multiple bit operation built-ins */
    result = __builtin_popcount(vseed);
    result += __builtin_clz(vseed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(vseed | 1);
    result += __builtin_ffs(vseed);
    
    /* Use in conditional */
    if (__builtin_parity(vseed)) {
        result += 100;
    }
    
    return result;
}

/* ====== Pattern 3: Overflow checking built-ins ====== */
NOOPT int test_builtin_overflow(int a, int b) {
    int result = 0;
    volatile int overflow_flag;
    
    /* Test various overflow built-ins */
    int sum;
    if (__builtin_add_overflow(a, b, &sum)) {
        result = INT_MAX;
    } else {
        result = sum;
    }
    overflow_flag = result;
    
    int product;
    if (__builtin_mul_overflow(a, b/2 + 1, &product)) {
        result += INT_MIN;
    }
    
    long long lsum;
    __builtin_add_overflow((long long)a, (long long)b * 2, &lsum);
    result += (int)lsum;
    
    return result;
}

/* ====== Pattern 4: Function with explicit attributes ====== */
/* Declaration with attributes matching target flags */
static int __attribute__((visibility("hidden"), nothrow, used))
attributed_builtin_user(int x) {
    /* Use built-in inside attributed function */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x) + __builtin_clz(x | 1);
    }
    return 0;
}

NOOPT int test_attributed_function(int seed) {
    /* Call the attributed function */
    int r1 = attributed_builtin_user(seed);
    
    /* Declare and use another built-in with attributes */
    int __attribute__((used)) r2 = __builtin_bswap32(seed);
    
    /* Combine with inline assembly barrier */
    asm volatile("" : "+r"(r2) : : "memory");
    
    return r1 + r2;
}

/* ====== Pattern 5: External linkage simulation ====== */
/* Forward declaration */
extern int external_builtin_helper(int x) __attribute__((visibility("hidden")));

/* Definition with built-in usage */
int external_builtin_helper(int x) {
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();  /* Mark path as unreachable */
    }
    
    /* Use other built-ins */
    int result = __builtin_abs(x);
    
    /* String built-in */
    char buffer[20];
    int len = __builtin_snprintf(buffer, sizeof(buffer), "%d", result);
    result += len;
    
    return result;
}

NOOPT int test_external_linkage(int seed) {
    /* Call externally declared function */
    int r1 = external_builtin_helper(seed);
    
    /* Direct built-in call with external linkage */
    extern int __builtin_popcountll(unsigned long long);
    unsigned long long large = (unsigned long long)seed * 1000;
    int r2 = __builtin_popcountll(large);
    
    return r1 + r2;
}

/* ====== Pattern 6: Complex expression with multiple built-ins ====== */
NOOPT int test_complex_expressions(int seed) {
    volatile int v = seed;
    int result = 0;
    
    /* Nested built-in calls */
    result = __builtin_abs(__builtin_abs(v) - 50);
    
    /* Built-in in ternary expression */
    result += __builtin_expect(v > 0, 1) ? 
              __builtin_clz(v) : 
              __builtin_ctz(-v | 1);
    
    /* Built-in with side effects */
    int tmp;
    __builtin_prefetch(&vol_result, 0, 3);
    result += __builtin_ffs(v) * 2;
    
    /* Memory built-in */
    __builtin_memset(&tmp, v & 0xFF, sizeof(tmp));
    result += tmp;
    
    return result;
}

/* ====== Pattern 7: Static function with built-ins ====== */
static NOOPT int static_builtin_wrapper(int x) {
    /* Multiple built-ins in static function */
    int a = __builtin_abs(x);
    int b = __builtin_popcount(a);
    int c = __builtin_clz(a | 1);
    
    /* Use in switch with built-in */
    switch (__builtin_expect(a % 4, 0)) {
        case 0: return b;
        case 1: return c;
        case 2: return __builtin_ffs(a);
        default: return __builtin_parity(a);
    }
}

/* ====== Main function combining all patterns ====== */
int main(int argc, char *argv[]) {
    int final_result = 0;
    volatile int seed = argc > 1 ? argv[1][0] : vol_seed;
    
    /* Execute all test patterns */
    final_result += test_builtin_arithmetic(seed);
    final_result += test_builtin_bitops(seed);
    final_result += test_builtin_overflow(seed, seed * 2);
    final_result += test_attributed_function(seed);
    final_result += test_external_linkage(seed);
    final_result += test_complex_expressions(seed);
    final_result += static_builtin_wrapper(seed);
    
    /* Additional direct built-in calls in main */
    final_result += __builtin_abs(seed - 100);
    final_result += __builtin_popcount(final_result);
    
    /* Use result to prevent dead code elimination */
    vol_result = final_result;
    
    /* Print result to ensure all code is live */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;  /* Return non-zero result */
}

/* External function definition (simulating another translation unit) */
int external_builtin_user(int x) {
    /* Use visibility attribute */
    __attribute__((visibility("hidden"))) static int counter = 0;
    
    /* Multiple built-ins */
    int result = __builtin_abs(x);
    result += __builtin_clz(result | 1);
    
    /* Mark as used */
    __attribute__((used)) int unused_var = __builtin_bswap16(result);
    
    return result + counter++;
}

/* Hidden visibility function definition */
void __attribute__((visibility("hidden"), nothrow))
hidden_visibility_func(void) {
    /* Use built-in in hidden function */
    volatile int x = 42;
    int r = __builtin_abs(x);
    __builtin_prefetch(&r, 0, 0);
}
