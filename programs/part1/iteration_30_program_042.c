/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int use_result = 0;

/* Test different i_f_bits scenarios */
void test_i_f_bits_8(void) {
    printf("Testing i_f_bits=8 scenarios...\n");
    
    /* For i_f_bits=8, max_s = (2^8 - 1) = 255 */
    /* Test case 1: a_high == max_r (0) AND a_low > max_s (255) */
    /* This should trigger the second part of the condition */
    unsigned short _Accum usa = 256.0uhk;  /* 256 = 256 << 8 = 65536 in internal representation */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    use_result += (unsigned int)usf;
    
    /* Test case 2: a_high > max_r (positive high part) */
    /* Need a value with high part > 0 */
    unsigned long _Accum ula = 65536.0ulk;  /* Large value that will have high part > 0 */
    unsigned short _Fract usf2;
    
    usf2 = (unsigned short _Fract)ula;
    use_result += (unsigned int)usf2;
}

void test_i_f_bits_16(void) {
    printf("Testing i_f_bits=16 scenarios...\n");
    
    /* For i_f_bits=16, max_s = (2^16 - 1) = 65535 */
    /* Test low part overflow with zero high part */
    unsigned _Accum ua = 65536.0uhk;  /* 65536 = 65536 << 16 = 2^32 in internal rep */
    unsigned _Fract uf;
    
    uf = (unsigned _Fract)ua;
    use_result += (unsigned int)uf;
    
    /* Test positive high part */
    unsigned long _Accum ula = 4294967296.0ulk;  /* 2^32, will have high part > 0 */
    unsigned _Fract uf2;
    
    uf2 = (unsigned _Fract)ula;
    use_result += (unsigned int)uf2;
}

void test_i_f_bits_24(void) {
    printf("Testing i_f_bits=24 scenarios...\n");
    
    /* For i_f_bits=24, max_s = (2^24 - 1) = 16777215 */
    /* Test with long accum to fract conversion */
    unsigned long _Accum ula = 16777216.0ulk;  /* 2^24 */
    unsigned long _Fract ulf;
    
    ulf = (unsigned long _Fract)ula;
    use_result += (unsigned int)ulf;
}

void test_signed_types(void) {
    printf("Testing signed types...\n");
    
    /* Test signed overflow scenarios */
    short _Accum sa = 128.0hk;  /* Max for 7 int bits + 8 frac bits? */
    short _Fract sf;
    
    sf = (short _Fract)sa;
    use_result += (unsigned int)sf;
    
    /* Test negative to positive overflow */
    signed long _Accum sla = -129.0lk;
    short _Fract sf2;
    
    sf2 = (short _Fract)sla;
    use_result += (unsigned int)sf2;
}

void test_arithmetic_overflow(void) {
    printf("Testing arithmetic overflow...\n");
    
    /* Arithmetic that overflows */
    unsigned short _Fract f1 = 0.8uhr;
    unsigned short _Fract f2 = 0.8uhr;
    unsigned short _Fract sum;
    
    sum = f1 + f2;  /* 0.8 + 0.8 = 1.6 > 1.0, should overflow */
    use_result += (unsigned int)sum;
    
    /* Multiplication overflow */
    unsigned _Fract m1 = 0.9ur;
    unsigned _Fract m2 = 0.9ur;
    unsigned _Fract prod;
    
    prod = m1 * m2;  /* 0.9 * 0.9 = 0.81 < 1.0, but test path anyway */
    use_result += (unsigned int)prod;
}

void test_builtin_overflow(void) {
    printf("Testing builtin overflow...\n");
    
    /* Use overflow builtins with fixed-point */
    unsigned short _Fract a = 0.9uhr;
    unsigned short _Fract b = 0.9uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* These might trigger the overflow checking path */
    overflow = __builtin_add_overflow(a, b, &result);
    use_result += overflow;
    
    overflow = __builtin_mul_overflow(a, b, &result);
    use_result += overflow;
}

void test_saturation(void) {
    printf("Testing saturation behavior...\n");
    
    /* Test with saturation attribute */
    unsigned short _Fract __attribute__((saturated)) sat_f1 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) sat_f2 = 0.9uhr;
    unsigned short _Fract sat_sum;
    
    sat_sum = sat_f1 + sat_f2;  /* Should saturate to max */
    use_result += (unsigned int)sat_sum;
    
    /* Overflowing conversion with saturation */
    unsigned short _Accum usa = 300.0uhk;
    unsigned short _Fract __attribute__((saturated)) usf_sat;
    
    usf_sat = (unsigned short _Fract)usa;  /* Should saturate */
    use_result += (unsigned int)usf_sat;
}

void test_mixed_conversions(void) {
    printf("Testing mixed conversions...\n");
    
    /* Integer to fixed-point with potential overflow */
    unsigned int large_int = 65536;
    unsigned _Fract from_int;
    
    from_int = (unsigned _Fract)large_int;  /* Likely overflows */
    use_result += (unsigned int)from_int;
    
    /* Fixed-point to integer with overflow */
    unsigned _Accum ua = 256.0uhk;
    unsigned char small_int;
    
    small_int = (unsigned char)ua;  /* May overflow */
    use_result += small_int;
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_i_f_bits_8();
    test_i_f_bits_16();
    test_i_f_bits_24();
    test_signed_types();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_saturation();
    test_mixed_conversions();
    
    printf("\nAll tests completed. use_result = %u\n", use_result);
    printf("(Non-zero indicates operations were performed)\n");
    
    return 0;
}
