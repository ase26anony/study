/* test_builtin_hooks.c - Comprehensive built-in function test for GCC targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* Prevent optimization of helper functions */
#define NOOPT __attribute__((noinline, noclone))

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));

/* Function with multiple attributes that may interact with built-in processing */
static int hidden_used_function(int x) 
    __attribute__((visibility("hidden"), used, nothrow, artificial));

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_result = 0;

/* ==================== TEST 1: Basic built-in arithmetic ==================== */
NOOPT int test_builtin_arithmetic(volatile int seed) {
    volatile int acc = 0;
    
    /* Use volatile counter to prevent loop unrolling */
    volatile int i;
    for (i = 0; i < 3; i++) {
        /* Mix different built-in types */
        int val = seed + i * 100;
        
        /* __builtin_abs - common arithmetic built-in */
        int abs_val = __builtin_abs(val);
        
        /* __builtin_sqrtf - math built-in (float version) */
        float sqrt_val = __builtin_sqrtf((float)(abs_val + 1));
        
        /* __builtin_expect - branch prediction built-in */
        if (__builtin_expect(abs_val > 100, 1)) {
            /* __builtin_clz - count leading zeros */
            int leading_zeros = __builtin_clz((unsigned int)abs_val);
            acc += leading_zeros;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        acc += (int)sqrt_val;
    }
    
    return acc;
}

/* ==================== TEST 2: Bit manipulation built-ins ==================== */
NOOPT int test_builtin_bitops(volatile unsigned int seed) {
    unsigned int x = seed ^ 0xDEADBEEF;
    int result = 0;
    
    /* __builtin_popcount - population count */
    result += __builtin_popcount(x);
    
    /* __builtin_ctz - count trailing zeros */
    result += __builtin_ctz(x | 1);  /* OR with 1 to avoid undefined behavior */
    
    /* __builtin_ffs - find first set bit */
    result += __builtin_ffs((int)x);
    
    /* __builtin_parity - parity of number of 1-bits */
    result += __builtin_parity(x);
    
    /* __builtin_bswap32 - byte swap */
    unsigned int swapped = __builtin_bswap32(x);
    result += __builtin_popcount(swapped);
    
    return result;
}

/* ==================== TEST 3: Overflow checking built-ins ==================== */
NOOPT int test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    long long overflow_result;
    bool overflow;
    
    /* __builtin_add_overflow */
    overflow = __builtin_add_overflow(a, b, &overflow_result);
    result += overflow ? 1 : 0;
    result += (int)overflow_result;
    
    /* __builtin_mul_overflow */
    overflow = __builtin_mul_overflow(a, b, &overflow_result);
    result += overflow ? 2 : 0;
    result += (int)overflow_result;
    
    /* __builtin_sub_overflow */
    overflow = __builtin_sub_overflow(a, b, &overflow_result);
    result += overflow ? 4 : 0;
    result += (int)overflow_result;
    
    /* __builtin_sadd_overflow - signed add overflow */
    int int_result;
    overflow = __builtin_sadd_overflow(a, b, &int_result);
    result += overflow ? 8 : 0;
    result += int_result;
    
    return result;
}

/* ==================== TEST 4: Attributed function with built-ins ==================== */
/* This static function has attributes that should trigger flag setting */
static int hidden_used_function(int x) {
    /* __builtin_expect used inside attributed function */
    if (__builtin_expect(x > 0, 1)) {
        /* __builtin_constant_p - test if value is constant */
        if (!__builtin_constant_p(x)) {
            /* __builtin_powi - power function */
            double power = __builtin_powif((float)x, 2);
            return (int)power + __builtin_abs(x);
        }
    }
    
    /* __builtin_unreachable in a dead branch */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    return x;
}

/* ==================== TEST 5: External linkage simulation ==================== */
/* Function defined with extern declaration earlier */
int external_builtin_user(int x) {
    int result = 0;
    
    /* Use various built-ins in external-like function */
    result += __builtin_abs(x);
    
    /* __builtin_frame_address - get frame address */
    void* frame_addr = __builtin_frame_address(0);
    result += (int)((long)frame_addr & 0xFF);
    
    /* __builtin_return_address */
    void* return_addr = __builtin_return_address(0);
    result += (int)((long)return_addr & 0xFF);
    
    /* Conditional __builtin_unreachable */
    if (x == INT_MIN) {
        __builtin_unreachable();
    }
    
    return result;
}

/* ==================== TEST 6: Complex expression with built-ins ==================== */
NOOPT int test_complex_expressions(volatile int seed) {
    volatile int a = seed;
    volatile int b = seed * 2;
    volatile int c = seed / 3;
    
    /* Complex expression mixing multiple built-ins */
    int result = __builtin_abs(a) + 
                 __builtin_popcount((unsigned int)b) * 2 -
                 __builtin_clz((unsigned int)(c | 1)) +
                 (__builtin_expect(a > b, 0) ? 10 : 20);
    
    /* Use built-in in inline assembly context */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r" (result)
        : "r" (__builtin_abs(b))
        : "cc"
    );
    
    /* Memory clobber to prevent optimization */
    asm volatile("" : : : "memory");
    
    return result;
}

/* ==================== TEST 7: Built-in declarations with attributes ==================== */
/* Explicit built-in function declarations with attributes */
int __builtin_popcount(unsigned int) 
    __attribute__((visibility("hidden"), nothrow, used));

int __builtin_abs(int) 
    __attribute__((visibility("hidden"), nothrow));

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char* argv[]) {
    volatile int seed = argc > 1 ? argv[1][0] : 12345;
    int checksum = 0;
    
    printf("Testing GCC built-in hooks with seed: %d\n", seed);
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed + 100);
    
    /* Test 4: Attributed function */
    checksum += hidden_used_function(seed);
    
    /* Test 5: External linkage simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Complex expressions */
    checksum += test_complex_expressions(seed);
    
    /* Direct built-in calls with volatile results */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed);
    direct_result += __builtin_popcount((unsigned int)seed);
    direct_result += __builtin_clz((unsigned int)(seed | 1));
    
    /* Use __builtin_expect in main */
    if (__builtin_expect(checksum > 0, 1)) {
        direct_result += checksum;
    }
    
    /* Final result computation to ensure all code is live */
    int final_result = checksum + direct_result;
    
    /* Prevent dead code elimination */
    global_result = final_result;
    
    printf("Final checksum: %d\n", final_result);
    
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
