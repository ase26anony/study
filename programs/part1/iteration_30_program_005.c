/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test test.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test different fractional bit counts */
void test_i_f_bits_8(void) {
    printf("Testing i_f_bits = 8 (unsigned short _Fract)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       max value = (2^8 - 1)/2^8 = 255/256 ≈ 0.996
       We need a value where high part > 0 OR (high part = 0 AND low > 255) */
    
    /* Case 1: High part > 0 - use a value > 1.0 */
    unsigned short _Accum a1 = 1.5uhk;  /* 1.5 in unsigned short _Accum */
    unsigned short _Fract f1 = (unsigned short _Fract)a1;  /* Should trigger overflow */
    
    /* Case 2: High part = 0 but low > 255 - use value between 1.0 and 2.0 */
    unsigned short _Accum a2 = 1.1uhk;  /* 1.1 > 1.0 */
    unsigned short _Fract f2 = (unsigned short _Fract)a2;  /* Should trigger overflow */
    
    dummy = f1 + f2;  /* Prevent dead code elimination */
}

void test_i_f_bits_16(void) {
    printf("Testing i_f_bits = 16 (unsigned _Fract)\n");
    
    /* For unsigned _Fract with 16 fractional bits:
       max value = (2^16 - 1)/2^16 ≈ 0.99998
       max_s = 65535 */
    
    /* Create value with high part = 0, low part > 65535 */
    unsigned _Accum a1 = 1.0001ur;  /* Slightly > 1.0 */
    unsigned _Fract f1 = (unsigned _Fract)a1;  /* Should trigger overflow */
    
    /* Value with high part > 0 */
    unsigned _Accum a2 = 2.0ur;  /* Definitely > 1.0 */
    unsigned _Fract f2 = (unsigned _Fract)a2;  /* Should trigger overflow */
    
    dummy = f1 + f2;
}

void test_i_f_bits_24(void) {
    printf("Testing i_f_bits = 24 (unsigned long _Fract)\n");
    
    /* For unsigned long _Fract with 24 fractional bits:
       max value = (2^24 - 1)/2^24 ≈ 0.99999994
       max_s = 16777215 */
    
    /* Use builtin to create overflow */
    unsigned long _Accum a1 = 1.0000001ulr;  /* Very slightly > 1.0 */
    unsigned long _Fract f1 = (unsigned long _Fract)a1;
    
    /* Arithmetic that overflows */
    unsigned long _Fract f2 = 0.9ulr;
    unsigned long _Fract f3 = 0.2ulr;
    unsigned long _Fract sum = f2 + f3;  /* 1.1 > 1.0 */
    
    dummy = f1 + sum;
}

void test_i_f_bits_32(void) {
    printf("Testing i_f_bits = 32 (unsigned long long _Fract)\n");
    
    /* For unsigned long long _Fract with 32 fractional bits */
    unsigned long long _Accum a1 = 1.0000000001ullr;
    unsigned long long _Fract f1 = (unsigned long long _Fract)a1;
    
    dummy = f1;
}

/* Test signed types as well */
void test_signed_i_f_bits_8(void) {
    printf("Testing signed i_f_bits = 8 (short _Fract)\n");
    
    /* For signed short _Fract with 8 fractional bits:
       max positive = (2^7 - 1)/2^7 = 127/128 ≈ 0.992
       We need positive overflow */
    
    short _Accum a1 = 1.0hk;  /* 1.0 > 0.992 */
    short _Fract f1 = (short _Fract)a1;  /* Should trigger positive overflow */
    
    /* Also test negative overflow */
    short _Accum a2 = -1.0hk;  /* -1.0 < -1.0? Actually min is -1.0, so this should be OK */
    
    dummy = f1 + a2;
}

/* Test with saturation attribute */
void test_saturation(void) {
    printf("Testing with saturation\n");
    
    /* With saturation, overflow should clamp to max value */
    unsigned short _Fract __attribute__((saturated)) fsat;
    unsigned short _Accum a = 2.0uhk;
    
    fsat = (unsigned short _Fract)a;  /* Should saturate to max */
    
    /* Test arithmetic with saturation */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.8uhr;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.3uhr;
    unsigned short _Fract __attribute__((saturated)) sum = f1 + f2;  /* 1.1 > 1.0, should saturate */
    
    dummy = fsat + sum;
}

/* Test using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Testing builtin overflow\n");
    
    unsigned short _Fract f1 = 0.9uhr;
    unsigned short _Fract f2 = 0.2uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* This might use the same overflow checking logic */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    printf("Add overflow: %d\n", overflow);
    
    /* Test multiplication overflow */
    unsigned short _Fract f3 = 1.5uhr;  /* Actually can't represent 1.5 in _Fract */
    unsigned short _Fract f4 = 0.8uhr;
    overflow = __builtin_mul_overflow(f3, f4, &result);
    
    printf("Mul overflow: %d\n", overflow);
}

/* Test conversions from integer to fixed-point */
void test_int_to_fixed(void) {
    printf("Testing integer to fixed-point conversion\n");
    
    /* Convert integer > 1 to _Fract */
    unsigned int i1 = 2;
    unsigned short _Fract f1 = (unsigned short _Fract)i1;  /* 2 > 255/256, should overflow */
    
    /* Convert large integer */
    unsigned long long i2 = 1000;
    unsigned _Fract f2 = (unsigned _Fract)i2;  /* Definitely overflows */
    
    dummy = f1 + f2;
}

/* Test with different optimization levels */
void test_mixed_operations(void) {
    printf("Testing mixed operations\n");
    
    /* Complex expression that might overflow */
    unsigned short _Fract f1 = 0.5uhr;
    unsigned short _Fract f2 = 0.6uhr;
    
    /* This addition results in 1.1 which exceeds max for _Fract */
    unsigned short _Fract sum = f1 + f2;
    
    /* Chain conversions */
    unsigned _Accum a1 = 1.5ur;
    unsigned short _Fract f3 = (unsigned short _Fract)(unsigned _Fract)a1;
    
    /* Use in conditional to prevent optimization */
    if (sum > 0.9uhr) {
        dummy = f3;
    }
}

int main(void) {
    int result = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    /* Run all tests */
    test_i_f_bits_8();
    test_i_f_bits_16();
    test_i_f_bits_24();
    test_i_f_bits_32();
    test_signed_i_f_bits_8();
    test_saturation();
    test_builtin_overflow();
    test_int_to_fixed();
    test_mixed_operations();
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any issues detected */
    return result;
}
