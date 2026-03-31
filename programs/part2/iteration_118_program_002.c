/* Test program for fixed-point range calculation coverage */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -o fixed_test fixed_test.c */

#include <stdio.h>

/* Force compile-time evaluation with constexpr */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum satacc;
    long _Accum lacc;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.99999hr, -0.99999r, 0.99999ur, 255.99999k, 32767.99999lk},
    {0.00001hr, -0.00001r, 0.00001ur, -255.99999k, -32767.99999lk},
    {0.5hr, -0.5r, 0.5ur, 127.5k, 16383.5lk}
};

int main(void) {
    /* Test 1: Extreme values at representable limits */
    const unsigned _Fract max_uf = 0.999999r;  /* Max unsigned fract */
    const unsigned _Fract min_uf = 0.000001r;  /* Min unsigned fract */
    const signed _Fract max_sf = 0.999999r;    /* Max signed fract */
    const signed _Fract min_sf = -1.0r;        /* Min signed fract */
    
    /* Test 2: Saturation types with overflow/underflow */
    unsigned _Sat _Fract usat1 = 0.8r;
    unsigned _Sat _Fract usat2 = 0.9r;
    signed _Sat _Accum ssat1 = 127.999k;
    signed _Sat _Accum ssat2 = 127.999k;
    
    /* Test 3: Accum types with different precisions */
    short _Accum sacc = 127.999hk;
    _Accum acc = 255.999k;
    long _Accum lacc = 32767.999lk;
    long long _Accum llacc = 2147483647.999llk;
    
    /* Force constant folding with ternary operator */
    static const unsigned _Fract const_folded = 
        EVAL_CONST(0.999999r + 0.000001r) > 0.5r ? 0.999999r : 0.5r;
    
    /* Complex expression that should trigger range checking */
    volatile unsigned _Fract result1 = 
        (max_uf * max_uf) + (min_uf * min_uf);
    
    /* Operations that may overflow into saturation logic */
    usat1 = usat1 + usat2;  /* Should saturate to 0.999999r */
    ssat1 = ssat1 + ssat2;  /* Should saturate to 127.999k */
    
    /* Loop with fixed iteration for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Conditional based on fixed-point comparison */
        if (init_data[i].sf > 0.0r) {
            sacc = sacc * init_data[i].usf;
        } else {
            sacc = sacc * (-init_data[i].sf);
        }
        
        /* Mix with integer operations */
        acc = acc + (_Accum)((int)init_data[i].sf * 10);
    }
    
    /* Test conversions that require range checking */
    int int_from_fract = (int)(max_sf * 1000.0r);
    float float_from_accum = (float)lacc;
    long long_ll_from_acc = (long)(llacc / 2.0k);
    
    /* Force use of all variables to prevent optimization */
    volatile unsigned _Fract dump1 = usat1;
    volatile signed _Accum dump2 = ssat1;
    volatile short _Accum dump3 = sacc;
    volatile _Accum dump4 = acc;
    volatile long _Accum dump5 = lacc;
    volatile int dump6 = int_from_fract;
    volatile float dump7 = float_from_accum;
    volatile long dump8 = long_ll_from_acc;
    volatile unsigned _Fract dump9 = const_folded;
    
    /* Use __builtin_constant_p to create compile-time paths */
#if __builtin_constant_p(const_folded)
    /* This path only taken if compiler can evaluate const_folded at compile time */
    volatile int array_index = (int)(const_folded * 10.0r);
    int array[10] = {0};
    if (array_index >= 0 && array_index < 10) {
        array[array_index] = 1;
    }
#endif
    
    /* Test shifts with fixed-point (through integer conversion) */
    unsigned _Fract shift_test = 0.75r;
    for (int i = 0; i < 4; i++) {
        /* Convert to integer, shift, convert back */
        unsigned int temp = (unsigned int)(shift_test * 256.0r);
        temp = temp << 1;  /* May overflow */
        shift_test = (unsigned _Fract)(temp / 256.0r);
    }
    
    /* Extreme case: multiplication at limits */
    long _Accum extreme1 = 32767.999lk;
    long _Accum extreme2 = 32767.999lk;
    long _Accum product = extreme1 * extreme2;  /* Will overflow fixed-point range */
    
    /* Cast to void to avoid unused variable warnings */
    (void)dump1; (void)dump2; (void)dump3; (void)dump4;
    (void)dump5; (void)dump6; (void)dump7; (void)dump8;
    (void)dump9; (void)product; (void)shift_test;
    
    printf("Fixed-point test completed\n");
    return 0;
}
