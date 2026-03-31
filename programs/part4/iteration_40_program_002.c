/* test_builtin_hooks.c - Comprehensive test for built-in function declaration hooks */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_seed = 42;
static volatile int volatile_result = 0;

/* ========== Pattern 1: Basic built-in usage with volatile context ========== */

/* Function with multiple attributes that may interact with hook */
static int __attribute__((used, noinline, noclone))
test_builtin_arithmetic(volatile int seed)
{
    int result = 0;
    
    /* Use volatile counter to prevent loop optimization */
    volatile int i;
    for (i = 0; i < 3; i++) {
        /* Mix different built-ins in non-optimizable expressions */
        int val = seed + i;
        
        /* __builtin_abs with volatile input */
        int abs_val = __builtin_abs(val);
        
        /* __builtin_expect to influence branching */
        if (__builtin_expect(abs_val > 100, 0)) {
            /* __builtin_sqrtf with type conversion */
            float sqrt_val = __builtin_sqrtf((float)abs_val);
            result += (int)sqrt_val;
        } else {
            /* __builtin_ffs to find first set bit */
            result += __builtin_ffs(abs_val);
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* ========== Pattern 2: Bit manipulation built-ins ========== */

/* Static function with hidden visibility attribute */
static int __attribute__((visibility("hidden"), nothrow, noinline))
test_builtin_bitops(unsigned int pattern)
{
    unsigned int x = pattern ^ 0xDEADBEEF;
    int count = 0;
    
    /* Chain multiple bit built-in operations */
    count += __builtin_popcount(x);
    count += __builtin_clz(x | 1);  /* Ensure non-zero */
    count += __builtin_ctz(x | 1);  /* Ensure non-zero */
    count += __builtin_parity(x);
    
    /* Use __builtin_bswap for endianness operations */
    unsigned int swapped = __builtin_bswap32(x);
    count += __builtin_popcount(swapped);
    
    /* Store to volatile to ensure side effect */
    volatile_result = count;
    
    return count;
}

/* ========== Pattern 3: Overflow checking built-ins ========== */

/* Function with artificial control flow */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b)
{
    int result = 0;
    bool overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        /* __builtin_unreachable in dead branch */
        if (result == INT_MAX) {
            __builtin_unreachable();  /* Should never happen */
        }
        result = a;
    }
    
    /* Test multiplication overflow */
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        overflow = true;
        /* Use __builtin_constant_p */
        if (__builtin_constant_p(a)) {
            result /= 2;
        }
    } else {
        overflow = false;
        result += mul_result;
    }
    
    /* Test subtraction overflow */
    int sub_result;
    __builtin_sub_overflow(a, b, &sub_result);
    result += sub_result;
    
    /* Complex expression with __builtin_choose_expr */
    result = __builtin_choose_expr(
        sizeof(int) == 4,
        result * 2,
        result
    );
    
    return result;
}

/* ========== Pattern 4: Function with explicit attributes ========== */

/* Declaration matching the uncovered lines' characteristics */
static void __attribute__((visibility("hidden"), nothrow, used, artificial))
artificial_builtin_user(void)
{
    /* Use __builtin_assume_aligned */
    int array[4] = {1, 2, 3, 4};
    int *aligned_ptr = __builtin_assume_aligned(array, 16);
    
    /* Use __builtin_prefetch */
    __builtin_prefetch(aligned_ptr, 0, 3);
    
    /* Complex built-in combination */
    int x = volatile_seed;
    int y = __builtin_abs(x);
    int z = __builtin_clz(y | 1);
    
    /* Use __builtin_expect with pointer */
    int *ptr = &volatile_seed;
    if (__builtin_expect(ptr != NULL, 1)) {
        *ptr += z;
    }
}

/* ========== Pattern 5: External linkage simulation ========== */

/* External declaration (defined later in same file) */
extern int external_builtin_helper(int x) __attribute__((visibility("hidden")));

/* Function using external declaration */
static int test_external_linkage(int val)
{
    /* Call externally declared function */
    int result = external_builtin_helper(val);
    
    /* Use __builtin_classify_type */
    int type = __builtin_classify_type(result);
    
    /* Use __builtin_constant_p with external result */
    if (!__builtin_constant_p(result)) {
        result += type;
    }
    
    return result;
}

/* ========== External function definition ========== */

/* This simulates a function from another translation unit */
int __attribute__((visibility("hidden"), nothrow))
external_builtin_user(int x)
{
    /* Use __builtin_unreachable in conditional */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Multiple built-ins in sequence */
    int y = __builtin_abs(x);
    int z = __builtin_popcount(y);
    
    /* Use __builtin_expect with the result */
    return __builtin_expect(z > 0, 1) ? z : 1;
}

/* Another external-like function */
int __attribute__((visibility("hidden")))
external_builtin_helper(int x)
{
    /* Use __builtin_add_overflow_p (C++ built-in available in C with GCC) */
    int result = x;
    
    /* Overflow check without storing result */
    if (__builtin_add_overflow_p(x, 100, int)) {
        result = 100;
    } else {
        result += 100;
    }
    
    /* Use __builtin_rotateleft32 (GCC 13+) */
    #if __GNUC__ >= 13
    result = __builtin_rotateleft32(result, 3);
    #endif
    
    return result;
}

/* ========== Hidden visibility function definition ========== */

void __attribute__((visibility("hidden"), used))
hidden_visibility_func(void)
{
    /* Use various built-ins */
    int x = volatile_seed;
    
    /* __builtin_nan */
    double nan_val = __builtin_nan("");
    
    /* __builtin_inf */
    double inf_val = __builtin_inf();
    
    /* Use the values to prevent elimination */
    volatile_seed += (int)nan_val * 0;
    volatile_seed += (int)inf_val * 0;
    
    /* __builtin_isinf */
    if (__builtin_isinf(inf_val)) {
        volatile_result = 1;
    }
}

/* ========== Main function with comprehensive testing ========== */

int main(int argc, char *argv[])
{
    int checksum = 0;
    
    /* Initialize from argc to make it runtime-dependent */
    volatile_seed = argc * 17 + 1;
    
    /* Pattern 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(volatile_seed);
    
    /* Pattern 2: Bit operation built-ins */
    checksum += test_builtin_bitops(volatile_seed);
    
    /* Pattern 3: Overflow built-ins */
    checksum += test_builtin_overflow(volatile_seed, 7);
    
    /* Pattern 4: Call attributed function */
    artificial_builtin_user();
    checksum += volatile_seed;
    
    /* Pattern 5: External linkage simulation */
    checksum += test_external_linkage(volatile_seed);
    
    /* Call hidden visibility function */
    hidden_visibility_func();
    checksum += volatile_result;
    
    /* Use __builtin_return_address */
    void *return_addr = __builtin_return_address(0);
    checksum += (unsigned long)return_addr & 0xFF;
    
    /* Use __builtin_frame_address */
    void *frame_addr = __builtin_frame_address(0);
    checksum += (unsigned long)frame_addr & 0xFF;
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional global declaration with attributes */
int __attribute__((used, visibility("hidden")))
global_builtin_var = __builtin_abs(-10);

/* Function with __builtin_constant_p in static initializer */
static int __attribute__((used))
constant_p_init(void)
{
    return __builtin_constant_p(42) ? 42 : 0;
}

/* Static variable initialized with built-in result */
static int static_init = constant_p_init();
