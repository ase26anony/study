/* Test program to trigger overflow checks in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed test_fixed.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test different i_f_bits values through different fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_unsigned_short_fract() {
    printf("Test 1: unsigned short _Fract (i_f_bits=8)\n");
    
    /* Maximum representable value: (2^8-1)/2^8 = 255/256 = 0.99609375 */
    unsigned short _Fract max_val = 0.99609375ur;
    printf("  Max value: %u/256 = %f\n", 
           (unsigned)(max_val * 256.0), (double)max_val);
    
    /* Try to assign 1.0 which exceeds maximum */
    unsigned short _Fract overflow_val = 1.0ur;
    printf("  Overflow attempt (1.0): %u/256 = %f\n", 
           (unsigned)(overflow_val * 256.0), (double)overflow_val);
    
    KEEP(overflow_val);
    
    /* Conversion from larger type */
    unsigned _Accum large_val = 2.0uk;
    unsigned short _Fract converted = (unsigned short _Fract)large_val;
    printf("  Conversion from _Accum(2.0): %u/256 = %f\n",
           (unsigned)(converted * 256.0), (double)converted);
    
    KEEP(converted);
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_unsigned_fract() {
    printf("\nTest 2: unsigned _Fract (i_f_bits=16)\n");
    
    /* Maximum: (2^16-1)/2^16 = 65535/65536 */
    unsigned _Fract max_val = 0.9999847412109375ur;
    printf("  Max value: %u/65536\n", (unsigned)(max_val * 65536.0));
    
    /* Exceed by adding two values */
    unsigned _Fract f1 = 0.6ur;
    unsigned _Fract f2 = 0.5ur;
    unsigned _Fract sum = f1 + f2;  /* 1.1 > 1.0 */
    printf("  Overflow addition (0.6 + 0.5): %u/65536\n", 
           (unsigned)(sum * 65536.0));
    
    KEEP(sum);
    
    /* Multiplication that overflows */
    unsigned _Fract f3 = 0.9ur;
    unsigned _Fract f4 = 1.2ur;
    unsigned _Fract prod = f3 * f4;  /* 1.08 > 1.0 */
    printf("  Overflow multiplication (0.9 * 1.2): %u/65536\n",
           (unsigned)(prod * 65536.0));
    
    KEEP(prod);
}

/* Test 3: i_f_bits = 8 with signed type (signed short _Fract) */
void test_signed_short_fract() {
    printf("\nTest 3: signed short _Fract (i_f_bits=8)\n");
    
    /* Maximum: (2^7-1)/2^7 = 127/128 = 0.9921875 */
    signed short _Fract max_val = 0.9921875r;
    printf("  Max positive: %d/128 = %f\n", 
           (int)(max_val * 128.0), (double)max_val);
    
    /* Minimum: -1.0 */
    signed short _Fract min_val = -1.0r;
    printf("  Min negative: %d/128 = %f\n",
           (int)(min_val * 128.0), (double)min_val);
    
    /* Try to exceed positive bound */
    signed short _Fract overflow_pos = 1.0r;
    printf("  Positive overflow (1.0): %d/128 = %f\n",
           (int)(overflow_pos * 128.0), (double)overflow_pos);
    
    KEEP(overflow_pos);
    
    /* Try to exceed negative bound */
    signed short _Fract overflow_neg = -1.1r;
    printf("  Negative overflow (-1.1): %d/128 = %f\n",
           (int)(overflow_neg * 128.0), (double)overflow_neg);
    
    KEEP(overflow_neg);
}

/* Test 4: i_f_bits = 24 (unsigned long _Fract) */
void test_unsigned_long_fract() {
    printf("\nTest 4: unsigned long _Fract (i_f_bits=24)\n");
    
    /* Maximum: (2^24-1)/2^24 */
    unsigned long _Fract max_val = 0.999999940395355224609375ulr;
    printf("  Max value: approx %f\n", (double)max_val);
    
    /* Create overflow through integer conversion */
    unsigned int large_int = 2;  /* 2 > 1.0 */
    unsigned long _Fract from_int = (unsigned long _Fract)large_int;
    printf("  Conversion from int(2): approx %f\n", (double)from_int);
    
    KEEP(from_int);
}

