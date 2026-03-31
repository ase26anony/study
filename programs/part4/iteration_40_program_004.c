/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* Prevent optimization of helper functions */
#define NOOPT __attribute__((noinline, noclone))

/* External function declaration to simulate multi-file scope */
extern int external_helper(int x, int y);

/* Function with multiple attributes that may interact with built-in processing */
static int attribute_helper(int x) 
    __attribute__((visibility("hidden"), nothrow, used, noinline));

/* Volatile variables to prevent optimization */
static volatile int volatile_counter = 0;
static volatile int volatile_result = 0;

/* ========== Test 1: Arithmetic Built-ins ========== */
NOOPT int test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int i;
    
    /* Use volatile to prevent loop unrolling */
    for (i = 0; i < 5; i++) {
        /* Mix different arithmetic built-ins */
        int val = seed + i * 100;
        
        /* __builtin_abs - absolute value */
        int abs_val = __builtin_abs(val);
        
        /* __builtin_sqrtf requires math.h, but we'll use integer built-ins */
        /* __builtin_clz - count leading zeros */
        unsigned int uval = (unsigned int)(val > 0 ? val : -val);
        int clz_val = uval ? __builtin_clz(uval) : sizeof(int) * 8;
        
        /* __builtin_ffs - find first set bit */
        int ffs_val = __builtin_ffs(val);
        
        /* Combine results with memory barrier */
        asm volatile("" : : : "memory");
        
        result += abs_val + clz_val + ffs_val;
    }
    
    volatile_result = result;
    return result;
}

/* ========== Test 2: Bit Operation Built-ins ========== */
NOOPT int test_builtin_bitops(volatile int seed) {
    unsigned int mask = 0xAAAAAAAA;
    unsigned int value = (unsigned int)seed ^ mask;
    int result = 0;
    
    /* __builtin_popcount - count set bits */
    result += __builtin_popcount(value);
    
    /* __builtin_parity - parity of bits */
    result += __builtin_parity(value);
    
    /* __builtin_ctz - count trailing zeros */
    result += value ? __builtin_ctz(value) : 0;
    
    /* __builtin_bswap32 - byte swap */
    result += __builtin_bswap32(value) & 0xFF;
    
    /* Store in volatile to prevent elimination */
    volatile_counter = result;
    
    return result;
}

/* ========== Test 3: Overflow Built-ins ========== */
NOOPT int test_builtin_overflow(volatile int seed) {
    int result = 0;
    int a = seed;
    int b = seed * 2;
    int overflow_result;
    
    /* __builtin_add_overflow */
    if (__builtin_add_overflow(a, b, &overflow_result)) {
        result += 1;
    } else {
        result += overflow_result & 0xFF;
    }
    
    /* __builtin_mul_overflow */
    if (__builtin_mul_overflow(a, 100, &overflow_result)) {
        result += 2;
    } else {
        result += (overflow_result >> 8) & 0xFF;
    }
    
    /* __builtin_sub_overflow */
    if (__builtin_sub_overflow(INT_MAX, a, &overflow_result)) {
        result += 4;
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* ========== Test 4: Control Flow Built-ins ========== */
NOOPT int test_builtin_controlflow(volatile int seed) {
    int result = seed;
    
    /* __builtin_expect - branch prediction */
    if (__builtin_expect((seed & 1) != 0, 1)) {
        result += 100;
    } else {
        result += 200;
    }
    
    /* __builtin_unreachable in a conditional path */
    if (seed < 0) {
        /* This should never happen with our inputs */
        __builtin_unreachable();
    }
    
    /* __builtin_constant_p */
    if (__builtin_constant_p(seed)) {
        result += 1000;
    } else {
        result += 2000;
    }
    
    return result;
}

/* ========== Test 5: Attributed Function ========== */
/* This static function has attributes that may trigger the target flags */
static int attribute_helper(int x) 
    __attribute__((visibility("hidden"), nothrow, used));
    
static int attribute_helper(int x) {
    /* Use __builtin_expect inside attributed function */
    if (__builtin_expect(x > 100, 0)) {
        return __builtin_abs(x) * 2;
    }
    return __builtin_popcount((unsigned int)x);
}

NOOPT int test_attributed_function(volatile int seed) {
    /* Call the attributed helper multiple times */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += attribute_helper(seed + i);
    }
    return sum;
}

/* ========== Test 6: External Linkage Simulation ========== */
/* Forward declaration with extern */
extern int external_builtin_user(int x);

/* Actual definition later in the file */
int external_builtin_user(int x) {
    /* Use __builtin_unreachable for unreachable code */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Mix with other built-ins */
    int result = __builtin_abs(x);
    result += __builtin_clz((unsigned int)result);
    
    /* Force external linkage characteristics */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

NOOPT int test_external_linkage(volatile int seed) {
    /* Call the externally declared function */
    return external_builtin_user(seed);
}

/* ========== Test 7: Complex Expression with Built-ins ========== */
NOOPT int test_complex_expressions(volatile int seed) {
    int a = seed;
    int b = seed * 3;
    int c = seed / 2;
    
    /* Complex expression mixing multiple built-ins */
    int result = __builtin_abs(a) + 
                 (__builtin_popcount((unsigned int)b) * 2) +
                 (__builtin_clz((unsigned int)c) / 4);
    
    /* Use overflow built-in in expression */
    int overflow;
    if (__builtin_add_overflow(result, seed, &overflow)) {
        result = INT_MAX;
    } else {
        result = overflow;
    }
    
    /* Use __builtin_expect in loop */
    for (int i = 0; i < 5; i++) {
        if (__builtin_expect((result & (1 << i)) != 0, 0)) {
            result += i;
        }
    }
    
    return result;
}

/* ========== Test 8: Visibility-Specific Tests ========== */
/* Declare with explicit hidden visibility */
int __attribute__((visibility("hidden"))) 
hidden_builtin_user(int x) {
    /* Use various built-ins */
    int result = 0;
    
    result += __builtin_ffs(x);
    result += __builtin_parity((unsigned int)x);
    
    /* Prevent dead code elimination */
    volatile int dummy = result;
    (void)dummy;
    
    return result;
}

NOOPT int test_visibility_specific(volatile int seed) {
    return hidden_builtin_user(seed);
}

/* ========== Main Function ========== */
int main(int argc, char *argv[]) {
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    int checksum = 0;
    
    printf("Starting built-in function coverage test...\n");
    
    /* Run all test functions */
    checksum += test_builtin_arithmetic(seed);
    checksum += test_builtin_bitops(seed);
    checksum += test_builtin_overflow(seed);
    checksum += test_builtin_controlflow(seed);
    checksum += test_attributed_function(seed);
    checksum += test_external_linkage(seed);
    checksum += test_complex_expressions(seed);
    checksum += test_visibility_specific(seed);
    
    /* Use __builtin_printf if available */
    printf("Final checksum: %d\n", checksum);
    
    /* Return checksum modulo 256 to avoid overflow issues */
    return checksum & 0xFF;
}

/* External helper definition (simulating separate compilation unit) */
int external_helper(int x, int y) {
    /* Use built-ins in external-like function */
    int sum = __builtin_abs(x) + __builtin_abs(y);
    
    /* Use __builtin_add_overflow */
    int overflow;
    if (__builtin_add_overflow(sum, 100, &overflow)) {
        return INT_MAX;
    }
    
    return overflow;
}
