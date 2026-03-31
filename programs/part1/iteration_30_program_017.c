/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed_overflow test_fixed_overflow.c */

#include <stdio.h>

/* Helper to prevent optimization */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test different fractional bit counts */
void test_i_f_bits_8(void) {
    printf("Testing i_f_bits = 8 (unsigned short _Fract)\n");
    
    /* For i_f_bits=8, max_s = 2^8 - 1 = 255 */
    /* Case 1: High part = 0, low part > 255 */
    unsigned short _Fract f1;
    unsigned short _Accum a1 = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    
    /* Convert to _Fract - should trigger overflow since 1.0 > 255/256 */
    f1 = (unsigned short _Fract)a1;
    KEEP(f1);
    printf("  Conversion 1.0uhk -> us _Fract: %u/256\n", (unsigned)(f1 * 256));
    
    /* Case 2: High part > 0 */
    unsigned short _Accum a2 = 256.0uhk;  /* High part will be > 0 */
    unsigned short _Fract f2;
    
    f2 = (unsigned short _Fract)a2;
    KEEP(f2);
    printf("  Conversion 256.0uhk -> us _Fract: %u/256\n", (unsigned)(f2 * 256));
}

void test_i_f_bits_16(void) {
    printf("\nTesting i_f_bits = 16 (unsigned _Fract)\n");
    
    /* For i_f_bits=16, max_s = 2^16 - 1 = 65535 */
    unsigned _Fract f1;
    unsigned _Accum a1 = 1.0uk;  /* 1.0 in unsigned _Accum */
    
    /* Convert to _Fract - should trigger overflow */
    f1 = (unsigned _Fract)a1;
    KEEP(f1);
    printf("  Conversion 1.0uk -> u _Fract: %u/65536\n", (unsigned)(f1 * 65536));
    
    /* Test with value that has high part > 0 */
    unsigned _Accum a2 = 65536.0uk;
    unsigned _Fract f2;
    
    f2 = (unsigned _Fract)a2;
    KEEP(f2);
    printf("  Conversion 65536.0uk -> u _Fract: %u/65536\n", (unsigned)(f2 * 65536));
}

void test_i_f_bits_24(void) {
    printf("\nTesting i_f_bits = 24 (unsigned long _Fract)\n");
    
    /* For i_f_bits=24, max_s = 2^24 - 1 = 16777215 */
    unsigned long _Fract f1;
    unsigned long _Accum a1 = 1.0ulk;  /* 1.0 in unsigned long _Accum */
    
    /* Convert to _Fract - should trigger overflow */
    f1 = (unsigned long _Fract)a1;
    KEEP(f1);
    printf("  Conversion 1.0ulk -> ul _Fract: value stored\n");
    
    /* Test with value that has high part > 0 */
    unsigned long _Accum a2 = 16777216.0ulk;  /* 2^24 */
    unsigned long _Fract f2;
    
    f2 = (unsigned long _Fract)a2;
    KEEP(f2);
    printf("  Conversion 16777216.0ulk -> ul _Fract: value stored\n");
}

void test_signed_types(void) {
    printf("\nTesting signed types\n");
    
    /* Signed types work differently - they use sign extension */
    /* But the overflow check logic should still be triggered */
    short _Fract f1;
    short _Accum a1 = 1.0hk;  /* 1.0 in short _Accum */
    
    /* Convert to _Fract - may trigger overflow check */
    f1 = (short _Fract)a1;
    KEEP(f1);
    printf("  Signed conversion 1.0hk -> s _Fract: %d/128\n", (int)(f1 * 128));
    
    /* Test negative overflow too */
    short _Accum a2 = -1.0hk;
    short _Fract f2;
    
    f2 = (short _Fract)a2;
    KEEP(f2);
    printf("  Signed conversion -1.0hk -> s _Fract: %d/128\n", (int)(f2 * 128));
}

void test_arithmetic_overflow(void) {
    printf("\nTesting arithmetic overflow\n");
    
    /* Perform arithmetic that overflows */
    unsigned short _Fract f1 = 0.8uhr;
    unsigned short _Fract f2 = 0.5uhr;
    unsigned short _Fract sum;
    
    /* This addition might overflow 1.0 */
    sum = f1 + f2;
    KEEP(sum);
    printf("  Addition 0.8uhr + 0.5uhr = %u/256\n", (unsigned)(sum * 256));
    
    /* Multiplication that overflows */
    unsigned short _Fract f3 = 0.9uhr;
    unsigned short _Fract f4 = 0.9uhr;
    unsigned short _Fract product;
    
    product = f3 * f4;
    KEEP(product);
    printf("  Multiplication 0.9uhr * 0.9uhr = %u/256\n", (unsigned)(product * 256));
}

void test_builtin_overflow(void) {
    printf("\nTesting builtin overflow detection\n");
    
    /* Use builtins that might trigger the overflow checking code */
    unsigned short _Fract f1 = 0.8uhr;
    unsigned short _Fract f2 = 0.5uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* __builtin_add_overflow might use similar overflow checking */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(result);
    KEEP(overflow);
    printf("  Builtin add overflow check: result=%u/256, overflow=%d\n", 
           (unsigned)(result * 256), overflow);
    
    /* Test multiplication overflow */
    unsigned short _Fract f3 = 0.9uhr;
    unsigned short _Fract f4 = 0.9uhr;
    
    overflow = __builtin_mul_overflow(f3, f4, &result);
    KEEP(result);
    KEEP(overflow);
    printf("  Builtin mul overflow check: result=%u/256, overflow=%d\n",
           (unsigned)(result * 256), overflow);
}

void test_mixed_conversions(void) {
    printf("\nTesting mixed integer/fixed-point conversions\n");
    
    /* Convert from integer to fixed-point with overflow */
    unsigned int large_int = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract f1;
    
    f1 = (unsigned short _Fract)large_int;
    KEEP(f1);
    printf("  Integer 300 -> us _Fract: %u/256\n", (unsigned)(f1 * 256));
    
    /* Convert from fixed-point to integer with overflow */
    unsigned short _Accum a1 = 300.0uhk;
    unsigned int i1;
    
    i1 = (unsigned int)a1;
    KEEP(i1);
    printf("  Fixed 300.0uhk -> uint: %u\n", i1);
}

int main(void) {
    printf("=== Testing fixed-point overflow conditions ===\n");
    printf("Target: Trigger lines 264-277 in fixed-value.cc\n");
    printf("Condition: a_high > 0 OR (a_high == 0 AND a_low > 2^{i_f_bits}-1)\n\n");
    
    test_i_f_bits_8();
    test_i_f_bits_16();
    test_i_f_bits_24();
    test_signed_types();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_mixed_conversions();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
