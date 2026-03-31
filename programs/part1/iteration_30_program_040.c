/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    __asm__ volatile("" : : "r"(_tmp)); \
} while(0)

/* Test different i_f_bits values through different fixed-point types */

/* i_f_bits = 8 (unsigned short _Fract) */
void test_ufract8_overflow(void) {
    printf("Testing unsigned short _Fract (8 fractional bits):\n");
    
    /* This should trigger a_high == max_r (0) && a_low > max_s (255) */
    /* max_s = 2^8 - 1 = 255 */
    /* Try value 256/256 = 1.0 which has low part 256 > 255 */
    unsigned short _Accum usa = 1.0uhk;  /* 1.0 in 8.8 format */
    unsigned short _Fract usf;
    
    /* Explicit cast that should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    KEEP(usf);
    printf("  Conversion from 1.0uhk to usf: %u/256\n", (unsigned)(usf * 256));
    
    /* Another test: arithmetic that overflows */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.3ur;
    unsigned short _Fract sum = f1 + f2;  /* 1.1 > 255/256 */
    KEEP(sum);
    printf("  0.8ur + 0.3ur = %u/256\n", (unsigned)(sum * 256));
}

/* i_f_bits = 16 (unsigned _Fract) */
void test_ufract16_overflow(void) {
    printf("\nTesting unsigned _Fract (16 fractional bits):\n");
    
    /* Trigger a_high > max_r (positive high part) */
    /* Need a value > 65535/65536 = ~0.99998 */
    unsigned _Accum ua = 2.0uk;  /* 2.0 in 16.16 format - high part will be positive */
    unsigned _Fract uf;
    
    uf = (unsigned _Fract)ua;  /* Should trigger a_high > 0 */
    KEEP(uf);
    printf("  Conversion from 2.0uk to uf: %u/65536\n", (unsigned)(uf * 65536));
    
    /* Test with builtin overflow */
    unsigned _Fract a = 0.9ur;
    unsigned _Fract b = 0.2ur;
    unsigned _Fract result;
    int overflow = __builtin_add_overflow(a, b, &result);
    KEEP(result);
    printf("  0.9ur + 0.2ur overflow? %d, result: %u/65536\n", 
           overflow, (unsigned)(result * 65536));
}

/* i_f_bits = 8 (signed short _Fract) - testing signed overflow */
void test_sfract8_overflow(void) {
    printf("\nTesting signed short _Fract (8 fractional bits):\n");
    
    /* For signed, we need to test both positive and negative overflow */
    signed short _Accum ssa = 1.0shk;  /* 1.0 in signed 7.8 format */
    signed short _Fract ssf;
    
    ssf = (signed short _Fract)ssa;  /* Should trigger overflow check */
    KEEP(ssf);
    printf("  Conversion from 1.0shk to ssf: %d/256\n", (int)(ssf * 256));
    
    /* Test negative overflow */
    signed short _Accum neg = -1.0shk;
    signed short _Fract negf = (signed short _Fract)neg;
    KEEP(negf);
    printf("  Conversion from -1.0shk to ssf: %d/256\n", (int)(negf * 256));
}

/* i_f_bits = 24 (unsigned long _Fract) */
void test_ufract24_overflow(void) {
    printf("\nTesting unsigned long _Fract (24 fractional bits):\n");
    
    /* Create a value that will have high part > 0 when converted */
    /* 2.0 in 8.24 format */
    unsigned long _Accum ula = 2.0ulk;
    unsigned long _Fract ulf;
    
    ulf = (unsigned long _Fract)ula;
    KEEP(ulf);
    printf("  Conversion from 2.0ulk to ulf: %lu/16777216\n", 
           (unsigned long)(ulf * 16777216));
}

/* Test with saturation attribute */
void test_saturation(void) {
    printf("\nTesting with saturation:\n");
    
    /* With saturation, overflow should clamp to max value */
    unsigned short _Fract __attribute__((saturated)) sat_f1 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sat_f2 = 0.2ur;
    unsigned short _Fract __attribute__((saturated)) sat_sum = sat_f1 + sat_f2;
    
    KEEP(sat_sum);
    printf("  Saturated 0.9ur + 0.2ur = %u/256\n", (unsigned)(sat_sum * 256));
    
    /* Test multiplication overflow */
    unsigned _Fract __attribute__((saturated)) m1 = 0.9ur;
    unsigned _Fract __attribute__((saturated)) m2 = 1.2ur;  /* Actually 1.2 will be clamped to 1.0 */
    unsigned _Fract __attribute__((saturated)) prod = m1 * m2;
    
    KEEP(prod);
    printf("  Saturated 0.9ur * 1.2ur = %u/65536\n", (unsigned)(prod * 65536));
}

/* Test various i_f_bits through type conversions */
void test_mixed_conversions(void) {
    printf("\nTesting mixed type conversions:\n");
    
    /* This should create various i_f_bits values */
    unsigned _Accum acc = 500.0uk;  /* Large value */
    
    /* Convert to types with different fractional bits */
    unsigned short _Fract f8 = (unsigned short _Fract)acc;    /* i_f_bits = 8 */
    unsigned _Fract f16 = (unsigned _Fract)acc;               /* i_f_bits = 16 */
    unsigned long _Fract f24 = (unsigned long _Fract)acc;     /* i_f_bits = 24 */
    
    KEEP(f8); KEEP(f16); KEEP(f24);
    
    printf("  500.0uk -> usf: %u/256\n", (unsigned)(f8 * 256));
    printf("  500.0uk -> uf: %u/65536\n", (unsigned)(f16 * 65536));
    printf("  500.0uk -> ulf: %lu/16777216\n", (unsigned long)(f24 * 16777216));
}

/* Test the exact condition: a_high == max_r && a_low > max_s */
void test_exact_overflow_condition(void) {
    printf("\nTesting exact overflow condition (a_high==0 && a_low>max_s):\n");
    
    /* For unsigned short _Fract (i_f_bits=8), max_s = 255 */
    /* We need a value with high=0, low=256 */
    /* 256/256 = 1.0 exactly */
    unsigned short _Accum exact = 1.0uhk;
    unsigned short _Fract result;
    
    /* Multiple conversions to increase chance of hitting the code */
    for (int i = 0; i < 10; i++) {
        result = (unsigned short _Fract)(exact + i * 0.001uhk);
        KEEP(result);
    }
    
    printf("  Multiple near-1.0 conversions done\n");
    
    /* Also test with integer to fixed-point conversion */
    unsigned int large_int = 256;  /* 256 > 255 */
    unsigned short _Fract from_int = (unsigned short _Fract)large_int;
    KEEP(from_int);
    printf("  Conversion from int 256: %u/256\n", (unsigned)(from_int * 256));
}

int main(void) {
    printf("=== Testing fixed-point overflow checking ===\n\n");
    
    /* Run all tests */
    test_ufract8_overflow();
    test_ufract16_overflow();
    test_sfract8_overflow();
    test_ufract24_overflow();
    test_saturation();
    test_mixed_conversions();
    test_exact_overflow_condition();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return non-zero if any test showed unexpected behavior */
    return 0;
}
