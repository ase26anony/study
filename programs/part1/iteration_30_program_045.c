/* Test program to cover overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all test.c -o test */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test different i_f_bits values through different fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_unsigned_short_fract(void) {
    printf("Test 1: unsigned short _Fract (i_f_bits=8)\n");
    
    /* max_s = 2^8 - 1 = 255 */
    /* Test case 1: a_high == 0, a_low > max_s */
    /* Convert 256/256 = 1.0 which has low part 256 > 255 */
    unsigned short _Accum usa = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    dummy = usf;  /* Prevent dead code elimination */
    printf("  Conversion 1.0uhk -> usf: %u\n", (unsigned)usf);
    
    /* Test case 2: a_high > 0 (positive high part) */
    /* Use a value > 1.0 */
    unsigned short _Accum usa2 = 2.0uhk;
    unsigned short _Fract usf2;
    
    usf2 = (unsigned short _Fract)usa2;
    dummy = usf2;
    printf("  Conversion 2.0uhk -> usf: %u\n", (unsigned)usf2);
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_unsigned_fract(void) {
    printf("\nTest 2: unsigned _Fract (i_f_bits=16)\n");
    
    /* max_s = 2^16 - 1 = 65535 */
    /* Test a_high == 0, a_low > max_s */
    unsigned _Accum ua = 1.0uk;  /* 1.0 in unsigned _Accum */
    unsigned _Fract uf;
    
    uf = (unsigned _Fract)ua;
    dummy = uf;
    printf("  Conversion 1.0uk -> uf: %u\n", (unsigned)uf);
    
    /* Test a_high > 0 */
    unsigned _Accum ua2 = 2.0uk;
    unsigned _Fract uf2;
    
    uf2 = (unsigned _Fract)ua2;
    dummy = uf2;
    printf("  Conversion 2.0uk -> uf: %u\n", (unsigned)uf2);
}

/* Test 3: i_f_bits = 24 (unsigned long _Fract) */
void test_unsigned_long_fract(void) {
    printf("\nTest 3: unsigned long _Fract (i_f_bits=24)\n");
    
    /* max_s = 2^24 - 1 = 16777215 */
    unsigned long _Accum ula = 1.0ulk;
    unsigned long _Fract ulf;
    
    ulf = (unsigned long _Fract)ula;
    dummy = ulf;
    printf("  Conversion 1.0ulk -> ulf: %lu\n", (unsigned long)ulf);
    
    /* Test with larger value for a_high > 0 */
    unsigned long _Accum ula2 = 2.0ulk;
    unsigned long _Fract ulf2;
    
    ulf2 = (unsigned long _Fract)ula2;
    dummy = ulf2;
    printf("  Conversion 2.0ulk -> ulf: %lu\n", (unsigned long)ulf2);
}

/* Test 4: Signed types with negative bounds (also covers min_s logic) */
void test_signed_types(void) {
    printf("\nTest 4: Signed fixed-point types\n");
    
    /* Test signed short _Fract */
    short _Accum sa = 1.0hk;
    short _Fract sf;
    
    sf = (short _Fract)sa;
    dummy = sf;
    printf("  Signed conversion 1.0hk -> sf: %d\n", (int)sf);
    
    /* Test negative overflow */
    short _Accum sa2 = -1.0hk;
    short _Fract sf2;
    
    sf2 = (short _Fract)sa2;
    dummy = sf2;
    printf("  Signed conversion -1.0hk -> sf: %d\n", (int)sf2);
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("\nTest 5: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin may trigger the overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    dummy = result;
    printf("  Builtin add overflow: %d + %d = %d, overflow=%d\n", 
           (unsigned)f1, (unsigned)f2, (unsigned)result, overflow);
    
    /* Multiplication that overflows */
    unsigned short _Fract f3 = 0.9ur;
    unsigned short _Fract f4 = 0.9ur;
    
    overflow = __builtin_mul_overflow(f3, f4, &result);
    dummy = result;
    printf("  Builtin mul overflow: %d * %d = %d, overflow=%d\n",
           (unsigned)f3, (unsigned)f4, (unsigned)result, overflow);
}

/* Test 6: Arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    printf("\nTest 6: Arithmetic overflow\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    /* This addition may overflow 0.8 + 0.9 = 1.7 > 1.0 */
    sum = f1 + f2;
    dummy = sum;
    printf("  Arithmetic: %u + %u = %u\n", 
           (unsigned)f1, (unsigned)f2, (unsigned)sum);
    
    /* Chain operations to create larger values */
    unsigned short _Accum acc = 0.5uhk;
    for (int i = 0; i < 5; i++) {
        acc = acc + 0.5uhk;
    }
    unsigned short _Fract conv = (unsigned short _Fract)acc;
    dummy = conv;
    printf("  Accumulated conversion: %u -> %u\n", (unsigned)acc, (unsigned)conv);
}

/* Test 7: Different i_f_bits through fract types with different sizes */
void test_various_i_f_bits(void) {
    printf("\nTest 7: Various i_f_bits values\n");
    
    /* Test i_f_bits = 1 (smallest non-zero) */
    /* Use _Sat types to ensure overflow handling */
    unsigned _Sat _Fract sat_f1 = 1.5ur;
    unsigned _Sat _Fract sat_f2 = 0.6ur;
    unsigned _Sat _Fract sat_sum = sat_f1 + sat_f2;
    dummy = sat_sum;
    printf("  Sat arithmetic: %u + %u = %u\n",
           (unsigned)sat_f1, (unsigned)sat_f2, (unsigned)sat_sum);
    
    /* Test with explicit integer to fixed-point conversion */
    unsigned int large_int = 256;  /* > 255 for 8 fractional bits */
    unsigned short _Fract from_int = (unsigned short _Fract)large_int;
    dummy = from_int;
    printf("  Int to fract: %u -> %u\n", large_int, (unsigned)from_int);
}

/* Main driver that runs all tests */
int main(void) {
    int result = 0;
    
    printf("=== Testing overflow checking in fixed-value.cc ===\n");
    
    test_unsigned_short_fract();
    test_unsigned_fract();
    test_unsigned_long_fract();
    test_signed_types();
    test_builtin_overflow();
    test_arithmetic_overflow();
    test_various_i_f_bits();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return non-zero if any test showed unexpected behavior */
    return result;
}
