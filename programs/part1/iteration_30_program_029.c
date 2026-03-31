/* test_fixed_overflow.c - Test overflow conditions in fixed-point conversions */

/* Prevent optimizations from removing our test cases */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    __asm__ volatile ("" : : "r" (_tmp)); \
} while (0)

/* Test function for unsigned types with different fractional bits */
void test_unsigned_overflow() {
    /* Test 1: i_f_bits = 8 (unsigned short _Fract) */
    /* max_s = 2^8 - 1 = 255 */
    /* Condition: a_high == 0 && a_low > 255 */
    
    /* Create a value with low part > 255 and high part = 0 */
    unsigned short _Accum usa = 256.0uhk;  /* 256 in 8.8 format */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    KEEP(usf);
    
    /* Test 2: i_f_bits = 16 (unsigned _Fract) */
    /* max_s = 2^16 - 1 = 65535 */
    unsigned _Accum ua = 65536.0uk;  /* 65536 in 16.16 format */
    unsigned _Fract uf;
    
    uf = (unsigned _Fract)ua;
    KEEP(uf);
    
    /* Test 3: i_f_bits = 24 (unsigned long _Fract) */
    /* max_s = 2^24 - 1 = 16777215 */
    unsigned long _Accum ula = 16777216.0ulk;  /* 16777216 in 8.24 format? */
    unsigned long _Fract ulf;
    
    ulf = (unsigned long _Fract)ula;
    KEEP(ulf);
}

/* Test function for signed types with different fractional bits */
void test_signed_overflow() {
    /* For signed types, we need to trigger a_high > 0 condition */
    
    /* Test 1: i_f_bits = 8 (signed short _Fract) */
    /* Condition: a_high > 0 (positive high part) */
    
    /* Create a value with high part > 0 */
    signed short _Accum ssa = 128.0hk;  /* 128 in 7.8 format (max positive) */
    signed short _Fract ssf;
    
    /* Try to convert to a type with fewer integer bits */
    ssf = (signed short _Fract)ssa;
    KEEP(ssf);
    
    /* Test overflow through addition */
    signed short _Fract f1 = 0.5hr;
    signed short _Fract f2 = 0.6hr;
    signed short _Fract sum = f1 + f2;  /* Should be > 1.0, may overflow */
    KEEP(sum);
    
    /* Test 2: i_f_bits = 16 (signed _Fract) */
    signed _Accum sa = 32768.0k;  /* 32768 in 15.16 format */
    signed _Fract sf;
    
    sf = (signed _Fract)sa;
    KEEP(sf);
}

/* Test with saturation attribute */
void test_saturation() {
    /* With saturation, overflow should be handled explicitly */
    unsigned short _Fract __attribute__((saturated)) usf_sat;
    unsigned short _Accum usa = 256.0uhk;
    
    usf_sat = (unsigned short _Fract __attribute__((saturated)))usa;
    KEEP(usf_sat);
    
    /* Test saturated arithmetic */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) product = f1 * f2;
    KEEP(product);
}

/* Test with builtin overflow detection */
void test_builtin_overflow() {
    unsigned short _Fract f1 = 0.5uhr;
    unsigned short _Fract f2 = 0.6uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* Use generic overflow builtin */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
    
    /* Test multiplication overflow */
    overflow = __builtin_mul_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
}

/* Test conversions from integer to fixed-point */
void test_int_to_fixed() {
    /* Convert integer that's too large for the fixed-point type */
    unsigned int large_int = 256;
    unsigned short _Fract usf;
    
    usf = (unsigned short _Fract)large_int;
    KEEP(usf);
    
    /* Test with negative integer for signed types */
    int negative_int = -129;  /* Outside range of signed short _Fract */
    signed short _Fract ssf;
    
    ssf = (signed short _Fract)negative_int;
    KEEP(ssf);
}

/* Test edge cases with minimal i_f_bits */
void test_minimal_bits() {
    /* For very small i_f_bits values */
    
    /* Simulate i_f_bits = 1 */
    /* max_s = 2^1 - 1 = 1 */
    /* Any value with low part > 1 should trigger overflow */
    
    /* Use a custom type or rely on compiler to generate appropriate i_f_bits */
    unsigned _Fract uf;
    unsigned _Accum ua = 2.0uk;  /* Should overflow when converted */
    
    uf = (unsigned _Fract)ua;
    KEEP(uf);
    
    /* Test with exactly the boundary value */
    unsigned _Accum boundary = 1.0uk;
    uf = (unsigned _Fract)boundary;  /* Should not overflow */
    KEEP(uf);
}

/* Test mixed-type arithmetic */
void test_mixed_arithmetic() {
    /* Operations between different fixed-point types */
    unsigned short _Accum usa = 128.0uhk;
    unsigned short _Fract usf = 0.5uhr;
    
    /* Mixed arithmetic may trigger conversions with overflow checks */
    unsigned short _Accum result1 = usa * usf;
    KEEP(result1);
    
    unsigned short _Fract result2 = (unsigned short _Fract)(usa * usf);
    KEEP(result2);
}

/* Main driver function */
int main() {
    int test_result = 0;
    
    /* Execute all test functions */
    test_unsigned_overflow();
    test_signed_overflow();
    test_saturation();
    test_builtin_overflow();
    test_int_to_fixed();
    test_minimal_bits();
    test_mixed_arithmetic();
    
    /* The actual coverage happens at compile-time during constant folding
       and fixed-point conversion in GCC's middle-end. The runtime execution
       just ensures the code paths are taken. */
    
    return test_result;
}
