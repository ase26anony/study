/* test_fixed_range.c - Target GCC's fixed-point range analysis */
#include <stdint.h>

/* Prevent constant folding */
volatile int vi = 100;
volatile unsigned int vu = 200;
volatile long vl = 300;

/* Dummy consumer to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) sink = cp[i];
    (void)sink;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[16];
    unsigned _Fract uf_results[16];
    int result_idx = 0;
    
    /* Initialize with volatile seeds */
    volatile int seed1 = vi;
    volatile int seed2 = vu;
    volatile long seed3 = vl;
    
    /* Test 1: Signed _Accum near maximum range */
    for (int i = 0; i < 4; i++) {
        /* Create values near max representable _Accum */
        _Accum a = (_Accum)(seed1 + i) / 128.0k;  /* ~0.78k */
        _Accum b = (_Accum)(seed2 - i) / 64.0k;   /* ~1.56k */
        
        /* Multiplication that could overflow range */
        _Accum prod = a * b;
        
        /* Left shift to potentially exceed max */
        int shift = (i % 2) + 1;
        _Accum shifted = prod;
        for (int s = 0; s < shift; s++) {
            shifted = shifted + shifted;  /* Simulate << 1 */
        }
        
        /* Complex expression with intermediate overflow check */
        _Accum temp = shifted;
        if (temp > 0.9k) {
            temp = temp * 0.9999999k;  /* Very close to 1 */
        }
        
        results[result_idx++] = temp;
    }
    
    /* Test 2: Unsigned _Fract near 1.0 */
    volatile unsigned int u_seed = vu;
    for (int i = 0; i < 4; i++) {
        unsigned _Fract uf = (unsigned _Fract)(u_seed + i) / 256.0ur;
        if (uf > 0.5ur) {
            /* Operations that could wrap past 1.0 */
            uf = uf + 0.4999999ur;
            uf = uf * 1.0000001ur;  /* Slightly > 1 multiplier */
        }
        
        /* Shift-like operation through multiplication */
        uf = uf * 2.0ur;  /* This should saturate for values > 0.5 */
        
        uf_results[i] = uf;
    }
    
    /* Test 3: Long _Accum with extreme values */
    volatile long lseed = seed3;
    for (int i = 0; i < 4; i++) {
        long _Accum la = (long _Accum)lseed / 32768.0lk;
        long _Accum lb = (long _Accum)(lseed * 2) / 32768.0lk;
        
        /* Product that mathematically exceeds fixed-point range */
        long _Accum lprod = la * lb;
        
        /* Chain operations to force range analysis */
        long _Accum ltemp = lprod;
        for (int j = 0; j < 3; j++) {
            ltemp = ltemp * 0.999999999lk;
        }
        
        /* Conditional with overflow check */
        long _Accum lfinal = (ltemp > 0.5lk) ? 
            (ltemp * 1.9999999lk) : (ltemp * 0.5lk);
        
        /* Convert to _Accum (narrower type) */
        results[result_idx++] = (_Accum)lfinal;
    }
    
    /* Test 4: Mixed signed/unsigned with casts */
    volatile int mseed = vi * 2;
    for (int i = 0; i < 4; i++) {
        /* Start with integer */
        int ival = mseed + i * 50;
        
        /* Convert to fixed-point with scaling */
        _Fract f1 = (_Fract)ival / 256.0r;
        _Fract f2 = 0.9999999r;
        
        /* Multiplication near limits */
        _Fract fprod = f1 * f2;
        
        /* Check if we're at maximum (simulating overflow detection) */
        _Fract fmax = 0.9999999r;
        _Fract fresult;
        
        /* This comparison should invoke range analysis */
        if (fprod > fmax) {
            fresult = fmax;  /* Saturate */
        } else if (fprod < -fmax) {
            fresult = -fmax;
        } else {
            fresult = fprod;
        }
        
        results[result_idx++] = (_Accum)fresult;
    }
    
    /* Test 5: Short fixed-point types */
    volatile short hs = 100;
    for (int i = 0; i < 2; i++) {
        short _Fract sf = (short _Fract)hs / 128.0hr;
        short _Accum sa = (short _Accum)(hs * 2) / 128.0hk;
        
        /* Operations that could overflow short types */
        short _Accum sprod = sa * (_Accum)sf;
        
        /* Convert through intermediate _Accum */
        _Accum interm = (_Accum)sprod * 2.0k;
        results[result_idx++] = interm;
    }
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 16) result_idx = 16;
    
    /* Consume results to prevent optimization */
    consume(results, sizeof(results[0]) * result_idx);
    consume(uf_results, sizeof(uf_results));
    
    /* Return hash of results */
    unsigned int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Create hash from fixed-point bits */
        union { _Accum f; unsigned int u; } converter;
        converter.f = results[i];
        hash ^= converter.u + (hash << 6) + (hash >> 2);
    }
    
    for (int i = 0; i < 4; i++) {
        union { unsigned _Fract f; unsigned char u; } converter;
        converter.f = uf_results[i];
        hash ^= converter.u + (hash << 6) + (hash >> 2);
    }
    
    return (int)(hash % 256);
}
