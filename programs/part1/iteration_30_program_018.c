/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    (void)_tmp; \
} while(0)

/* Test 1: Overflow when high part is positive (a_high > max_r) */
void test_positive_high_part(void) {
    printf("Test 1: Positive high part overflow\n");
    
    /* Use long _Accum with 32 fractional bits, convert to short _Fract with 8 fractional bits */
    /* long _Accum has more integer bits, so a large value will have positive high part */
    long _Accum la = 256.0lk;  /* This should have positive high part when converted */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check with a_high > 0 */
    usf = (unsigned short _Fract)la;
    KEEP(usf);
    
    /* Also test signed version */
    short _Accum sa = 128.0hk;
    short _Fract sf = (short _Fract)sa;
    KEEP(sf);
}

/* Test 2: Overflow when high part is zero but low part > max_s */
void test_low_part_overflow(void) {
    printf("Test 2: Low part overflow (high part = 0, low > 2^{i_f_bits}-1)\n");
    
    /* For unsigned short _Fract with 8 fractional bits, max_s = 255 */
    /* We need a value where high part is 0 but low part > 255 */
    
    /* Create a value just above the maximum for 8 fractional bits */
    unsigned _Accum ua = 1.0uk - 0.001uk;  /* Very close to 1.0 */
    unsigned short _Fract usf;
    
    /* This should have high part = 0, low part > 255 when scaled */
    usf = (unsigned short _Fract)ua;
    KEEP(usf);
    
    /* Another approach: use arithmetic that produces overflow */
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    /* This addition might overflow in fixed-point arithmetic */
    sum = f1 + f2;
    KEEP(sum);
}

/* Test 3: Different i_f_bits values */
void test_various_fractional_bits(void) {
    printf("Test 3: Various fractional bit counts\n");
    
    /* Test with 16 fractional bits (short _Accum) */
    {
        int i = 100000;
        short _Accum sa = (short _Accum)i;
        short _Fract sf = (short _Fract)sa;  /* Should overflow */
        KEEP(sf);
    }
    
    /* Test with 24 fractional bits (_Accum) */
    {
        long l = 1000000L;
        _Accum a = (_Accum)l;
        _Fract f = (_Fract)a;  /* Should overflow */
        KEEP(f);
    }
    
    /* Test with 32 fractional bits (long _Accum) */
    {
        long long ll = 1000000000LL;
        long _Accum la = (long _Accum)ll;
        long _Fract lf = (long _Fract)la;  /* Should overflow */
        KEEP(lf);
    }
}

/* Test 4: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 4: Builtin overflow detection\n");
    
    /* These builtins might trigger the same overflow checking logic */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Simulate overflow check */
    result = f1 + f2;
    KEEP(result);
    
    /* Use volatile to force runtime evaluation */
    volatile unsigned short _Fract vf1 = 0.9ur;
    volatile unsigned short _Fract vf2 = 0.9ur;
    unsigned short _Fract sum = vf1 + vf2;
    KEEP(sum);
}

/* Test 5: Saturation behavior */
void test_saturation(void) {
    printf("Test 5: Saturation behavior\n");
    
    /* With saturation, values should clamp to max */
    unsigned _Sat short _Fract s1 = 0.9ur;
    unsigned _Sat short _Fract s2 = 0.9ur;
    unsigned _Sat short _Fract ssum = s1 + s2;
    
    KEEP(ssum);
    
    /* Test overflow in conversion with saturation */
    unsigned _Sat short _Accum usa = 256.0uhk;
    unsigned _Sat short _Fract usf = (unsigned _Sat short _Fract)usa;
    KEEP(usf);
}

/* Test 6: Edge cases */
void test_edge_cases(void) {
    printf("Test 6: Edge cases\n");
    
    /* Exactly at the boundary */
    unsigned short _Fract max_val = 0.99609375ur;  /* 255/256 for 8 fractional bits */
    KEEP(max_val);
    
    /* Just over the boundary */
    unsigned _Accum just_over = 1.0uk;
    unsigned short _Fract converted = (unsigned short _Fract)just_over;
    KEEP(converted);
    
    /* Negative overflow for signed types */
    short _Accum neg_big = -129.0hk;
    short _Fract neg_converted = (short _Fract)neg_big;
    KEEP(neg_converted);
}

/* Test 7: Complex expressions that might trigger overflow checking */
void test_complex_expressions(void) {
    printf("Test 7: Complex expressions\n");
    
    /* Nested expressions that overflow */
    unsigned short _Fract a = 0.5ur;
    unsigned short _Fract b = 0.6ur;
    unsigned short _Fract c = 0.7ur;
    
    /* This sum might overflow depending on optimization */
    unsigned short _Fract sum = a + b + c;
    KEEP(sum);
    
    /* Multiplication that overflows */
    unsigned short _Fract m1 = 0.9ur;
    unsigned short _Fract m2 = 0.9ur;
    unsigned short _Fract prod = m1 * m2;
    KEEP(prod);
}

int main(void) {
    int tests_passed = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    /* Run all tests */
    test_positive_high_part();
    tests_passed++;
    
    test_low_part_overflow();
    tests_passed++;
    
    test_various_fractional_bits();
    tests_passed++;
    
    test_builtin_overflow();
    tests_passed++;
    
    test_saturation();
    tests_passed++;
    
    test_edge_cases();
    tests_passed++;
    
    test_complex_expressions();
    tests_passed++;
    
    printf("\nAll %d tests completed.\n", tests_passed);
    printf("Note: The actual overflow checking happens at compile-time in GCC's middle-end.\n");
    printf("To verify coverage, compile with debugging flags:\n");
    printf("  gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all -da test.c\n");
    
    return 0;
}
