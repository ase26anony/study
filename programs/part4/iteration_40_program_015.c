/* test_builtin_hooks.c - Comprehensive test for built-in function declaration hooks */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 42;
volatile int g_volatile_result = 0;

/* ====== Pattern 1: Direct built-in usage with volatile barriers ====== */

/* Function with explicit hidden visibility and nothrow attributes */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_builtin_wrapper(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        /* Use __builtin_abs with volatile read to prevent constant folding */
        volatile int v = x;
        return __builtin_abs(v);
    }
    return 0;
}

/* Non-inlinable function containing multiple built-ins */
int __attribute__((noinline, noclone))
test_builtin_arithmetic(int seed) {
    volatile int acc = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 3; i = i + 1) {
        /* Use different math built-ins */
        int val = seed + (int)i;
        acc += __builtin_abs(val);
        
        /* Use sqrtf built-in with float conversion */
        float fval = (float)val;
        acc += (int)__builtin_sqrtf(fval);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return acc;
}

/* ====== Pattern 2: Bit manipulation built-ins ====== */

int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    volatile unsigned int result = 0;
    
    /* Use population count built-in */
    result += __builtin_popcount(seed);
    
    /* Use count leading zeros built-in */
    if (seed != 0) {
        result += __builtin_clz(seed);
    }
    
    /* Use count trailing zeros built-in */
    result += __builtin_ctz(seed | 1);  /* |1 to avoid undefined behavior for 0 */
    
    /* Use byte swap built-in */
    result += __builtin_bswap32(seed) & 0xFF;
    
    return (int)result;
}

/* ====== Pattern 3: Overflow checking built-ins ====== */

int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    volatile int overflow_result = 0;
    int sum, diff, prod;
    bool overflow;
    
    /* Addition overflow check */
    overflow = __builtin_add_overflow(a, b, &sum);
    overflow_result += overflow ? 1 : 0;
    overflow_result += sum;
    
    /* Subtraction overflow check */
    overflow = __builtin_sub_overflow(a, b, &diff);
    overflow_result += overflow ? 1 : 0;
    overflow_result += diff;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(a, b, &prod);
    overflow_result += overflow ? 1 : 0;
    overflow_result += prod;
    
    /* Use __builtin_expect with the overflow result */
    if (__builtin_expect(overflow_result > 1000, 0)) {
        asm volatile("" : : : "memory");
    }
    
    return overflow_result;
}

/* ====== Pattern 4: Function with complex built-in usage ====== */

/* Declare with both used and visibility attributes */
static int __attribute__((used, visibility("hidden")))
complex_builtin_function(int x, int y) {
    volatile int temp = x;
    
    /* Chain multiple built-ins */
    int abs_x = __builtin_abs(temp);
    int abs_y = __builtin_abs(y);
    
    /* Use __builtin_add_overflow_p for predicate overflow check */
    if (__builtin_add_overflow_p(abs_x, abs_y, int)) {
        /* Use __builtin_unreachable in dead code path */
        if (abs_x > INT_MAX) {
            __builtin_unreachable();
        }
        return -1;
    }
    
    /* Use __builtin_assume_aligned for alignment hint */
    int* ptr = &abs_x;
    ptr = (int*)__builtin_assume_aligned(ptr, 4);
    
    return abs_x + abs_y;
}

/* ====== Pattern 5: External linkage simulation ====== */

/* External declaration (defined later in same file) */
extern int external_builtin_user(int x);

/* Static function calling external function */
static int __attribute__((noinline))
call_external_builtin(int x) {
    return external_builtin_user(x);
}

/* ====== Pattern 6: Built-in in conditional unreachable ====== */

void __attribute__((nothrow))
handle_error(void) {
    /* Empty error handler */
}

int __attribute__((noinline, noclone))
test_builtin_unreachable(int x) {
    /* Use __builtin_unreachable in conditional */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use __builtin_expect in switch */
    switch (__builtin_expect(x, 10)) {
        case 0:
            return 1;
        case 10:
            return 2;
        default:
            return 3;
    }
}

/* ====== External function definition ====== */

/* This simulates a function from another translation unit */
int __attribute__((visibility("hidden")))
external_builtin_user(int x) {
    /* Use __builtin_ffs (find first set) */
    int result = __builtin_ffs(x);
    
    /* Use __builtin_constant_p to check if constant */
    if (__builtin_constant_p(x)) {
        result += 1000;
    }
    
    /* Use __builtin_trap in extreme case */
    if (result < 0) {
        __builtin_trap();
    }
    
    return result;
}

/* ====== Main test driver ====== */

int main(int argc, char** argv) {
    volatile int seed = g_volatile_seed + argc;
    int checksum = 0;
    
    /* Pattern 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Pattern 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Pattern 3: Overflow checking built-ins */
    checksum += test_builtin_overflow(seed, 7);
    
    /* Pattern 4: Complex built-in function */
    checksum += complex_builtin_function(seed, -seed);
    
    /* Pattern 5: External linkage */
    checksum += call_external_builtin(seed);
    
    /* Pattern 6: Unreachable built-in */
    checksum += test_builtin_unreachable(seed);
    
    /* Pattern 7: Hidden visibility wrapper */
    checksum += hidden_builtin_wrapper(seed);
    
    /* Use __builtin_printf if available (GCC extension) */
#ifdef __GNUC__
    checksum += __builtin_printf("Checksum: %d\n", checksum);
#else
    printf("Checksum: %d\n", checksum);
#endif
    
    /* Final memory barrier */
    asm volatile("" : : : "memory");
    
    /* Store to volatile global to ensure all results are used */
    g_volatile_result = checksum;
    
    return checksum & 0xFF;
}

/* ====== Additional static functions with attributes ====== */

/* Static function with artificial-like behavior */
static int __attribute__((used, visibility("hidden")))
static_builtin_helper(int x) {
    /* Use __builtin_clrsb (count leading redundant sign bits) */
    int clrsb = __builtin_clrsb(x);
    
    /* Use __builtin_parity (parity of bits) */
    int parity = __builtin_parity(x);
    
    return clrsb + parity;
}

/* Function that's never called but marked used */
int __attribute__((used, visibility("hidden")))
unused_but_present(void) {
    /* Use various built-ins */
    int x = __builtin_abs(-5);
    x += __builtin_ctz(32);
    x += __builtin_expect(x > 0, 1) ? 1 : 0;
    
    return x;
}
