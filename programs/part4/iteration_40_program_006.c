/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_func(void) __attribute__((visibility("hidden")));

/* Prevent optimization */
static volatile int global_volatile = 0;
static int __attribute__((noinline, noclone)) use_result(int x) {
    global_volatile = x;
    return x;
}

/* ========== Pattern 1: Built-in arithmetic with volatile barriers ========== */
static int __attribute__((noinline, noclone)) 
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int counter = seed % 10 + 1;
    
    /* Loop with built-ins to prevent dead code elimination */
    for (volatile int i = 0; i < counter; i++) {
        /* Use multiple arithmetic built-ins */
        int val = seed + i * 7;
        
        /* __builtin_abs - commonly used built-in */
        int abs_val = __builtin_abs(val);
        
        /* __builtin_expect for branch prediction */
        if (__builtin_expect((val % 3) == 0, 0)) {
            /* __builtin_sqrtf for floating point built-in */
            float fval = (float)val;
            float sqrt_val = __builtin_sqrtf(fval);
            result += (int)sqrt_val;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        result += abs_val;
    }
    
    return use_result(result);
}

/* ========== Pattern 2: Bit manipulation built-ins ========== */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int result = 0;
    
    /* __builtin_popcount - population count */
    result += __builtin_popcount(seed);
    
    /* __builtin_clz - count leading zeros */
    if (seed != 0) {
        result += __builtin_clz(seed);
    }
    
    /* __builtin_ctz - count trailing zeros */
    if (seed != 0) {
        result += __builtin_ctz(seed);
    }
    
    /* __builtin_ffs - find first set bit */
    result += __builtin_ffs(seed);
    
    /* __builtin_parity - parity of number of 1-bits */
    result += __builtin_parity(seed);
    
    /* Use bitwise built-ins in expression */
    unsigned int x = seed ^ 0xAAAAAAAA;
    result += __builtin_popcount(x);
    
    return use_result((int)result);
}

/* ========== Pattern 3: Overflow checking built-ins ========== */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int seed) {
    int result = 0;
    int overflow_result;
    
    /* __builtin_add_overflow */
    if (__builtin_add_overflow(seed, INT_MAX / 2, &overflow_result)) {
        result += 1;
    } else {
        result += overflow_result;
    }
    
    /* __builtin_sub_overflow */
    if (__builtin_sub_overflow(seed, INT_MIN / 2, &overflow_result)) {
        result += 2;
    } else {
        result += overflow_result;
    }
    
    /* __builtin_mul_overflow */
    if (__builtin_mul_overflow(seed, 3, &overflow_result)) {
        result += 4;
    } else {
        result += overflow_result;
    }
    
    /* __builtin_sadd_overflow - signed add overflow */
    long long ll_result;
    if (__builtin_saddll_overflow(seed, 1000000000, &ll_result)) {
        result += 8;
    } else {
        result += (int)ll_result;
    }
    
    return use_result(result);
}

/* ========== Pattern 4: Function with explicit attributes ========== */
/* Declare with multiple attributes that should trigger flag setting */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
attributed_builtin_user(int x) {
    /* Use __builtin_expect inside attributed function */
    if (__builtin_expect(x > 0, 1)) {
        /* __builtin_clrsb - count leading redundant sign bits */
        return __builtin_clrsb(x);
    }
    return 0;
}

static int __attribute__((noinline, noclone))
test_attributed_function(int seed) {
    /* Call the attributed function */
    int result = attributed_builtin_user(seed);
    
    /* Also use __builtin_constant_p */
    if (__builtin_constant_p(seed)) {
        result += 10;
    } else {
        result += 20;
    }
    
    /* __builtin_unreachable in controlled context */
    if (seed < 0) {
        __builtin_unreachable();  /* Should not be reached with our inputs */
    }
    
    return use_result(result);
}

/* ========== Pattern 5: External linkage simulation ========== */
/* This function is declared extern earlier and defined here */
int external_builtin_user(int x) {
    /* Use __builtin_unreachable under specific condition */
    if (x == 0) {
        __builtin_unreachable();
    }
    
    /* __builtin_bswap32 - byte swap */
    unsigned int swapped = __builtin_bswap32((unsigned int)x);
    
    /* __builtin_rotateleft32 - rotation built-in */
    unsigned int rotated = __builtin_rotateleft32(swapped, 5);
    
    return (int)rotated;
}

/* Another externally declared function with visibility attribute */
void __attribute__((visibility("hidden"), used))
external_visibility_func(void) {
    /* Use __builtin_trap in attributed function */
    if (global_volatile < 0) {
        __builtin_trap();
    }
    
    /* __builtin_prefetch */
    int *ptr = &global_volatile;
    __builtin_prefetch(ptr, 0, 3);
}

static int __attribute__((noinline, noclone))
test_external_linkage(int seed) {
    int result = 0;
    
    /* Call the externally declared function */
    result += external_builtin_user(seed);
    
    /* Call visibility-attributed function */
    external_visibility_func();
    
    /* Use __builtin_frame_address */
    void *frame_addr = __builtin_frame_address(0);
    result += (int)((long)frame_addr & 0xFF);
    
    return use_result(result);
}

/* ========== Pattern 6: Complex expression with multiple built-ins ========== */
static int __attribute__((noinline, noclone))
test_complex_expressions(int seed) {
    int result = seed;
    
    /* Chain multiple built-ins in a complex expression */
    result = __builtin_abs(
        __builtin_popcount(
            __builtin_bswap32(
                (unsigned int)result
            )
        ) - 
        __builtin_clz(
            (unsigned int)__builtin_abs(result) | 1
        )
    );
    
    /* Use __builtin_choose_expr */
    result = __builtin_choose_expr(
        sizeof(int) == 4,
        __builtin_ffs(result),
        __builtin_popcount(result)
    );
    
    /* __builtin_classify_type */
    int type_class = __builtin_classify_type(result);
    result += type_class;
    
    return use_result(result);
}

/* ========== Main function ========== */
int main(int argc, char *argv[]) {
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    int checksum = 0;
    
    /* Test all patterns */
    checksum += test_builtin_arithmetic(seed);
    checksum += test_builtin_bitops((unsigned int)seed);
    checksum += test_builtin_overflow(seed);
    checksum += test_attributed_function(seed);
    checksum += test_external_linkage(seed);
    checksum += test_complex_expressions(seed);
    
    /* Additional direct built-in usage in main */
    
    /* __builtin_return_address */
    void *ret_addr = __builtin_return_address(0);
    checksum += (int)((long)ret_addr & 0xFF);
    
    /* __builtin_constant_p with conditional */
    if (!__builtin_constant_p(checksum)) {
        checksum += 1000;
    }
    
    /* __builtin_expect for final result */
    if (__builtin_expect(checksum != 0, 1)) {
        /* Use result to prevent elimination */
        global_volatile = checksum;
    }
    
    /* Final output to ensure all code is live */
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;
}
