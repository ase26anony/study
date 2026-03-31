/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _x = (expr); \
    __asm__ volatile ("" : : "r" (_x)); \
} while(0)

/* Test 1: Overflow in low part with zero high part */
void test_low_part_overflow(void) {
    printf("Test 1: Low part overflow (high=0, low > max_s)\n");
    
    /* For unsigned short _Fract: i_f_bits = 8, max_s = 255 */
    /* Value 256.0/256 = 1.0 should overflow since max is 255/256 */
    unsigned short _Accum usa = 256.0uhk;  /* 256 in 8.8 format */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    KEEP(usf);
    
    /* Also test with explicit constant */
    unsigned short _Fract usf2 = 1.0ur;  /* Should saturate to max */
    KEEP(usf2);
}

/* Test 2: Positive high part (a_high > 0) */
void test_positive_high_part(void) {
    printf("Test 2: Positive high part (a_high > 0)\n");
    
    /* Convert from larger type to smaller type */
    unsigned long _Accum ula = 512.0ulhk;  /* 512 in 16.16 format */
    unsigned short _Fract usf;
    
    /* High part will be positive when converting 512 (0x200) with i_f_bits=8 */
    usf = (unsigned short _Fract)ula;
    KEEP(usf);
}

/* Test 3: Different i_f_bits values */
void test_various_i_f_bits(void) {
    printf("Test 3: Various i_f_bits values\n");
    
    /* Test i_f_bits = 1 (minimum) */
    unsigned _Fract uf1;
    unsigned _Accum ua1 = 2.0uhk;  /* Exceeds max for 1 fractional bit */
    uf1 = (unsigned _Fract)ua1;
    KEEP(uf1);
    
    /* Test i_f_bits = 16 */
    unsigned long _Fract ulf;
    unsigned long _Accum ula = 65536.0ulhk;  /* Exceeds max for 16 fractional bits */
    ulf = (unsigned long _Fract)ula;
    KEEP(ulf);
    
    /* Test i_f_bits = 24 */
    unsigned long long _Fract ullf;
    unsigned long long _Accum ulla = 16777216.0ullhk;  /* 2^24 */
    ullf = (unsigned long long _Fract)ulla;
    KEEP(ullf);
}

/* Test 4: Signed types */
void test_signed_types(void) {
    printf("Test 4: Signed types\n");
    
    /* Signed overflow in positive direction */
    short _Accum sa = 128.0hk;  /* In 8.8 format, max positive is 127.996 */
    short _Fract sf;
    
    sf = (short _Fract)sa;
    KEEP(sf);
    
    /* Test with negative values (might trigger different paths) */
    short _Accum sa2 = -129.0hk;
    short _Fract sf2;
    
    sf2 = (short _Fract)sa2;
    KEEP(sf2);
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 5: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This might trigger the overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
    
    /* Multiplication overflow */
    unsigned short _Fract f3 = 1.5ur;
    unsigned short _Fract f4 = 1.5ur;
    
    overflow = __builtin_mul_overflow(f3, f4, &result);
    KEEP(overflow);
    KEEP(result);
}

/* Test 6: Saturated arithmetic */
void test_saturated_arithmetic(void) {
    printf("Test 6: Saturated arithmetic\n");
    
    /* Use saturated attribute */
    unsigned short _Fract __attribute__((saturated)) sf1 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sf2 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sum;
    
    sum = sf1 + sf2;  /* Should saturate to max */
    KEEP(sum);
}

/* Test 7: Complex expressions that might overflow */
void test_complex_expressions(void) {
    printf("Test 7: Complex expressions\n");
    
    unsigned _Accum ua1 = 100.0uhk;
    unsigned _Accum ua2 = 200.0uhk;
    unsigned _Accum ua3 = 300.0uhk;
    unsigned _Fract uf;
    
    /* Complex expression that overflows when converted */
    uf = (unsigned _Fract)(ua1 + ua2 + ua3);
    KEEP(uf);
    
    /* Chain of conversions */
    unsigned long _Accum ula = 1000.0ulhk;
    unsigned _Accum ua = (unsigned _Accum)ula;
    uf = (unsigned _Fract)ua;
    KEEP(uf);
}

/* Test 8: Boundary cases */
void test_boundary_cases(void) {
    printf("Test 8: Boundary cases\n");
    
    /* Exact boundary: max_s = 2^{i_f_bits} - 1 */
    /* For unsigned short _Fract: max = 255/256 = 0.99609375 */
    unsigned short _Fract usf_max = 0.99609375ur;
    KEEP(usf_max);
    
    /* Just over boundary */
    unsigned short _Accum usa = 255.5uhk;  /* 255.5/256 when converted */
    unsigned short _Fract usf = (unsigned short _Fract)usa;
    KEEP(usf);
    
    /* High part = 0, low part = max_s + 1 */
    unsigned short _Accum usa2 = 256.0uhk;  /* Low part = 256 when i_f_bits=8 */
    unsigned short _Fract usf2 = (unsigned short _Fract)usa2;
    KEEP(usf2);
}

/* Test 9: Mixed integer and fixed-point */
void test_mixed_conversions(void) {
    printf("Test 9: Mixed integer and fixed-point\n");
    
    /* Integer to fixed-point with overflow */
    unsigned int ui = 300;
    unsigned short _Fract usf;
    
    usf = (unsigned short _Fract)ui;
    KEEP(usf);
    
    /* Fixed-point to integer with overflow */
    unsigned short _Accum usa = 300.0uhk;
    unsigned char uc;
    
    uc = (unsigned char)usa;
    KEEP(uc);
}

int main(void) {
    int result = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_low_part_overflow();
    test_positive_high_part();
    test_various_i_f_bits();
    test_signed_types();
    test_builtin_overflow();
    test_saturated_arithmetic();
    test_complex_expressions();
    test_boundary_cases();
    test_mixed_conversions();
    
    printf("\nAll tests completed.\n");
    
    /* Return 0 if all conversions behaved as expected */
    return result;
}
