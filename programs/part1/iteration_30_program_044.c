/* test_fixed_overflow.c - Test program to trigger overflow checks in fixed-value.cc */

/* Prevent dead code elimination */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    __asm__ volatile("" : : "r"(&_tmp)); \
} while(0)

/* Test different fixed-point types to cover various i_f_bits values */

/* Test 1: Unsigned fract with 8 fractional bits (i_f_bits=8) */
void test_unsigned_fract_overflow() {
    /* For unsigned short _Fract: 8 fractional bits, 0 integer bits */
    /* Max value = (2^8 - 1)/2^8 = 255/256 = 0.99609375 */
    
    /* This should trigger overflow (value > max_s) */
    volatile unsigned short _Accum source = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract target;
    
    /* Explicit cast that should trigger overflow check */
    target = (unsigned short _Fract)source;
    KEEP(target);
    
    /* Another test: arithmetic that exceeds bounds */
    unsigned short _Fract f1 = 0.5uhr;
    unsigned short _Fract f2 = 0.6uhr;
    unsigned short _Fract sum = f1 + f2;  /* 1.1 > 255/256 */
    KEEP(sum);
}

/* Test 2: Signed fract with 7 fractional bits (i_f_bits=7 for signed) */
void test_signed_fract_overflow() {
    /* For signed short _Fract: 7 fractional bits, 1 sign bit */
    /* Max value = (2^7 - 1)/2^7 = 127/128 = 0.9921875 */
    
    /* Positive overflow */
    volatile signed short _Accum source_pos = 1.0hk;  /* 1.0 in signed short _Accum */
    signed short _Fract target_pos = (signed short _Fract)source_pos;
    KEEP(target_pos);
    
    /* Negative overflow */
    volatile signed short _Accum source_neg = -1.0hk;
    signed short _Fract target_neg = (signed short _Fract)source_neg;
    KEEP(target_neg);
}

/* Test 3: Unsigned accum with 8 fractional bits (i_f_bits=8) */
void test_unsigned_accum_overflow() {
    /* For unsigned short _Accum: 8 fractional bits, 8 integer bits */
    /* Max value = (2^16 - 1)/2^8 = 65535/256 = 255.99609375 */
    
    /* This should trigger overflow (a_high > max_r) */
    volatile unsigned int source = 256u;  /* 256 > 255.996... */
    unsigned short _Accum target = (unsigned short _Accum)source;
    KEEP(target);
    
    /* Test with builtin overflow check */
    unsigned short _Accum a1 = 200.0uhk;
    unsigned short _Accum a2 = 100.0uhk;
    unsigned short _Accum product;
    int overflow = __builtin_mul_overflow(a1, a2, &product);
    KEEP(overflow);
    KEEP(product);
}

/* Test 4: Signed accum with 8 fractional bits (i_f_bits=8) */
void test_signed_accum_overflow() {
    /* For signed short _Accum: 8 fractional bits, 7 integer bits + sign */
    /* Max value = (2^15 - 1)/2^8 = 32767/256 = 127.99609375 */
    
    /* Positive overflow */
    volatile int source_pos = 128;
    signed short _Accum target_pos = (signed short _Accum)source_pos;
    KEEP(target_pos);
    
    /* Negative overflow */
    volatile int source_neg = -129;
    signed short _Accum target_neg = (signed short _Accum)source_neg;
    KEEP(target_neg);
}

/* Test 5: Mixed-type conversions with different i_f_bits */
void test_mixed_conversions() {
    /* Test various conversions that should trigger overflow checks */
    
    /* From larger to smaller fractional bits */
    unsigned long _Fract f1 = 0.999999ulr;  /* ~1.0 with many fractional bits */
    unsigned short _Fract f2 = (unsigned short _Fract)f1;
    KEEP(f2);
    
    /* Integer to fixed-point with overflow */
    long int big_int = 1000000L;
    short _Accum fixed = (short _Accum)big_int;
    KEEP(fixed);
    
    /* Using saturated arithmetic attribute */
    unsigned short _Fract __attribute__((saturated)) sat_f1 = 0.8uhr;
    unsigned short _Fract __attribute__((saturated)) sat_f2 = 0.9uhr;
    unsigned short _Fract sat_sum = sat_f1 + sat_f2;
    KEEP(sat_sum);
}

/* Test 6: Edge cases with specific values to trigger a_high == max_r && a_low > max_s */
void test_edge_case_overflow() {
    /* We want a value where high part is 0 but low part > max_s */
    /* For unsigned short _Fract: max_s = 2^8 - 1 = 255 */
    /* We need a value with high=0, low=256 */
    
    /* Create value that's exactly at the boundary + epsilon */
    volatile unsigned short _Accum boundary = 255.0uhk / 256.0uhk;  /* 255/256 */
    volatile unsigned short _Accum epsilon = 1.0uhk / 256.0uhk;     /* 1/256 */
    unsigned short _Accum exceed = boundary + epsilon;  /* 256/256 = 1.0 */
    
    /* This conversion should trigger the a_low > max_s check */
    unsigned short _Fract converted = (unsigned short _Fract)exceed;
    KEEP(converted);
}

/* Test 7: Using compile-time constants that force constant folding */
void test_compile_time_overflow() {
    /* These should be evaluated at compile time, triggering the code path */
    const unsigned short _Fract cf1 = (unsigned short _Fract)1.0uhk;
    const signed short _Fract cf2 = (signed short _Fract)-1.0hk;
    const unsigned short _Accum cf3 = (unsigned short _Accum)256u;
    
    KEEP(cf1);
    KEEP(cf2);
    KEEP(cf3);
}

/* Main driver that runs all tests */
int main() {
    int result = 0;
    
    /* Run all test functions */
    test_unsigned_fract_overflow();
    test_signed_fract_overflow();
    test_unsigned_accum_overflow();
    test_signed_accum_overflow();
    test_mixed_conversions();
    test_edge_case_overflow();
    test_compile_time_overflow();
    
    /* Return 0 if all tests compiled and ran */
    return result;
}
