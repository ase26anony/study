/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Overflow in low part only (a_high == 0, a_low > max_s) */
void test_low_part_overflow(void) {
    /* For unsigned short _Fract with 8 fractional bits:
       max_s = 2^8 - 1 = 255
       We need a value with high part 0 and low part > 255 */
    
    /* Create a value that's just above 255/256 */
    unsigned short _Accum source = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    dummy = (int)target;  /* Prevent dead code elimination */
    printf("Test 1: low part overflow, result = %f\n", (double)target);
}

/* Test 2: Overflow in high part (a_high > 0) */
void test_high_part_overflow(void) {
    /* For unsigned short _Fract with 8 fractional bits:
       Any value >= 1.0 will have high part > 0 */
    
    unsigned short _Accum source = 2.0uhk;  /* Definitely > 1.0 */
    unsigned short _Fract target;
    
    target = (unsigned short _Fract)source;
    
    dummy = (int)target;
    printf("Test 2: high part overflow, result = %f\n", (double)target);
}

/* Test 3: Signed types with negative overflow */
void test_signed_overflow(void) {
    /* For signed short _Fract with 7 fractional bits (1 sign bit):
       max_s = 2^7 - 1 = 127
       We need to test both positive and negative overflow */
    
    signed short _Accum pos_overflow = 2.0hk;
    signed short _Accum neg_overflow = -2.0hk;
    signed short _Fract target1, target2;
    
    target1 = (signed short _Fract)pos_overflow;
    target2 = (signed short _Fract)neg_overflow;
    
    dummy = (int)target1 + (int)target2;
    printf("Test 3: signed overflow, pos=%f, neg=%f\n", 
           (double)target1, (double)target2);
}

/* Test 4: Different fractional bit counts */
void test_different_fbits(void) {
    /* Test with _Fract (8 bits), short _Fract (8 bits), 
       long _Fract (16 bits), long long _Fract (32 bits) */
    
    /* 8 fractional bits */
    unsigned _Fract f1;
    unsigned short _Accum a1 = 1.5uhk;
    f1 = (unsigned _Fract)a1;
    
    /* 16 fractional bits */
    unsigned long _Fract f2;
    unsigned long _Accum a2 = 1.1ulhk;
    f2 = (unsigned long _Fract)a2;
    
    /* 32 fractional bits */
    unsigned long long _Fract f3;
    unsigned long long _Accum a3 = 1.01ullhk;
    f3 = (unsigned long long _Fract)a3;
    
    dummy = (int)f1 + (int)f2 + (int)f3;
    printf("Test 4: different fbits: %f, %f, %f\n",
           (double)f1, (double)f2, (double)f3);
}

/* Test 5: Arithmetic operations causing overflow */
void test_arithmetic_overflow(void) {
    /* Addition that overflows */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.5ur;
    unsigned short _Fract sum = f1 + f2;  /* 1.3 > 255/256 */
    
    /* Multiplication that overflows */
    unsigned short _Fract f3 = 0.9ur;
    unsigned short _Fract f4 = 0.9ur;
    unsigned short _Fract prod = f3 * f4;  /* 0.81 < 1.0, but test anyway */
    
    dummy = (int)sum + (int)prod;
    printf("Test 5: arithmetic: sum=%f, prod=%f\n", (double)sum, (double)prod);
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.5ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin might trigger the overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    dummy = (int)result + overflow;
    printf("Test 6: builtin overflow=%d, result=%f\n", overflow, (double)result);
}

/* Test 7: Saturated arithmetic */
#ifdef __SAT_FRACT__
void test_saturated_arithmetic(void) {
    /* With saturation attribute */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sum = f1 + f2;
    
    dummy = (int)sum;
    printf("Test 7: saturated sum=%f\n", (double)sum);
}
#endif

/* Test 8: Edge case - exact boundary value */
void test_boundary(void) {
    /* For unsigned short _Fract: max value = 255/256 ≈ 0.99609375 */
    unsigned short _Accum boundary = 255.0uhk / 256.0uhk;
    unsigned short _Fract target;
    
    /* This should be exactly at the boundary */
    target = (unsigned short _Fract)boundary;
    
    /* Now try just above boundary */
    unsigned short _Accum above_boundary = 256.0uhk / 256.0uhk;  /* 1.0 */
    unsigned short _Fract target2 = (unsigned short _Fract)above_boundary;
    
    dummy = (int)target + (int)target2;
    printf("Test 8: boundary=%f, above=%f\n", (double)target, (double)target2);
}

/* Test 9: Mixed integer and fixed-point conversions */
void test_mixed_conversions(void) {
    /* Convert from integer to fixed-point */
    unsigned int large_int = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract from_int = (unsigned short _Fract)large_int;
    
    /* Convert from fixed-point to integer (reverse direction) */
    unsigned short _Fract f = 0.75ur;
    unsigned int to_int = (unsigned int)f;
    
    dummy = (int)from_int + to_int;
    printf("Test 9: from_int=%f, to_int=%u\n", (double)from_int, to_int);
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n");
    
    test_low_part_overflow();
    test_high_part_overflow();
    test_signed_overflow();
    test_different_fbits();
    test_arithmetic_overflow();
    test_builtin_overflow();
    
    #ifdef __SAT_FRACT__
    test_saturated_arithmetic();
    #endif
    
    test_boundary();
    test_mixed_conversions();
    
    printf("All tests completed.\n");
    
    /* Return non-zero if any test showed unexpected behavior */
    return dummy != 0 ? 1 : 0;
}
