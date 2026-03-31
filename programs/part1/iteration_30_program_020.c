/* Test program to trigger uncovered bounds checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Test 1: Overflow in low part with zero high part */
void test_low_part_overflow(void) {
    /* For unsigned short _Fract: i_f_bits = 8, max_s = 255 */
    /* We need a_high = 0 and a_low > 255 */
    
    /* Create a value that when converted to unsigned short _Fract 
       has high part = 0 and low part > 255 */
    unsigned short _Accum usa = 1.0uhk;  /* 1.0 in 8.8 format */
    unsigned short _Fract usf;
    
    /* Multiply to get value > 255/256 */
    usa = usa * 1.5uhk;  /* Should be 1.5 in 8.8 format = 384 */
    
    /* This conversion should trigger the overflow check:
       a_high = 0 (since 384 < 65536), a_low = 384 > 255 */
    usf = (unsigned short _Fract)usa;
    
    /* Use the result to prevent dead code elimination */
    if (usf != 0) global_counter++;
}

/* Test 2: Positive high part case */
void test_positive_high_part(void) {
    /* We need a_high > 0 */
    /* Use a larger type that can hold values > 1.0 */
    
    unsigned long _Accum ula = 2.0ULK;  /* 2.0 in 16.16 format */
    unsigned short _Fract usf;
    
    /* This should have a_high > 0 when converted to 8 fractional bits */
    usf = (unsigned short _Fract)ula;
    
    if (usf != 0) global_counter++;
}

/* Test 3: Signed types with positive high part */
void test_signed_positive_high(void) {
    short _Accum sa = 128.0hk;  /* Large positive value */
    short _Fract sf;
    
    /* This should trigger overflow with positive high part */
    sf = (short _Fract)sa;
    
    if (sf != 0) global_counter++;
}

/* Test 4: Using builtins that might trigger overflow checking */
void test_builtin_overflow(void) {
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This addition might overflow and trigger checking */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    if (overflow) global_counter++;
}

/* Test 5: Different fractional bit counts */
void test_various_i_f_bits(void) {
    /* Test with _Fract (8 bits), _Accum (16 bits), long _Accum (32 bits) */
    
    /* 8 fractional bits */
    {
        unsigned _Fract uf;
        unsigned _Accum ua = 300.0uhk;  /* > 255 */
        uf = (unsigned _Fract)ua;
        if (uf != 0) global_counter++;
    }
    
    /* 16 fractional bits */
    {
        unsigned _Accum ua1;
        unsigned long _Accum ula = 70000.0ULK;  /* > 65535 */
        ua1 = (unsigned _Accum)ula;
        if (ua1 != 0) global_counter++;
    }
    
    /* 32 fractional bits (if supported) */
    {
        unsigned long _Accum ula1;
        /* Use a very large value that would overflow when converted */
        unsigned long long value = 0x100000000ULL;  /* 2^32 */
        ula1 = (unsigned long _Accum)value;
        if (ula1 != 0) global_counter++;
    }
}

/* Test 6: Arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract sum;
    
    /* This sum (0.8 + 0.8 = 1.6) exceeds 1.0 for unsigned fract */
    sum = f1 + f2;
    
    if (sum != 0) global_counter++;
}

/* Test 7: Explicit overflow with constants */
void test_constant_overflow(void) {
    /* Direct assignment that should overflow */
    unsigned short _Fract f = 1.5ur;  /* > 1.0 */
    
    if (f != 0) global_counter++;
}

/* Test 8: Saturated arithmetic */
void test_saturated_arithmetic(void) {
    /* With saturation, overflow should still be checked */
    _Sat unsigned short _Fract f1 = 0.9ur;
    _Sat unsigned short _Fract f2 = 0.9ur;
    _Sat unsigned short _Fract sum;
    
    sum = f1 + f2;  /* Should saturate to 1.0ur */
    
    if (sum == 1.0ur) global_counter++;
}

/* Test 9: Mixed-type expressions */
void test_mixed_expressions(void) {
    int i = 256;  /* > 255 */
    unsigned short _Fract f;
    
    f = (unsigned short _Fract)i;  /* Should trigger overflow */
    
    if (f != 0) global_counter++;
}

/* Test 10: Complex expression that might overflow during evaluation */
void test_complex_expression(void) {
    unsigned short _Accum a = 100.0uhk;
    unsigned short _Accum b = 100.0uhk;
    unsigned short _Accum c = 100.0uhk;
    unsigned short _Fract f;
    
    /* (100 + 100 + 100) / 256 = 300/256 > 1.0 */
    f = (unsigned short _Fract)(a + b + c);
    
    if (f != 0) global_counter++;
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n");
    
    test_low_part_overflow();
    printf("Test 1 completed\n");
    
    test_positive_high_part();
    printf("Test 2 completed\n");
    
    test_signed_positive_high();
    printf("Test 3 completed\n");
    
    test_builtin_overflow();
    printf("Test 4 completed\n");
    
    test_various_i_f_bits();
    printf("Test 5 completed\n");
    
    test_arithmetic_overflow();
    printf("Test 6 completed\n");
    
    test_constant_overflow();
    printf("Test 7 completed\n");
    
    test_saturated_arithmetic();
    printf("Test 8 completed\n");
    
    test_mixed_expressions();
    printf("Test 9 completed\n");
    
    test_complex_expression();
    printf("Test 10 completed\n");
    
    printf("All tests completed. Global counter: %d\n", global_counter);
    
    /* Return non-zero if any test triggered overflow checking */
    return global_counter == 0 ? 0 : 1;
}
