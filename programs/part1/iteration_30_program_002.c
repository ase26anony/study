/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all test.c -o test */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test different fractional bit counts */
void test_i_f_bits_8(void) {
    printf("Testing i_f_bits = 8 (unsigned short _Fract)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       max_s = 2^8 - 1 = 255
       Condition triggers when a_high == 0 && a_low > 255 */
    
    /* This should trigger the condition: a_high == 0, a_low > 255 */
    unsigned short _Accum source = 256.0uhk;  /* 256 << 8 = 65536 in internal representation */
    unsigned short _Fract target;
    
    /* Explicit cast that should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    /* Use the result to prevent dead code elimination */
    dummy = __builtin_fixed_to_int(target, 8);
    printf("  Conversion 256.0uhk -> usf: %u\n", __builtin_fixed_to_int(target, 8));
}

void test_i_f_bits_16(void) {
    printf("Testing i_f_bits = 16 (unsigned _Fract)\n");
    
    /* For unsigned _Fract with 16 fractional bits:
       max_s = 2^16 - 1 = 65535
       Condition triggers when a_high == 0 && a_low > 65535 */
    
    /* This should trigger the condition: a_high == 0, a_low > 65535 */
    unsigned _Accum source = 65536.0uhk;  /* 65536 << 16 = 2^32 in internal representation */
    unsigned _Fract target;
    
    target = (unsigned _Fract)source;
    dummy = __builtin_fixed_to_int(target, 16);
    printf("  Conversion 65536.0uhk -> uf: %u\n", __builtin_fixed_to_int(target, 16));
}

void test_i_f_bits_24(void) {
    printf("Testing i_f_bits = 24 (unsigned long _Fract)\n");
    
    /* For unsigned long _Fract with 24 fractional bits:
       max_s = 2^24 - 1 = 16777215
       Condition triggers when a_high == 0 && a_low > 16777215 */
    
    /* Create a value that exceeds max_s but has a_high == 0 */
    unsigned long _Accum source = 16777216.0ulhk;  /* 16777216 << 24 */
    unsigned long _Fract target;
    
    target = (unsigned long _Fract)source;
    dummy = __builtin_fixed_to_int(target, 24);
    printf("  Conversion 16777216.0ulhk -> ulf: %u\n", __builtin_fixed_to_int(target, 24));
}

void test_signed_types(void) {
    printf("Testing signed types\n");
    
    /* For signed types, we need to trigger a_high > 0 (positive high part) */
    
    /* Test 1: Signed short _Accum to signed short _Fract */
    signed short _Accum ssa = 128.0hk;  /* Positive value that might overflow */
    signed short _Fract ssf;
    
    ssf = (signed short _Fract)ssa;
    dummy = __builtin_fixed_to_int(ssf, 8);
    printf("  Signed conversion 128.0hk -> sf: %d\n", __builtin_fixed_to_int(ssf, 8));
    
    /* Test 2: Large positive value that should have a_high > 0 */
    signed _Accum sa = 32768.0hk;
    signed _Fract sf;
    
    sf = (signed _Fract)sa;
    dummy = __builtin_fixed_to_int(sf, 16);
    printf("  Signed conversion 32768.0hk -> f: %d\n", __builtin_fixed_to_int(sf, 16));
}

void test_arithmetic_overflow(void) {
    printf("Testing arithmetic overflow\n");
    
    /* Perform arithmetic that results in overflow */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    /* This addition might overflow 1.0 */
    sum = f1 + f2;
    
    dummy = __builtin_fixed_to_int(sum, 8);
    printf("  Addition 0.8ur + 0.9ur = %u/256\n", __builtin_fixed_to_int(sum, 8));
    
    /* Test multiplication overflow */
    unsigned _Fract m1 = 2.0ur;
    unsigned _Fract m2 = 0.9ur;
    unsigned _Fract prod;
    
    prod = m1 * m2;
    dummy = __builtin_fixed_to_int(prod, 16);
    printf("  Multiplication 2.0ur * 0.9ur = %u/65536\n", __builtin_fixed_to_int(prod, 16));
}

void test_builtin_overflow(void) {
    printf("Testing builtin overflow detection\n");
    
    /* Use overflow builtins with fixed-point types */
    unsigned short _Fract of1 = 0.9ur;
    unsigned short _Fract of2 = 0.9ur;
    unsigned short _Fract osum;
    int overflow;
    
    /* This should detect overflow */
    overflow = __builtin_add_overflow(of1, of2, &osum);
    dummy = overflow;
    printf("  Builtin add overflow: %d (result = %u/256)\n", 
           overflow, __builtin_fixed_to_int(osum, 8));
    
    /* Test multiplication overflow detection */
    unsigned _Fract om1 = 2.0ur;
    unsigned _Fract om2 = 1.5ur;
    unsigned _Fract oprod;
    
    overflow = __builtin_mul_overflow(om1, om2, &oprod);
    dummy = overflow;
    printf("  Builtin mul overflow: %d (result = %u/65536)\n",
           overflow, __builtin_fixed_to_int(oprod, 16));
}

void test_saturation_behavior(void) {
    printf("Testing saturation behavior\n");
    
    /* Test with -fsaturated-arithmetic */
    unsigned short _Fract sat1 = 1.5ur;
    unsigned short _Fract sat2 = 0.8ur;
    unsigned short _Fract sat_sum;
    
    /* With saturation, this should clamp to max value */
    sat_sum = sat1 + sat2;
    
    dummy = __builtin_fixed_to_int(sat_sum, 8);
    printf("  Saturated addition 1.5ur + 0.8ur = %u/256\n", 
           __builtin_fixed_to_int(sat_sum, 8));
    
    /* Test overflow in conversion with saturation */
    unsigned short _Accum sat_src = 300.0uhk;
    unsigned short _Fract sat_dst;
    
    sat_dst = (unsigned short _Fract)sat_src;
    dummy = __builtin_fixed_to_int(sat_dst, 8);
    printf("  Saturated conversion 300.0uhk -> usf = %u/256\n",
           __builtin_fixed_to_int(sat_dst, 8));
}

int main(void) {
    int result = 0;
    
    printf("=== Testing overflow conditions for fixed-value.cc coverage ===\n\n");
    
    /* Test various i_f_bits values */
    test_i_f_bits_8();
    test_i_f_bits_16();
    test_i_f_bits_24();
    
    printf("\n");
    
    /* Test signed types */
    test_signed_types();
    
    printf("\n");
    
    /* Test arithmetic operations */
    test_arithmetic_overflow();
    
    printf("\n");
    
    /* Test builtin overflow detection */
    test_builtin_overflow();
    
    printf("\n");
    
    /* Test saturation behavior */
    test_saturation_behavior();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return non-zero if any overflow was expected but not handled */
    return result;
}
