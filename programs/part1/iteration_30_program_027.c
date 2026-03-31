/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test test.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test different i_f_bits values through various fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_unsigned_short_fract(void) {
    printf("Test 1: unsigned short _Fract (i_f_bits=8)\n");
    
    /* Maximum value for unsigned short _Fract is (2^8-1)/2^8 = 255/256 */
    /* Try to assign a value that exceeds this */
    unsigned short _Accum large_val = 1.0hk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract f1;
    
    /* This conversion should trigger overflow check */
    f1 = (unsigned short _Fract)large_val;
    
    /* Use the value to prevent dead code elimination */
    dummy = (int)(f1 * 256);
    printf("  Conversion result: %u/256\n", (unsigned int)(f1 * 256));
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_unsigned_fract(void) {
    printf("Test 2: unsigned _Fract (i_f_bits=16)\n");
    
    /* Maximum value is (2^16-1)/2^16 = 65535/65536 */
    /* Create a value with high part = 0, low part > 2^16-1 */
    unsigned _Accum a1 = 1.0uk;  /* 1.0 in unsigned _Accum */
    unsigned _Fract f2;
    
    /* This should trigger the a_high == max_r && a_low > max_s case */
    f2 = (unsigned _Fract)a1;
    
    dummy = (int)(f2 * 65536);
    printf("  Conversion result: %u/65536\n", (unsigned int)(f2 * 65536));
}

/* Test 3: i_f_bits = 8 with signed type (short _Fract) */
void test_signed_short_fract(void) {
    printf("Test 3: short _Fract (i_f_bits=8)\n");
    
    /* For signed, we need positive overflow */
    short _Accum large_val = 1.0hk;  /* Positive value > max */
    short _Fract f3;
    
    f3 = (short _Fract)large_val;
    
    dummy = (int)(f3 * 256);
    printf("  Conversion result: %d/256\n", (int)(f3 * 256));
}

/* Test 4: i_f_bits = 24 (unsigned long _Fract) */
void test_unsigned_long_fract(void) {
    printf("Test 4: unsigned long _Fract (i_f_bits=24)\n");
    
    /* Create overflow through arithmetic */
    unsigned long _Fract f4 = 0.8ULr;
    unsigned long _Fract f5 = 0.3ULr;
    unsigned long _Fract sum;
    
    /* Sum is 1.1ULr which exceeds maximum of (2^24-1)/2^24 */
    sum = f4 + f5;
    
    dummy = (int)(sum * (1 << 24));
    printf("  Sum result: approx %u/2^24\n", (unsigned int)(sum * (1 << 24)));
}

/* Test 5: i_f_bits = 1 (edge case) */
void test_small_i_f_bits(void) {
    printf("Test 5: Testing with small i_f_bits\n");
    
    /* Use saturating arithmetic to force overflow checks */
    _Sat unsigned _Fract sf1 = 0.5ur;
    _Sat unsigned _Fract sf2 = 0.6ur;
    _Sat unsigned _Fract ssum;
    
    ssum = sf1 + sf2;  /* 1.1ur should saturate to max */
    
    dummy = (int)(ssum * 65536);
    printf("  Saturating sum: %u/65536\n", (unsigned int)(ssum * 65536));
}

/* Test 6: Using builtins that might trigger the path */
void test_with_builtins(void) {
    printf("Test 6: Using overflow builtins\n");
    
    unsigned short _Fract bf1 = 0.5uhr;
    unsigned short _Fract bf2 = 0.6uhr;
    unsigned short _Fract bresult;
    int overflow;
    
    /* Multiplication might overflow */
    overflow = __builtin_mul_overflow((int)(bf1 * 256), (int)(bf2 * 256), 
                                      &dummy);
    
    /* Force a conversion that should overflow */
    unsigned _Accum acc_val = 2.0uk;  /* Definitely > 1.0 */
    unsigned _Fract conv_result;
    
    conv_result = (unsigned _Fract)acc_val;
    
    printf("  Builtin overflow detected: %d\n", overflow);
    printf("  Conversion from 2.0uk: %u/65536\n", 
           (unsigned int)(conv_result * 65536));
}

/* Test 7: Explicit overflow by exceeding low part bound */
void test_low_part_overflow(void) {
    printf("Test 7: Testing low part overflow specifically\n");
    
    /* We need a value where high part = 0 but low part > 2^i_f_bits - 1 */
    /* For unsigned short _Fract (i_f_bits=8), create value between 1.0 and 2.0 */
    unsigned short _Accum val = 1.5uhk;  /* 1.5 in 8.8 format */
    unsigned short _Fract result;
    
    /* When converting to 0.8 format, the integer part (1) goes to high part */
    /* But wait, for unsigned, high part should still be 0 if value < 2.0 */
    /* Actually, let me think about the representation... */
    
    /* Better: use a value just slightly above 1.0 */
    unsigned short _Accum val2 = 1.0uhk + (1.0uhk / 256.0);  /* 1 + 1/256 */
    result = (unsigned short _Fract)val2;
    
    dummy = (int)(result * 256);
    printf("  Value just above 1.0: %u/256\n", (unsigned int)(result * 256));
}

/* Test 8: Integer to fixed-point conversion */
void test_int_to_fixed(void) {
    printf("Test 8: Integer to fixed-point conversion\n");
    
    /* Convert integer that's too large */
    unsigned int large_int = 2;  /* > 1 for unsigned _Fract */
    unsigned _Fract from_int;
    
    from_int = (unsigned _Fract)large_int;
    
    dummy = (int)(from_int * 65536);
    printf("  Integer 2 -> _Fract: %u/65536\n", 
           (unsigned int)(from_int * 65536));
}

/* Test 9: Different fractional bits via _Accum types */
void test_accum_types(void) {
    printf("Test 9: Testing _Accum types\n");
    
    /* unsigned short _Accum has 8 integer, 8 fractional bits */
    /* Converting to unsigned _Fract (0.16) requires different i_f_bits */
    unsigned short _Accum usa = 300.0uhk;  /* 300 in 8.8 format */
    unsigned _Fract uf;
    
    uf = (unsigned _Fract)usa;  /* Should overflow */
    
    dummy = (int)(uf * 65536);
    printf("  300.0uhk -> _Fract: %u/65536\n", (unsigned int)(uf * 65536));
}

/* Test 10: Using volatile to force runtime evaluation */
void test_volatile_conversion(void) {
    printf("Test 10: Volatile forcing\n");
    
    volatile unsigned _Accum volatile_val = 1.5uk;
    unsigned _Fract volatile_result;
    
    /* This should prevent constant folding */
    volatile_result = (unsigned _Fract)volatile_val;
    
    printf("  Volatile conversion: approx %u/65536\n", 
           (unsigned int)(volatile_result * 65536));
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_unsigned_short_fract();
    printf("\n");
    
    test_unsigned_fract();
    printf("\n");
    
    test_signed_short_fract();
    printf("\n");
    
    test_unsigned_long_fract();
    printf("\n");
    
    test_small_i_f_bits();
    printf("\n");
    
    test_with_builtins();
    printf("\n");
    
    test_low_part_overflow();
    printf("\n");
    
    test_int_to_fixed();
    printf("\n");
    
    test_accum_types();
    printf("\n");
    
    test_volatile_conversion();
    printf("\n");
    
    printf("All tests completed.\n");
    
    /* Return non-zero if any test might have triggered unexpected behavior */
    return dummy == 0 ? 0 : 1;
}
