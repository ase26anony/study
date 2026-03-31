/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all test.c */

#include <stdio.h>

/* Test 1: Unsigned fract with 8 fractional bits (i_f_bits = 8) */
void test_unsigned_fract_overflow() {
    printf("Test 1: Unsigned short _Fract overflow\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       - max_s = 2^8 - 1 = 255
       - Condition triggers when high part > 0 OR (high part == 0 AND low part > 255)
    */
    
    /* Create a value that exceeds the maximum representable value */
    unsigned short _Accum large_val = 256.0uhk;  /* 256 * 2^8 = 65536 in internal representation */
    
    /* This conversion should trigger overflow check:
       - Internal representation has high part = 0, low part = 65536
       - 65536 > 255, so condition should trigger
    */
    unsigned short _Fract f = (unsigned short _Fract)large_val;
    
    /* Prevent optimization */
    volatile unsigned short _Fract vf = f;
    printf("  Result: %hu (internal: %u)\n", (unsigned short)vf, (unsigned)vf);
}

/* Test 2: Signed fract with 7 fractional bits (i_f_bits = 7 for signed) */
void test_signed_fract_overflow() {
    printf("Test 2: Signed short _Fract overflow\n");
    
    /* Create a value that's too large */
    signed short _Accum large_val = 128.0hk;  /* Exceeds maximum for signed fract */
    
    /* This should trigger overflow check */
    signed short _Fract f = (signed short _Fract)large_val;
    
    volatile signed short _Fract vf = f;
    printf("  Result: %hd (internal: %d)\n", (short)vf, (int)vf);
}

/* Test 3: Test with high part > 0 (first part of OR condition) */
void test_high_part_positive() {
    printf("Test 3: High part positive overflow\n");
    
    /* Create a value with high part > 0 */
    unsigned _Accum very_large = 65536.0uhk;  /* High part will be > 0 */
    
    /* Convert to smaller type - should trigger a_high.sgt(max_r) */
    unsigned short _Fract f = (unsigned short _Fract)very_large;
    
    volatile unsigned short _Fract vf = f;
    printf("  Result: %hu\n", (unsigned short)vf);
}

/* Test 4: Use builtin overflow detection */
void test_builtin_overflow() {
    printf("Test 4: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    
    /* This multiplication should overflow and may trigger the check */
    int overflow = __builtin_mul_overflow(f1, f2, &result);
    
    printf("  Overflow detected: %d\n", overflow);
    printf("  Result: %hu\n", (unsigned short)result);
}

/* Test 5: Arithmetic operations causing overflow */
void test_arithmetic_overflow() {
    printf("Test 5: Arithmetic overflow\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    
    /* Addition that exceeds 1.0 - should trigger overflow */
    unsigned short _Fract sum = f1 + f2;
    
    volatile unsigned short _Fract vsum = sum;
    printf("  Sum: %hu (0x%04x)\n", (unsigned short)vsum, (unsigned)vsum);
}

/* Test 6: Different fractional bit counts */
void test_various_fractional_bits() {
    printf("Test 6: Various fractional bit counts\n");
    
    /* Test with _Fract (implementation-defined bits, often 15 or 31) */
    unsigned _Fract uf;
    unsigned _Accum ua = 2.0ur;  /* Too large for _Fract */
    uf = (unsigned _Fract)ua;
    
    volatile unsigned _Fract vuf = uf;
    printf("  _Fract conversion: %u\n", (unsigned)vuf);
    
    /* Test with long _Fract (more bits) */
    unsigned long _Fract ulf;
    unsigned long _Accum ula = 4.0ulr;
    ulf = (unsigned long _Fract)ula;
    
    volatile unsigned long _Fract vulf = ulf;
    printf("  Long _Fract conversion: %lu\n", (unsigned long)vulf);
}

/* Test 7: Explicit saturation behavior */
void test_saturation() {
    printf("Test 7: Saturation behavior\n");
    
    /* With -fsaturated-arithmetic, overflow should saturate */
    unsigned short _Fract max_val = 0.999ur;  /* Should be ~255/256 */
    unsigned short _Fract small_val = 0.1ur;
    
    /* This should saturate to maximum */
    unsigned short _Fract saturated = max_val + small_val;
    
    volatile unsigned short _Fract vsat = saturated;
    printf("  Saturated result: %hu (0x%04x)\n", 
           (unsigned short)vsat, (unsigned)vsat);
}

/* Test 8: Boundary case - exactly at max_s */
void test_boundary_case() {
    printf("Test 8: Boundary case\n");
    
    /* Create value that equals max_s (255 for 8 fractional bits) */
    unsigned short _Accum exact_max = 255.0uhk / 256.0uhk;  /* 255/256 */
    
    /* This should NOT trigger overflow (a_low == max_s) */
    unsigned short _Fract f = (unsigned short _Fract)exact_max;
    
    volatile unsigned short _Fract vf = f;
    printf("  Boundary result: %hu\n", (unsigned short)vf);
}

/* Test 9: Using volatile to prevent constant folding */
void test_volatile_conversion(volatile unsigned short _Accum input) {
    printf("Test 9: Volatile conversion\n");
    
    /* Force runtime conversion */
    unsigned short _Fract f = (unsigned short _Fract)input;
    
    volatile unsigned short _Fract vf = f;
    printf("  Volatile conversion result: %hu\n", (unsigned short)vf);
}

/* Test 10: Nested conversions with overflow */
void test_nested_conversions() {
    printf("Test 10: Nested conversions\n");
    
    /* Create overflow through multiple conversions */
    unsigned _Accum a1 = 512.0uhk;      /* Too large for short fract */
    unsigned short _Accum a2 = (unsigned short _Accum)a1;  /* First conversion */
    unsigned short _Fract f = (unsigned short _Fract)a2;   /* Second conversion - should overflow */
    
    volatile unsigned short _Fract vf = f;
    printf("  Nested conversion result: %hu\n", (unsigned short)vf);
}

int main() {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_unsigned_fract_overflow();
    printf("\n");
    
    test_signed_fract_overflow();
    printf("\n");
    
    test_high_part_positive();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_arithmetic_overflow();
    printf("\n");
    
    test_various_fractional_bits();
    printf("\n");
    
    test_saturation();
    printf("\n");
    
    test_boundary_case();
    printf("\n");
    
    test_volatile_conversion(300.0uhk);
    printf("\n");
    
    test_nested_conversions();
    printf("\n");
    
    printf("All tests completed.\n");
    
    return 0;
}
