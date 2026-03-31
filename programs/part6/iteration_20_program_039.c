/* Fixed-point range analysis test targeting GCC's fixed-value.cc */
/* Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test.c -o test.o */

#include <stdint.h>

/* Prevent optimization of inputs */
volatile int vi = 0;
volatile unsigned int vu = 0;
volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

/* Helper to create complex expressions */
__attribute__((noinline, noipa))
unsigned _Fract get_ufract_near_one(unsigned seed) {
    /* Return value very close to 1.0ur */
    unsigned _Fract base = 0.9999999ur;
    /* Small adjustment based on seed to prevent constant folding */
    unsigned _Fract adj = (seed & 0xF) * 0.00000001ur;
    return (base + adj) > 1.0ur ? 1.0ur : (base + adj);
}

__attribute__((noinline, noipa))
_Accum get_accum_near_max(int seed) {
    /* Return value near maximum _Accum */
    _Accum base = 0.999999999k;
    _Accum adj = (seed & 0x7) * 0.000000001k;
    return base + adj;
}

__attribute__((noinline, noipa))
long _Accum get_long_accum_near_max(long seed) {
    /* Return value near maximum long _Accum */
    long _Accum base = 0.999999999999999999k;
    long _Accum adj = (seed & 0xF) * 0.000000000000000001k;
    return base + adj;
}

int main(void) {
    /* Array to store results and prevent optimization */
    static _Accum results_acc[256];
    static unsigned _Fract results_ufr[256];
    static long _Accum results_lacc[256];
    static _Fract results_fr[256];
    
    int i, j;
    
    /* Test 1: Operations on unsigned _Fract near overflow boundary */
    for (i = 0; i < 128; i++) {
        unsigned _Fract u1 = get_ufract_near_one(vu + i);
        unsigned _Fract u2 = get_ufract_near_one(vu + i + 1);
        
        /* These operations should trigger range analysis near max */
        unsigned _Fract prod = u1 * u2;  /* Near 1.0 * 1.0 */
        unsigned _Fract sum = u1 + u2;   /* May overflow conceptually */
        
        /* Complex expression with intermediate values */
        unsigned _Fract temp = prod;
        for (j = 0; j < 3; j++) {
            temp = temp * u1;
        }
        
        /* Conditional that depends on overflow-like behavior */
        results_ufr[i] = (temp > 0.9999999ur) ? 1.0ur : temp;
        
        /* Explicit check against maximum */
        if (sum == 1.0ur) {
            results_ufr[i + 128] = 0.5ur;
        } else {
            results_ufr[i + 128] = sum;
        }
    }
    
    /* Test 2: Signed _Accum operations with near-maximum values */
    for (i = 0; i < 64; i++) {
        _Accum a1 = get_accum_near_max(vi + i);
        _Accum a2 = get_accum_near_max(vi + i * 2);
        
        /* Multiplication that could overflow the fixed-point range */
        _Accum prod = a1 * a2;
        
        /* Left-shift simulation through multiplication by power of 2 */
        _Accum shifted = prod * 2.0k;  /* Equivalent to << 1 */
        
        /* Complex expression mixing operations */
        _Accum temp = a1;
        for (j = 0; j < 4; j++) {
            temp = temp * a2;
            /* This should trigger the sgt/ugt comparisons in fixed-value.cc */
            if (j == 2) {
                temp = temp * 1.5k;
            }
        }
        
        /* Store results with conditional */
        results_acc[i] = (temp > 0.999999999k) ? 0.999999999k : temp;
        results_acc[i + 64] = shifted;
        
        /* Test with negative values near minimum */
        _Accum neg1 = -a1;
        _Accum neg2 = -a2;
        _Accum neg_prod = neg1 * neg2;  /* Positive, but could overflow */
        results_acc[i + 128] = neg_prod;
    }
    
    /* Test 3: Long _Accum with extreme values */
    for (i = 0; i < 32; i++) {
        long _Accum la1 = get_long_accum_near_max(vl + i);
        long _Accum la2 = get_long_accum_near_max(vl + i * 3);
        
        /* Operations that should trigger range analysis for long fixed-point */
        long _Accum prod = la1 * la2;
        long _Accum sum = la1 + la2;
        
        /* Multi-step computation */
        long _Accum temp = prod;
        for (j = 0; j < 2; j++) {
            temp = temp * la1;
            temp = temp * 1.1k;  /* Slightly > 1 to push beyond max */
        }
        
        results_lacc[i] = temp;
        results_lacc[i + 32] = sum;
        
        /* Test with explicit integer conversion */
        int int_val = (vi + i) & 0xFF;
        long _Accum from_int = (long _Accum)int_val * 0.123456789k;
        results_lacc[i + 64] = from_int;
    }
    
    /* Test 4: Mixed types and conversions */
    for (i = 0; i < 32; i++) {
        /* Start with unsigned fract */
        unsigned _Fract uf = get_ufract_near_one(vu + i * 5);
        
        /* Convert to signed fract */
        _Fract sf = (_Fract)uf - 0.5r;
        
        /* Operations that cross the -1.0 to 1.0 boundary */
        _Fract temp = sf;
        for (j = 0; j < 3; j++) {
            temp = temp * sf;  /* Square, could approach 1.0 */
            if (temp > 0.9r) {
                temp = -temp;  /* Flip sign to test negative range */
            }
        }
        
        /* Final operation that could trigger min/max range check */
        _Fract scaled = temp * 2.0r;  /* Could overflow */
        
        results_fr[i] = scaled;
        results_fr[i + 32] = (scaled < -0.9999999r) ? -0.9999999r : scaled;
    }
    
    /* Test 5: Edge case - minimum values */
    for (i = 0; i < 16; i++) {
        /* Minimum signed fract */
        _Fract min_fract = -1.0r;
        _Fract almost_min = -0.9999999r;
        
        /* Operations that could underflow */
        _Fract diff = min_fract - almost_min;
        _Fract prod = min_fract * almost_min;  /* Near +1.0 */
        
        /* This should trigger the min_s comparison logic */
        results_fr[i + 64] = diff;
        results_fr[i + 80] = prod;
        
        /* Test with _Accum */
        _Accum min_accum = -1.0k;
        _Accum almost_min_accum = -0.999999999k;
        _Accum accum_prod = min_accum * almost_min_accum;
        results_acc[i + 192] = accum_prod;
    }
    
    /* Prevent dead code elimination */
    consume(results_acc, sizeof(results_acc));
    consume(results_ufr, sizeof(results_ufr));
    consume(results_lacc, sizeof(results_lacc));
    consume(results_fr, sizeof(results_fr));
    
    /* Return hash of results to ensure all computations matter */
    unsigned int hash = 0;
    unsigned char *p = (unsigned char *)results_acc;
    for (i = 0; i < sizeof(results_acc); i++) {
        hash = (hash * 31) + p[i];
    }
    
    return hash & 0xFF;
}
