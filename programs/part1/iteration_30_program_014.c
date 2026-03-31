/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test_fixed_overflow test_fixed_overflow.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Test different i_f_bits values through different fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_unsigned_short_fract_overflow() {
    printf("Test 1: unsigned short _Fract (i_f_bits=8)\n");
    
    /* Maximum value for unsigned short _Fract is (2^8-1)/2^8 = 255/256 ≈ 0.996 */
    unsigned short _Fract usf_max = 0.99609375ur;  /* 255/256 */
    unsigned short _Fract usf_test;
    
    /* This should trigger overflow check with a_high == 0, a_low > max_s */
    /* max_s = (2^8 - 1) = 255 */
    /* We need a value where high part is 0 and low part > 255 */
    usf_test = 1.0ur;  /* 1.0 = 256/256, low part = 256 > 255 */
    
    /* Force the conversion/assignment */
    asm volatile("" : "+r"(usf_test));
    
    printf("  usf_max = %u/%u = %f\n", 
           (unsigned)(usf_max * 256), 256,
           (double)usf_max);
    printf("  usf_test = %u/%u = %f\n", 
           (unsigned)(usf_test * 256), 256,
           (double)usf_test);
    
    global_counter++;
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_unsigned_fract_overflow() {
    printf("\nTest 2: unsigned _Fract (i_f_bits=16)\n");
    
    /* Maximum value is (2^16-1)/2^16 = 65535/65536 ≈ 0.9999847 */
    unsigned _Fract uf_max = 0.9999847412109375ur;  /* 65535/65536 */
    unsigned _Fract uf_test;
    
    /* Trigger overflow: 1.0 = 65536/65536, low part = 65536 > 65535 */
    uf_test = 1.0ur;
    
    asm volatile("" : "+r"(uf_test));
    
    printf("  uf_max = %u/%u = %.15f\n",
           (unsigned)(uf_max * 65536), 65536,
           (double)uf_max);
    printf("  uf_test = %u/%u = %.15f\n",
           (unsigned)(uf_test * 65536), 65536,
           (double)uf_test);
    
    global_counter++;
}

/* Test 3: i_f_bits = 8 with positive high part (signed _Accum) */
void test_signed_accum_overflow() {
    printf("\nTest 3: signed short _Accum (i_f_bits=8)\n");
    
    /* For signed short _Accum with 8 fractional bits:
       Max positive: (2^(15-1)-1)/2^8 = 16383/256 ≈ 63.996 */
    signed short _Accum ssa_max = 63.99609375hk;
    signed short _Accum ssa_test;
    
    /* Trigger overflow with positive high part (a_high > 0) */
    /* 64.0 = 16384/256, which exceeds max */
    ssa_test = 64.0hk;
    
    asm volatile("" : "+r"(ssa_test));
    
    printf("  ssa_max = %d/%d = %f\n",
           (int)(ssa_max * 256), 256,
           (double)ssa_max);
    printf("  ssa_test = %d/%d = %f\n",
           (int)(ssa_test * 256), 256,
           (double)ssa_test);
    
    global_counter++;
}

/* Test 4: i_f_bits = 24 (unsigned long _Fract) */
void test_unsigned_long_fract_overflow() {
    printf("\nTest 4: unsigned long _Fract (i_f_bits=24)\n");
    
    /* Maximum: (2^24-1)/2^24 = 16777215/16777216 ≈ 0.99999994 */
    unsigned long _Fract ulf_max = 0.999999940395355224609375ur;
    unsigned long _Fract ulf_test;
    
    /* Trigger overflow: 1.0 = 16777216/16777216 */
    ulf_test = 1.0ur;
    
    asm volatile("" : "+r"(ulf_test));
    
    printf("  ulf_max = %.10f\n", (double)ulf_max);
    printf("  ulf_test = %.10f\n", (double)ulf_test);
    
    global_counter++;
}

/* Test 5: Cross-type conversions that trigger overflow */
void test_cross_type_conversions() {
    printf("\nTest 5: Cross-type conversions\n");
    
    /* Convert from type with more range to type with less range */
    unsigned _Accum ua = 300.0uk;  /* 300.0 in 16.16 format */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check */
    usf = (unsigned short _Fract)ua;  /* 300.0 >> 8 fractional bits */
    
    asm volatile("" : "+r"(usf));
    
    printf("  ua = %f, usf = %f\n", (double)ua, (double)usf);
    
    /* Another test with signed types */
    signed _Accum sa = 100.0k;
    signed short _Fract ssf;
    
    ssf = (signed short _Fract)sa;
    
    asm volatile("" : "+r"(ssf));
    
    printf("  sa = %f, ssf = %f\n", (double)sa, (double)ssf);
    
    global_counter++;
}

/* Test 6: Arithmetic operations that cause overflow */
void test_arithmetic_overflow() {
    printf("\nTest 6: Arithmetic overflow\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    /* 0.8 + 0.9 = 1.7 which exceeds max of ~0.996 */
    sum = f1 + f2;
    
    asm volatile("" : "+r"(sum));
    
    printf("  f1 = %f, f2 = %f, sum = %f\n", 
           (double)f1, (double)f2, (double)sum);
    
    /* Test multiplication overflow */
    unsigned short _Fract f3 = 1.5ur;  /* Already overflowed */
    unsigned short _Fract f4 = 0.7ur;
    unsigned short _Fract prod;
    
    prod = f3 * f4;
    
    asm volatile("" : "+r"(prod));
    
    printf("  f3 = %f, f4 = %f, prod = %f\n",
           (double)f3, (double)f4, (double)prod);
    
    global_counter++;
}

/* Test 7: Using __builtin_add_overflow with fixed-point */
void test_builtin_overflow() {
    printf("\nTest 7: Builtin overflow checks\n");
    
    unsigned short _Fract of1 = 0.8ur;
    unsigned short _Fract of2 = 0.9ur;
    unsigned short _Fract osum;
    int overflow_flag;
    
    /* This builtin should trigger the overflow checking code */
    overflow_flag = __builtin_add_overflow(of1, of2, &osum);
    
    printf("  of1 + of2 overflow? %d (osum = %f)\n", 
           overflow_flag, (double)osum);
    
    global_counter++;
}

/* Test 8: Saturated arithmetic */
void test_saturated_arithmetic() {
    printf("\nTest 8: Saturated arithmetic\n");
    
    /* Use attribute to enable saturation */
    unsigned short _Fract __attribute__((saturated)) sf1 = 0.8ur;
    unsigned short _Fract __attribute__((saturated)) sf2 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) ssum;
    
    /* With saturation, this should clamp to max value */
    ssum = sf1 + sf2;
    
    asm volatile("" : "+r"(ssum));
    
    printf("  sf1 + sf2 with saturation = %f\n", (double)ssum);
    
    global_counter++;
}

/* Test 9: Different i_f_bits values through explicit scaling */
void test_various_i_f_bits() {
    printf("\nTest 9: Various i_f_bits values\n");
    
    /* Test i_f_bits = 1 */
    /* Create a custom fixed-point type simulation */
    {
        /* For i_f_bits=1, max_s = (2^1 - 1) = 1 */
        /* We need a value > 0.5 to trigger overflow */
        unsigned _Fract f1 = 0.6ur;  /* Will be scaled differently internally */
        asm volatile("" : "+r"(f1));
    }
    
    /* Test i_f_bits = 32 (if supported) */
    {
        unsigned long _Fract f2 = 1.0ur;
        asm volatile("" : "+r"(f2));
    }
    
    global_counter++;
}

int main() {
    printf("Starting fixed-point overflow tests...\n");
    printf("=====================================\n");
    
    /* Run all tests to trigger different overflow scenarios */
    test_unsigned_short_fract_overflow();
    test_unsigned_fract_overflow();
    test_signed_accum_overflow();
    test_unsigned_long_fract_overflow();
    test_cross_type_conversions();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_saturated_arithmetic();
    test_various_i_f_bits();
    
    printf("\n=====================================\n");
    printf("All tests completed. Global counter: %d\n", global_counter);
    
    /* Return non-zero if any test might have failed */
    /* In practice, we'd check actual results, but for coverage we just run */
    return 0;
}
