/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test_fixed_overflow test_fixed_overflow.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { asm volatile("" : : "r"(expr)); } while(0)

/* Test 1: Overflow when high part is positive (a_high.sgt(max_r)) */
void test_positive_high_part(void) {
    printf("Test 1: Positive high part overflow\n");
    
    /* Use _Accum types which have more integer bits */
    long _Accum la = 256.0lk;  /* Value > 1.0 */
    
    /* Convert to _Fract which has no integer bits (only fractional) */
    /* This should trigger a_high > 0 condition */
    unsigned short _Fract usf = (unsigned short _Fract)la;
    
    KEEP(usf);
    printf("  Converted %Lfk to %#x\n", (long double)la, (unsigned int)usf);
}

/* Test 2: Overflow when high part is 0 but low part > max_s */
void test_low_part_overflow(void) {
    printf("Test 2: Low part overflow (high part = 0, low > max_s)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       max_s = (2^8 - 1) = 255
       We need a value where high part = 0, low part > 255 */
    
    /* Create a value slightly less than 1.0 but with low part > 255 */
    /* 0.999... in fixed-point representation */
    unsigned _Accum ua = 0.999999ur;  /* Very close to 1.0 */
    
    /* Convert to unsigned short _Fract (8 fractional bits) */
    /* The representation might have low part > 255 */
    unsigned short _Fract usf = (unsigned short _Fract)ua;
    
    KEEP(usf);
    printf("  Converted %ur to %#x\n", ua, (unsigned int)usf);
}

/* Test 3: Different i_f_bits values - using _Sat types */
void test_saturated_types(void) {
    printf("Test 3: Saturated types with different i_f_bits\n");
    
    /* Test with 16 fractional bits */
    unsigned long _Fract ulf = 1.0ULr;
    unsigned short _Fract usf = (unsigned short _Fract)ulf;  /* 16->8 bits */
    
    /* Test with 24 fractional bits (long _Accum) */
    long _Accum la = 128.0lk;
    short _Accum sa = (short _Accum)la;  /* More integer bits -> fewer */
    
    KEEP(usf);
    KEEP(sa);
    printf("  Conversions done\n");
}

/* Test 4: Arithmetic operations causing overflow */
void test_arithmetic_overflow(void) {
    printf("Test 4: Arithmetic overflow\n");
    
    /* Addition that exceeds 1.0 */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.3ur;
    unsigned short _Fract sum = f1 + f2;  /* 1.1 > 1.0 */
    
    /* Multiplication that exceeds range */
    unsigned short _Accum a1 = 2.0uhk;
    unsigned short _Accum a2 = 2.0uhk;
    unsigned short _Accum prod = a1 * a2;  /* 4.0 > max for short _Accum? */
    
    KEEP(sum);
    KEEP(prod);
    printf("  Arithmetic results: sum=%#x, prod=%#x\n", 
           (unsigned int)sum, (unsigned int)prod);
}

/* Test 5: Explicit casts with overflow */
void test_explicit_overflows(void) {
    printf("Test 5: Explicit overflow casts\n");
    
    /* Integer to fixed-point with overflow */
    int large_int = 1000;
    unsigned short _Fract from_int = (unsigned short _Fract)large_int;
    
    /* Between different fixed-point types */
    unsigned long _Accum ula = 300.0ULK;
    unsigned short _Accum usa = (unsigned short _Accum)ula;
    
    KEEP(from_int);
    KEEP(usa);
    printf("  Cast results: from_int=%#x, usa=%#x\n",
           (unsigned int)from_int, (unsigned int)usa);
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 6: Builtin overflow checks\n");
    
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin might trigger the overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    KEEP(result);
    KEEP(overflow);
    printf("  Builtin add overflow: result=%#x, overflow=%d\n",
           (unsigned int)result, overflow);
}

/* Test 7: Edge cases with minimum i_f_bits */
void test_edge_cases(void) {
    printf("Test 7: Edge cases\n");
    
    /* Test with very few fractional bits */
    /* _Sat types might use different code paths */
    unsigned _Fract uf_sat __attribute__((saturated)) = 1.5ur;
    unsigned _Fract uf2_sat __attribute__((saturated)) = 1.5ur;
    unsigned _Fract sum_sat = uf_sat + uf2_sat;
    
    /* Direct assignment that should overflow */
    unsigned short _Fract max_val = 1.0ur;
    unsigned short _Fract overflow_val = max_val + (unsigned short _Fract)0.1ur;
    
    KEEP(sum_sat);
    KEEP(overflow_val);
    printf("  Edge case results\n");
}

int main(void) {
    int test_result = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    /* Run all tests to maximize coverage */
    test_positive_high_part();
    test_low_part_overflow();
    test_saturated_types();
    test_arithmetic_overflow();
    test_explicit_overflows();
    test_builtin_overflow();
    test_edge_cases();
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any test showed unexpected behavior */
    return test_result;
}
