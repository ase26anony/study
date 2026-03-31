/* Test program to trigger uncovered lines in fixed-value.cc
 * Lines 264-277: overflow checking for fixed-point conversions
 */

/* Prevent optimizations from removing overflow checks */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test different fractional bit counts through different fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_8bit_fractional(void) {
    /* max_s = 2^8 - 1 = 255 */
    /* Case 1: high part = 0, low part > 255 */
    unsigned short _Fract f1;
    unsigned short _Accum a1 = 256.0uhk;  /* 256 = 0x100, low part = 0x100 > 255 */
    
    f1 = (unsigned short _Fract)a1;  /* Should trigger overflow check */
    KEEP(f1);
    
    /* Case 2: high part > 0 */
    unsigned int _Accum a2 = 65536.0uhk;  /* High part > 0 */
    unsigned short _Fract f2;
    
    f2 = (unsigned short _Fract)a2;  /* Should trigger high part > 0 check */
    KEEP(f2);
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_16bit_fractional(void) {
    /* max_s = 2^16 - 1 = 65535 */
    unsigned _Fract f1;
    unsigned _Accum a1 = 65536.0uk;  /* 0x10000 > 65535, high part = 0 */
    
    f1 = (unsigned _Fract)a1;  /* Should trigger overflow */
    KEEP(f1);
    
    /* Test with signed types */
    _Fract f2;
    long _Accum a2 = 32768.0lk;  /* Positive value */
    
    f2 = (_Fract)a2;  /* Should trigger check */
    KEEP(f2);
}

/* Test 3: i_f_bits = 24 (unsigned long _Fract) */
void test_24bit_fractional(void) {
    /* Note: actual i_f_bits may vary by platform */
    unsigned long _Fract f1;
    unsigned long _Accum a1 = 16777216.0ulk;  /* 2^24 */
    
    f1 = (unsigned long _Fract)a1;  /* Should overflow */
    KEEP(f1);
}

/* Test 4: i_f_bits = 32 (unsigned long long _Fract) */
void test_32bit_fractional(void) {
    unsigned long long _Fract f1;
    unsigned long long _Accum a1 = 4294967296.0ullk;  /* 2^32 */
    
    f1 = (unsigned long long _Fract)a1;  /* Should overflow */
    KEEP(f1);
}

/* Test 5: Small i_f_bits = 1 (edge case) */
void test_1bit_fractional(void) {
    /* For 1 fractional bit, max_s = 2^1 - 1 = 1 */
    /* Use saturating types to ensure overflow logic is used */
    _Sat unsigned short _Fract f1;
    unsigned short _Accum a1 = 2.0uhk;  /* 2 > 1 */
    
    f1 = (_Sat unsigned short _Fract)a1;  /* Should saturate to max */
    KEEP(f1);
}

/* Test 6: Arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    unsigned short _Fract f1 = 0.8uhr;
    unsigned short _Fract f2 = 0.8uhr;
    unsigned short _Fract sum;
    
    /* Addition that exceeds 1.0 */
    sum = f1 + f2;  /* 0.8 + 0.8 = 1.6 > 1.0 */
    KEEP(sum);
    
    /* Multiplication overflow */
    unsigned short _Accum a1 = 300.0uhk;
    unsigned short _Accum a2 = 2.0uhk;
    unsigned short _Fract product;
    
    product = (unsigned short _Fract)(a1 * a2);  /* 600 > 255 */
    KEEP(product);
}

/* Test 7: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    unsigned short _Fract f1 = 0.9uhr;
    unsigned short _Fract f2 = 0.9uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* Use overflow checking builtin */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
    
    /* Test multiplication overflow */
    unsigned short _Accum a1 = 200.0uhk;
    unsigned short _Accum a2 = 2.0uhk;
    unsigned short _Fract mul_result;
    
    overflow = __builtin_mul_overflow((unsigned short _Fract)a1, 
                                      (unsigned short _Fract)a2, 
                                      &mul_result);
    KEEP(overflow);
    KEEP(mul_result);
}

/* Test 8: Mixed signed/unsigned conversions */
void test_mixed_sign_conversions(void) {
    /* Signed to unsigned with positive overflow */
    long _Accum sa = 500.0lk;
    unsigned short _Fract uf;
    
    uf = (unsigned short _Fract)sa;  /* 500 > 255 */
    KEEP(uf);
    
    /* Unsigned to signed with overflow */
    unsigned long _Accum ua = 300.0ulk;
    short _Fract sf;
    
    sf = (short _Fract)ua;  /* 300 > 127 (max positive for signed 8-bit fract) */
    KEEP(sf);
}

/* Test 9: Very large values to trigger high part > 0 */
void test_large_values(void) {
    /* Use long long accum to get large high parts */
    long long _Accum lla = 1000000000000.0llk;
    short _Fract sf;
    
    sf = (short _Fract)lla;  /* Definitely high part > 0 */
    KEEP(sf);
    
    unsigned long long _Accum ulla = 1000000000000.0ullk;
    unsigned short _Fract usf;
    
    usf = (unsigned short _Fract)ulla;  /* High part > 0 */
    KEEP(usf);
}

/* Test 10: Boundary cases exactly at max_s */
void test_boundary_cases(void) {
    /* Test value exactly equal to max_s (should not trigger overflow) */
    unsigned short _Fract f1;
    unsigned short _Accum a1 = 255.0uhk;  /* Exactly max_s for 8-bit */
    
    f1 = (unsigned short _Fract)a1;  /* Should NOT overflow */
    KEEP(f1);
    
    /* Test value just above max_s */
    unsigned short _Accum a2 = 255.1uhk;  /* Just above 255 */
    
    f1 = (unsigned short _Fract)a2;  /* Should overflow */
    KEEP(f1);
}

/* Main driver that calls all tests */
int main(void) {
    int test_count = 0;
    
    test_8bit_fractional();
    test_count++;
    
    test_16bit_fractional();
    test_count++;
    
    test_24bit_fractional();
    test_count++;
    
    test_32bit_fractional();
    test_count++;
    
    test_1bit_fractional();
    test_count++;
    
    test_arithmetic_overflow();
    test_count++;
    
    test_builtin_overflow();
    test_count++;
    
    test_mixed_sign_conversions();
    test_count++;
    
    test_large_values();
    test_count++;
    
    test_boundary_cases();
    test_count++;
    
    /* Return number of tests run for verification */
    return test_count == 10 ? 0 : 1;
}
