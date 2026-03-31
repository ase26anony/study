/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int vol_accum = 0;
static volatile int vol_seed = 0;

/* ====== Pattern 1: Basic built-in arithmetic with volatile context ====== */
static int __attribute__((noinline, noclone)) 
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int local_vol = seed;
    
    /* Use multiple arithmetic built-ins in a loop */
    for (int i = 0; i < 5; i++) {
        /* __builtin_abs with volatile input */
        int abs_val = __builtin_abs(local_vol + i - 10);
        
        /* __builtin_sqrtf with type conversion */
        float sqrt_val = __builtin_sqrtf((float)(abs_val + 1));
        
        /* __builtin_expect to influence branch prediction */
        if (__builtin_expect(abs_val > 5, 1)) {
            result += (int)sqrt_val;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        local_vol += i;
    }
    
    /* Store to volatile to ensure side effect */
    vol_accum += result;
    return result;
}

/* ====== Pattern 2: Bit manipulation built-ins ====== */
static int __attribute__((noinline, noclone, used))
test_builtin_bitops(unsigned int seed) {
    unsigned int val = seed;
    int total_bits = 0;
    
    /* Chain multiple bit operation built-ins */
    for (int i = 0; i < 4; i++) {
        /* __builtin_popcount */
        total_bits += __builtin_popcount(val);
        
        /* __builtin_clz (count leading zeros) */
        if (val != 0) {
            total_bits += __builtin_clz(val);
        }
        
        /* __builtin_ctz (count trailing zeros) */
        if (val != 0) {
            total_bits += __builtin_ctz(val);
        }
        
        /* __builtin_ffs (find first set) */
        total_bits += __builtin_ffs(val);
        
        /* Modify value for next iteration */
        val = (val * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Optimization barrier */
        asm volatile("" : "+r"(val) : : "memory");
    }
    
    return total_bits;
}

/* ====== Pattern 3: Overflow checking built-ins ====== */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    int result = 0;
    int overflow_flag;
    
    /* __builtin_add_overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        result = a;  // Use original value on overflow
    }
    
    /* __builtin_sub_overflow */
    int sub_result;
    if (__builtin_sub_overflow(a, b, &sub_result)) {
        result -= b;
    } else {
        result += sub_result;
    }
    
    /* __builtin_mul_overflow */
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        result = result / 2;  // Handle overflow case
    } else {
        result += mul_result;
    }
    
    /* __builtin_sadd_overflow (signed add) */
    long long ll_result;
    if (__builtin_saddll_overflow(a, b, &ll_result)) {
        result += 1000;
    }
    
    return result;
}

/* ====== Pattern 4: Function with explicit attributes ====== */
/* This static function has attributes that may trigger the target flags */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
attributed_builtin_user(int x) {
    /* Use __builtin_expect inside attributed function */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x) + __builtin_popcount((unsigned int)x);
    }
    return 0;
}

/* ====== Pattern 5: External linkage simulation ====== */
/* Forward declaration with built-in prototype */
int __builtin_custom_test(int) __attribute__((visibility("hidden")));

/* Function that uses the external built-in declaration */
static int __attribute__((noinline))
test_external_linkage(int val) {
    int result = val;
    
    /* Call to externally declared built-in-like function */
    result = external_builtin_user(result);
    
    /* Use __builtin_unreachable under specific condition */
    if (result < 0) {
        __builtin_unreachable();  /* Should not happen */
    }
    
    /* __builtin_assume to provide optimization hint */
    __builtin_assume(result >= 0);
    
    return result;
}

/* ====== Pattern 6: Complex expression with multiple built-ins ====== */
static int __attribute__((noinline, noclone))
test_complex_expression(volatile int base) {
    int a = base;
    int b = a + 1;
    int c = a * 2;
    
    /* Nested built-in calls in complex expression */
    int complex_result = 
        __builtin_abs(a) * 
        (__builtin_popcount((unsigned int)b) + 1) / 
        (__builtin_clz((unsigned int)c) + 1);
    
    /* Use __builtin_constant_p to check if value is constant */
    if (!__builtin_constant_p(a)) {
        complex_result += __builtin_ffs(b);
    }
    
    /* __builtin_prefetch */
    __builtin_prefetch(&vol_accum, 0, 3);
    
    return complex_result;
}

/* ====== External function definition (simulating another TU) ====== */
int external_builtin_user(int x) {
    /* Use different built-ins in external function */
    int result = __builtin_bswap32(x);  /* Byte swap */
    
    /* __builtin_parity */
    result ^= __builtin_parity((unsigned int)x) << 16;
    
    /* __builtin_rotateleft32 (if available) */
    #ifdef __builtin_rotateleft32
    result = __builtin_rotateleft32(result, 3);
    #endif
    
    return result;
}

/* ====== Hidden visibility function definition ====== */
void __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(void) {
    /* Function body that uses built-ins */
    volatile int local = 42;
    int temp = __builtin_abs(local - 50);
    
    /* Store to global volatile to ensure side effect */
    vol_accum += temp;
    
    /* __builtin_trap under impossible condition */
    if (local < 0) {
        __builtin_trap();
    }
}

/* ====== Main function orchestrating all tests ====== */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed from runtime sources */
    vol_seed = argc;
    vol_seed ^= (int)time(NULL) & 0xFF;
    vol_seed |= 1;  /* Ensure non-zero */
    
    int final_checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    final_checksum += test_builtin_arithmetic(vol_seed);
    
    /* Test 2: Bit operation built-ins */
    final_checksum += test_builtin_bitops((unsigned int)vol_seed);
    
    /* Test 3: Overflow built-ins */
    final_checksum += test_builtin_overflow(vol_seed, vol_seed + 1);
    
    /* Test 4: Attributed function with built-ins */
    final_checksum += attributed_builtin_user(vol_seed);
    
    /* Test 5: External linkage pattern */
    final_checksum += test_external_linkage(vol_seed);
    
    /* Test 6: Complex expression */
    final_checksum += test_complex_expression(vol_seed);
    
    /* Call hidden visibility function */
    hidden_visibility_func();
    
    /* Additional direct built-in usage in main */
    int direct_result = 0;
    for (int i = 0; i < 3; i++) {
        /* __builtin_ctzll (64-bit version) */
        direct_result += __builtin_ctzll((unsigned long long)(vol_seed + i));
        
        /* __builtin_fabs */
        direct_result += (int)__builtin_fabs((double)(vol_seed - i));
    }
    final_checksum += direct_result;
    
    /* Use __builtin_constant_p with conditional */
    if (__builtin_constant_p(argc)) {
        final_checksum += 1000;
    } else {
        final_checksum += 2000;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: %d (vol_accum: %d)\n", final_checksum, vol_accum);
    
    return final_checksum & 0xFF;  /* Return non-zero result */
}
