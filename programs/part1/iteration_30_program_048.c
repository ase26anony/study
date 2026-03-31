/* test-fixed-value-coverage.c
 * Test program to trigger uncovered lines in fixed-value.cc (lines 264-277)
 * Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test test-fixed-value-coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
#define KEEP_USED(var) asm volatile("" : : "r"(var))

/* Test different i_f_bits values through various fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) 
 * max_s = (2^8 - 1) = 255, max_r = 0
 * Need a_high == 0 && a_low > 255
 */
void test_unsigned_short_fract_overflow() {
    printf("Test 1: unsigned short _Fract overflow (i_f_bits=8)\n");
    
    /* Create value that exceeds max_s when converted */
    unsigned short _Accum source = 256.0uhk;  /* 256 in 8.8 format */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check:
     * source = 256 = 0x10000 in 8.8 format
     * When converting to 0.8 format:
     *   a_high = 0 (since we're looking at integer part)
     *   a_low = 0x100 (256 decimal) > max_s (255)
     */
    target = (unsigned short _Fract)source;
    
    KEEP_USED(target);
    printf("  Conversion result: %u/256\n", (unsigned)(target * 256));
}

/* Test 2: i_f_bits = 16 (unsigned _Fract)
 * max_s = (2^16 - 1) = 65535, max_r = 0
 */
void test_unsigned_fract_overflow() {
    printf("Test 2: unsigned _Fract overflow (i_f_bits=16)\n");
    
    unsigned _Accum source = 65536.0uk;  /* 65536 in 16.16 format */
    unsigned _Fract target;
    
    /* source = 65536 = 0x100000000 in 16.16 format
     * Converting to 0.16 format:
     *   a_high = 0
     *   a_low = 0x10000 (65536) > max_s (65535)
     */
    target = (unsigned _Fract)source;
    
    KEEP_USED(target);
    printf("  Conversion result: %u/65536\n", (unsigned)(target * 65536));
}

/* Test 3: i_f_bits = 8 with positive high part (a_high > max_r)
 * max_r = 0, so any positive high part triggers overflow
 */
void test_signed_accum_overflow() {
    printf("Test 3: signed _Accum overflow with positive high part\n");
    
    /* Use long long _Accum to ensure we have a high part > 0 */
    signed long long _Accum source = 32768.0llk;  /* Large value */
    signed short _Accum target;  /* Fewer integer bits */
    
    /* This should trigger a_high > max_r (0) */
    target = (signed short _Accum)source;
    
    KEEP_USED(target);
    printf("  Conversion attempted\n");
}

/* Test 4: i_f_bits = 24 (unsigned long _Fract) */
void test_unsigned_long_fract_overflow() {
    printf("Test 4: unsigned long _Fract overflow (i_f_bits=24)\n");
    
    unsigned long _Accum source = 16777216.0ulk;  /* 2^24 */
    unsigned long _Fract target;
    
    /* source = 2^24, max_s = 2^24 - 1 */
    target = (unsigned long _Fract)source;
    
    KEEP_USED(target);
    printf("  Conversion attempted\n");
}

/* Test 5: Using builtins that might trigger overflow checking */
void test_builtin_overflow() {
    printf("Test 5: Using overflow builtins\n");
    
    unsigned short _Fract f1 = 0.9uhr;
    unsigned short _Fract f2 = 0.9uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* Multiplication that might overflow */
    overflow = __builtin_mul_overflow((unsigned short)f1 * 256, 
                                     (unsigned short)f2 * 256, 
                                     (unsigned short*)&result);
    
    KEEP_USED(overflow);
    KEEP_USED(result);
    printf("  Builtin overflow check: %d\n", overflow);
}

/* Test 6: Saturated arithmetic (might use different code path) */
void test_saturated_arithmetic() {
    printf("Test 6: Saturated arithmetic\n");
    
    /* Use attribute to force saturation behavior */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) sum;
    
    /* This addition might saturate */
    sum = f1 + f2;
    
    KEEP_USED(sum);
    printf("  Saturated sum: %u/256\n", (unsigned)(sum * 256));
}

/* Test 7: Mixed-type operations */
void test_mixed_type_conversions() {
    printf("Test 7: Mixed type conversions\n");
    
    /* Chain of conversions that might trigger overflow checks */
    unsigned _Accum a1 = 1000.0uk;
    unsigned short _Fract f1;
    unsigned _Fract f2;
    
    /* Multiple conversions with potential overflow */
    f1 = (unsigned short _Fract)a1;  /* Likely overflow */
    f2 = (unsigned _Fract)f1;        /* Back conversion */
    
    /* Arithmetic that might overflow */
    unsigned _Fract f3 = f2 + f2 + f2;
    
    KEEP_USED(f1);
    KEEP_USED(f2);
    KEEP_USED(f3);
    printf("  Mixed conversions completed\n");
}

/* Test 8: Edge case with i_f_bits = 1 */
void test_minimal_fractional_bits() {
    printf("Test 8: Minimal fractional bits (i_f_bits=1)\n");
    
    /* For i_f_bits=1, max_s = 1, max_r = 0 */
    /* We need a value where low part > 1 */
    unsigned _Accum source = 2.5uk;  /* Binary: 10.1 */
    /* Hypothetical type with 1 fractional bit */
    /* Using short _Fract which has at least 8 bits, so simulate with shift */
    
    /* Alternative: use saturating arithmetic attribute */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.99uhr;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.99uhr;
    unsigned short _Fract product = f1 * f2;
    
    KEEP_USED(source);
    KEEP_USED(product);
    printf("  Edge case test completed\n");
}

/* Test 9: Runtime computation that can't be fully constant folded */
void test_runtime_overflow(unsigned _Accum input) {
    printf("Test 9: Runtime overflow check\n");
    
    /* Force runtime conversion to prevent compile-time optimization */
    unsigned short _Fract result;
    
    /* This should trigger overflow check at runtime/compile-time */
    result = (unsigned short _Fract)input;
    
    KEEP_USED(result);
    printf("  Runtime conversion: input=%u.%u, result=%u/256\n",
           (unsigned)(input), 
           (unsigned)((input - (unsigned _Accum)(unsigned)(input)) * 1000),
           (unsigned)(result * 256));
}

/* Main driver that calls all tests */
int main() {
    printf("Starting fixed-value.cc coverage tests...\n\n");
    
    test_unsigned_short_fract_overflow();
    printf("\n");
    
    test_unsigned_fract_overflow();
    printf("\n");
    
    test_signed_accum_overflow();
    printf("\n");
    
    test_unsigned_long_fract_overflow();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_saturated_arithmetic();
    printf("\n");
    
    test_mixed_type_conversions();
    printf("\n");
    
    test_minimal_fractional_bits();
    printf("\n");
    
    /* Test with different runtime values */
    for (int i = 0; i < 5; i++) {
        test_runtime_overflow(100.0uk * i + 200.0uk);
    }
    
    printf("\nAll tests completed.\n");
    return 0;
}
