/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed test_fixed.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Overflow when converting from larger to smaller fractional type */
void test_overflow_fract_conversion() {
    printf("Test 1: _Accum to _Fract conversion overflow\n");
    
    /* unsigned short _Accum has 8 integer bits, 8 fractional bits */
    /* unsigned short _Fract has 0 integer bits, 8 fractional bits */
    unsigned short _Accum usa = 1.5hk;  /* 1.5 in 8.8 format */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check since 1.5 > max _Fract value (255/256) */
    usf = (unsigned short _Fract)usa;
    
    dummy = (int)usf;
    printf("  usa = 1.5hk, usf = (unsigned short _Fract)usa = %u/256\n", (unsigned)usf);
}

/* Test 2: Overflow with positive high part (a_high > 0) */
void test_positive_high_part() {
    printf("\nTest 2: Positive high part overflow\n");
    
    /* long _Accum to short _Fract - large value that definitely has positive high part */
    long _Accum la = 1000.0lk;
    short _Fract sf;
    
    /* This should trigger a_high.sgt(0) condition */
    sf = (short _Fract)la;
    
    dummy = (int)sf;
    printf("  la = 1000.0lk, sf = (short _Fract)la = %d/256\n", (int)sf);
}

/* Test 3: Overflow with zero high part but low part > max_s */
void test_zero_high_large_low() {
    printf("\nTest 3: Zero high part, large low part overflow\n");
    
    /* For unsigned _Fract with 8 fractional bits, max_s = 255 */
    /* We need a value with high=0, low > 255 */
    
    /* Use _Sat types to see saturation behavior */
    unsigned _Sat _Accum usa_sat = 1.0uhk;  /* Exactly 1.0 in 8.8 format = 256 */
    unsigned _Sat _Fract usf_sat;
    
    /* 1.0 in 8.8 format has high=0, low=256 (which is > 255) */
    /* This should trigger (a_high == max_r && a_low.ugt(max_s)) */
    usf_sat = (unsigned _Sat _Fract)usa_sat;
    
    dummy = (int)usf_sat;
    printf("  usa_sat = 1.0uhk, usf_sat = %u/256 (should be 255/256 due to saturation)\n", 
           (unsigned)usf_sat);
}

/* Test 4: Different i_f_bits values - test with 16 fractional bits */
void test_16_fractional_bits() {
    printf("\nTest 4: 16 fractional bits overflow\n");
    
    /* unsigned _Accum has 8 int, 16 frac bits (8.16) */
    /* unsigned short _Fract has 0 int, 16 frac bits (0.16) */
    unsigned _Accum ua = 1.0uhk;  /* 1.0 in 8.16 format = 65536 */
    unsigned short _Fract usf16;
    
    /* max_s for 16 frac bits = 65535, so 65536 > max_s */
    usf16 = (unsigned short _Fract)ua;
    
    dummy = (int)usf16;
    printf("  ua = 1.0uhk, usf16 = (unsigned short _Fract)ua = %u/65536\n", 
           (unsigned)usf16);
}

/* Test 5: Signed types with negative overflow */
void test_signed_overflow() {
    printf("\nTest 5: Signed type overflow\n");
    
    /* Test both positive and negative overflow for signed types */
    _Sat _Accum sa_pos = 1.0hk;
    _Sat _Accum sa_neg = -1.0hk;
    _Sat _Fract sf_pos, sf_neg;
    
    /* Positive overflow */
    sf_pos = (_Sat _Fract)sa_pos;
    
    /* Negative overflow (should trigger different path but still interesting) */
    sf_neg = (_Sat _Fract)sa_neg;
    
    dummy = (int)sf_pos + (int)sf_neg;
    printf("  sa_pos = 1.0hk -> sf_pos = %d/256\n", (int)sf_pos);
    printf("  sa_neg = -1.0hk -> sf_neg = %d/256\n", (int)sf_neg);
}

/* Test 6: Arithmetic operations that cause overflow */
void test_arithmetic_overflow() {
    printf("\nTest 6: Arithmetic operation overflow\n");
    
    unsigned _Sat _Fract f1 = 0.8ur;
    unsigned _Sat _Fract f2 = 0.5ur;
    unsigned _Sat _Fract sum;
    
    /* 0.8 + 0.5 = 1.3 which exceeds max _Fract value of ~0.996 */
    sum = f1 + f2;
    
    dummy = (int)sum;
    printf("  f1 = 0.8ur, f2 = 0.5ur, f1 + f2 = %u/256 (should saturate to 255/256)\n",
           (unsigned)sum);
}

/* Test 7: Multiplication overflow */
void test_multiplication_overflow() {
    printf("\nTest 7: Multiplication overflow\n");
    
    unsigned _Sat _Fract f1 = 0.9ur;
    unsigned _Sat _Fract f2 = 0.9ur;
    unsigned _Sat _Fract product;
    
    /* 0.9 * 0.9 = 0.81, but let's try with values that overflow */
    unsigned _Sat _Accum a1 = 2.0uhk;
    unsigned _Sat _Accum a2 = 2.0uhk;
    unsigned _Sat _Accum prod_acc;
    
    product = f1 * f2;
    prod_acc = a1 * a2;  /* 4.0 in 8.8 format, converting to _Fract would overflow */
    
    dummy = (int)product + (int)prod_acc;
    printf("  f1 * f2 = %u/256\n", (unsigned)product);
    printf("  a1 * a2 = %u (in 8.8 format)\n", (unsigned)prod_acc);
}

/* Test 8: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("\nTest 8: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin might trigger the overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    dummy = (int)result + overflow;
    printf("  __builtin_add_overflow(0.5ur, 0.6ur): overflow = %d, result = %u/256\n",
           overflow, (unsigned)result);
}

/* Test 9: Explicit casts with different fractional bits */
void test_various_fractional_bits() {
    printf("\nTest 9: Various fractional bit counts\n");
    
    /* Test with 4, 12, 20, 28 fractional bits if supported */
    unsigned _Fract f4;      /* Typically 8 fractional bits */
    unsigned long _Fract f12; /* Typically 12 fractional bits? */
    
    /* Create values that just exceed the maximum for each type */
    unsigned _Accum a_for_f4 = 1.0uhk;  /* For 8 fractional bits */
    
    f4 = (unsigned _Fract)a_for_f4;
    
    dummy = (int)f4;
    printf("  Conversion that should overflow for 8 fractional bits\n");
}

int main() {
    int total_checks = 0;
    
    printf("=== Testing fixed-point overflow checking ===\n\n");
    
    test_overflow_fract_conversion();
    total_checks++;
    
    test_positive_high_part();
    total_checks++;
    
    test_zero_high_large_low();
    total_checks++;
    
    test_16_fractional_bits();
    total_checks++;
    
    test_signed_overflow();
    total_checks++;
    
    test_arithmetic_overflow();
    total_checks++;
    
    test_multiplication_overflow();
    total_checks++;
    
    test_builtin_overflow();
    total_checks++;
    
    test_various_fractional_bits();
    total_checks++;
    
    printf("\n=== All %d tests completed ===\n", total_checks);
    printf("Note: The actual overflow checking happens at compile-time\n");
    printf("in GCC's middle-end when processing fixed-point conversions.\n");
    
    return 0;
}
