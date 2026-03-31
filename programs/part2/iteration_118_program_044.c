/* Test program to exercise fixed-point range calculations in GCC */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test_fixed.c */

#include <stdio.h>

/* Fixed-point type declarations covering various modes */
static const unsigned short _Fract usf_max = 0.999999hr;
static const signed short _Fract ssf_min = -0.999999hr;
static const unsigned _Fract uf_max = 0.999999r;
static const signed _Fract sf_min = -0.999999r;
static const unsigned long _Fract ulf_max = 0.999999lr;
static const signed long _Fract slf_min = -0.999999lr;

/* Accumulator types with saturation */
static const unsigned _Sat _Accum usa_max = 255.999999k;
static const signed _Sat _Accum ssa_min = -128.999999k;
static const unsigned long _Sat _Accum ulsa_max = 65535.999999lk;
static const signed long _Sat _Accum slsa_min = -32768.999999lk;

/* Mixed precision struct for aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned long _Accum ula;
    signed long _Sat _Accum slsa;
};

/* Array initialized with fixed-point constants at boundaries */
static const struct FixedPointData fp_array[] = {
    {0.999999hr, -0.999999r, 65535.999999lk, -32768.999999lk},
    {0.5hr, 0.0r, 32767.999999lk, 0.0lk},
    {0.0hr, 0.999999r, 0.0lk, 32767.999999lk}
};

/* Compile-time constant expression using fixed-point */
#define CHECK_OVERFLOW(x) \
    (__builtin_constant_p(x) ? \
     ((x) > (typeof(x))0.999999 ? (typeof(x))0.999999 : (x)) : (x))

/* Force constant folding with ternary operator */
#define FOLD_CONSTANT(fp_val) \
    (sizeof(fp_val) == sizeof(short _Fract) ? fp_val * fp_val : \
     sizeof(fp_val) == sizeof(_Fract) ? fp_val + fp_val : \
     fp_val - fp_val)

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Basic fixed-point operations at boundaries */
    unsigned _Sat _Fract usf1 = usf_max;
    usf1 = usf1 + usf1;  /* Should saturate to max */
    
    signed _Sat _Accum ssa1 = ssa_min;
    ssa1 = ssa1 - 1.0k;  /* Should saturate at min */
    
    /* 2. Constant folding with ternary operators */
    const unsigned _Fract uf_const = (uf_max > 0.5r) ? uf_max * 2.0r : uf_max / 2.0r;
    const signed long _Fract slf_const = (slf_min < -0.5lr) ? slf_min + slf_min : slf_min - slf_min;
    
    /* 3. Mixed-type conversions triggering range checks */
    int int_from_fract = (int)uf_max;  /* Conversion to integer */
    float float_from_accum = (float)ulsa_max;  /* Conversion to float */
    
    /* 4. Loop with fixed iterations for unrolling */
    unsigned _Accum accum = 0.0k;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (accum > 128.0k) {
            accum = accum - 64.0k;
        } else {
            accum = accum + usa_max / 4;  /* Use constant at boundary */
        }
        
        /* Array access with fixed-point derived index */
        int idx = (int)(accum / 64.0k);
        if (idx >= 0 && idx < 3) {
            accum = accum + (unsigned _Accum)fp_array[idx].usf;
        }
    }
    
    /* 5. Compile-time conditional using preprocessor */
#if 1  /* Always true, but forces evaluation */
    const signed _Fract sf_compiled = CHECK_OVERFLOW(sf_min * 2.0r);
    result += (int)(sf_compiled * 100.0r);
#endif
    
    /* 6. Complex expression with multiple conversions */
    unsigned long _Sat _Accum ulsa_result = 
        (unsigned long _Sat _Accum)((float)ulsa_max * 0.999f) +
        (unsigned long _Sat _Accum)((int)usf_max * 100);
    
    /* 7. Force evaluation of FOLD_CONSTANT macro */
    const unsigned short _Fract folded_usf = FOLD_CONSTANT(usf_max);
    const signed long _Fract folded_slf = FOLD_CONSTANT(slf_min);
    
    /* 8. Operations that should trigger overflow detection */
    signed _Sat _Accum test_overflow = ssa_min;
    for (int i = 0; i < 3; i++) {
        test_overflow = test_overflow * 2.0k;  /* Should saturate */
    }
    
    /* 9. Boundary value comparisons */
    int cmp1 = (usa_max == 255.999999k);
    int cmp2 = (ssa_min <= -128.999999k);
    int cmp3 = (uf_max >= 0.999999r);
    
    /* Prevent optimization of all variables */
    result += (int)usf1 + (int)ssa1 + (int)accum + (int)ulsa_result;
    result += int_from_fract + (int)float_from_accum;
    result += cmp1 + cmp2 + cmp3;
    result += (int)folded_usf + (int)folded_slf;
    result += (int)test_overflow;
    
    printf("Result: %d\n", result);
    
    return 0;
}