/* Test 5: _Accum types with more integer bits */
void test_accum_types() {
    printf("\nTest 5: _Accum types\n");
    
    /* unsigned short _Accum: 8 integer bits, 8 fractional bits */
    unsigned short _Accum usa_max = 255.99609375uhk;  /* (2^8-1) + (2^8-1)/2^8 */
    printf("  unsigned short _Accum max: %f\n", (double)usa_max);
    
    /* Overflow by exceeding integer bits */
    unsigned short _Accum usa_overflow = 256.0uhk;
    printf("  unsigned short _Accum overflow (256.0): %f\n", 
           (double)usa_overflow);
    
    KEEP(usa_overflow);
    
    /* signed _Accum: 15 integer bits, 16 fractional bits */
    signed _Accum sa_max = 32767.9999847412109375k;
    printf("  signed _Accum max: %f\n", (double)sa_max);
    
    signed _Accum sa_overflow = 32768.0k;
    printf("  signed _Accum overflow (32768.0): %f\n", (double)sa_overflow);
    
    KEEP(sa_overflow);
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("\nTest 6: Builtin overflow detection\n");
    
    unsigned _Fract a = 0.7ur;
    unsigned _Fract b = 0.6ur;
    unsigned _Fract result;
    
    /* This builtin might trigger the overflow checking path */
    int overflow = __builtin_add_overflow(a, b, &result);
    printf("  __builtin_add_overflow(0.7ur, 0.6ur): overflow=%d, result=%u/65536\n",
           overflow, (unsigned)(result * 65536.0));
    
    KEEP(result);
    KEEP(overflow);
    
    /* Multiplication overflow */
    unsigned _Fract c = 0.9ur;
    unsigned _Fract d = 1.2ur;
    int overflow_mul = __builtin_mul_overflow(c, d, &result);
    printf("  __builtin_mul_overflow(0.9ur, 1.2ur): overflow=%d, result=%u/65536\n",
           overflow_mul, (unsigned)(result * 65536.0));
    
    KEEP(result);
    KEEP(overflow_mul);
}

/* Test 7: Mixed-type conversions */
void test_mixed_conversions() {
    printf("\nTest 7: Mixed-type conversions\n");
    
    /* From integer to fixed-point with overflow */
    int large_int = 1000;
    unsigned short _Fract from_large_int = (unsigned short _Fract)large_int;
    printf("  int(1000) to unsigned short _Fract: %u/256\n",
           (unsigned)(from_large_int * 256.0));
    
    KEEP(from_large_int);
    
    /* Between different fixed-point types */
    unsigned _Accum accum_val = 1.5uk;
    unsigned short _Fract fract_val = (unsigned short _Fract)accum_val;
    printf("  _Accum(1.5) to unsigned short _Fract: %u/256\n",
           (unsigned)(fract_val * 256.0));
    
    KEEP(fract_val);
    
    /* Chain of operations that might overflow */
    unsigned _Fract base = 0.8ur;
    unsigned _Fract increment = 0.3ur;
    for (int i = 0; i < 5; i++) {
        base = base + increment;
        printf("  Iteration %d: %u/65536\n", i, (unsigned)(base * 65536.0));
    }
    
    KEEP(base);
}

/* Test 8: Saturated arithmetic explicitly */
void test_saturated_arithmetic() {
    printf("\nTest 8: Saturated arithmetic\n");
    
    /* With saturation attribute */
    unsigned _Fract __attribute__((saturated)) sat_a = 0.8ur;
    unsigned _Fract __attribute__((saturated)) sat_b = 0.9ur;
    unsigned _Fract __attribute__((saturated)) sat_sum = sat_a + sat_b;
    
    printf("  Saturated addition (0.8ur + 0.9ur): %u/65536\n",
           (unsigned)(sat_sum * 65536.0));
    
    KEEP(sat_sum);
    
    /* Saturated multiplication */
    unsigned _Fract __attribute__((saturated)) sat_c = 0.95ur;
    unsigned _Fract __attribute__((saturated)) sat_d = 1.1ur;
    unsigned _Fract __attribute__((saturated)) sat_prod = sat_c * sat_d;
    
    printf("  Saturated multiplication (0.95ur * 1.1ur): %u/65536\n",
           (unsigned)(sat_prod * 65536.0));
    
    KEEP(sat_prod);
}

int main() {
    int test_result = 0;
    
    printf("=== Testing fixed-point overflow conditions ===\n");
    printf("Target: Trigger lines 264-277 in fixed-value.cc\n");
    printf("Condition: a_high > 0 OR (a_high == 0 AND a_low > 2^{i_f_bits}-1)\n\n");
    
    /* Run all tests */
    test_unsigned_short_fract();
    test_unsigned_fract();
    test_signed_short_fract();
    test_unsigned_long_fract();
    test_accum_types();
    test_builtin_overflow();
    test_mixed_conversions();
    test_saturated_arithmetic();
    
    printf("\n=== All tests completed ===\n");
    printf("Note: The actual overflow checking happens at compile-time\n");
    printf("in GCC's middle-end when processing fixed-point conversions.\n");
    printf("Check coverage with: gcov -b fixed-value.cc\n");
    
    return test_result;
}
