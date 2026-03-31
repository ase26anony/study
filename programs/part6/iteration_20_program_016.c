/* Test program to trigger fixed-point range analysis overflow checks in GCC */
/* Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test.c -o test.o */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *p, int size) {
    asm volatile("" : : "r"(p), "r"(size) : "memory");
}

/* Helper to create complex expressions */
__attribute__((noinline, noipa))
unsigned _Accum compute_unsigned_range(unsigned _Accum a, unsigned _Accum b) {
    /* This should trigger unsigned max range checks */
    unsigned _Accum temp = a * b;
    /* Left shift to potentially overflow */
    temp = temp << 1;
    return temp;
}

__attribute__((noinline, noipa))
_Accum compute_signed_range(_Accum a, _Accum b) {
    /* Complex expression to force range analysis */
    _Accum t1 = a * b;
    _Accum t2 = t1 + (_Accum)0.5k;
    _Accum t3 = t2 << 2;  /* Left shift - may overflow */
    return t3;
}

__attribute__((noinline, noipa))
long _Accum compute_long_accum_range(long _Accum a, long _Accum b) {
    /* Near-maximum values for long accum */
    long _Accum product = a * b;
    /* Additional operations to force range checks */
    product = product + (long _Accum)0.0000001lk;
    product = product << 1;  /* Potential overflow */
    return product;
}

__attribute__((noinline, noipa))
_Fract compute_fract_range(_Fract a, _Fract b) {
    /* Operations near -1.0 and 1.0 boundaries */
    _Fract sum = a + b;
    _Fract scaled = sum * (_Fract)0.9999999r;
    /* Conditional based on value range */
    if (scaled > (_Fract)0.9r) {
        scaled = scaled << 1;  /* May overflow for large values */
    }
    return scaled;
}

__attribute__((noinline, noipa))
unsigned _Fract compute_unsigned_fract_range(unsigned _Fract a, unsigned _Fract b) {
    /* Very close to 1.0ur */
    unsigned _Fract sum = a + b;
    if (sum > (unsigned _Fract)0.9999999ur) {
        /* This addition might wrap */
        sum = sum + (unsigned _Fract)0.0000001ur;
    }
    return sum;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[10];
    unsigned _Accum uresults[10];
    long _Accum lresults[10];
    _Fract fresults[10];
    unsigned _Fract ufresults[10];
    
    int i;
    
    /* Initialize with volatile values to prevent constant folding */
    int seed1 = vi;
    unsigned int seed2 = vu;
    long seed3 = vl;
    
    /* Loop with varying values to force range analysis */
    for (i = 0; i < 10; i++) {
        /* Create values near boundaries using seeds */
        _Accum a = (_Accum)((seed1 + i * 100) % 1000) / 1000.0k;
        _Accum b = (_Accum)((seed2 + i * 200) % 1000) / 1000.0k;
        
        /* Make values approach max/min */
        if (i % 2 == 0) {
            a = (_Accum)0.9999999k - a;
            b = (_Accum)0.9999999k - b;
        } else {
            a = (_Accum)-0.9999999k + a;
            b = (_Accum)-0.9999999k + b;
        }
        
        /* Compute with near-boundary values */
        results[i] = compute_signed_range(a, b);
        
        /* Unsigned accum tests */
        unsigned _Accum ua = (unsigned _Accum)((seed1 + i * 300) % 1000) / 1000.0uk;
        unsigned _Accum ub = (unsigned _Accum)((seed2 + i * 400) % 1000) / 1000.0uk;
        ua = (unsigned _Accum)0.9999999uk - ua;
        ub = (unsigned _Accum)0.9999999uk - ub;
        uresults[i] = compute_unsigned_range(ua, ub);
        
        /* Long accum tests - values very close to max */
        long _Accum la = (long _Accum)((seed3 + i * 500) % 10000) / 10000.0lk;
        long _Accum lb = (long _Accum)((seed3 + i * 600) % 10000) / 10000.0lk;
        la = (long _Accum)0.9999999999lk - la;
        lb = (long _Accum)0.9999999999lk - lb;
        lresults[i] = compute_long_accum_range(la, lb);
        
        /* Fract tests */
        _Fract fa = (_Fract)((seed1 + i * 700) % 1000) / 1000.0r;
        _Fract fb = (_Fract)((seed2 + i * 800) % 1000) / 1000.0r;
        fa = (_Fract)0.9999999r - fa;
        fb = (_Fract)0.9999999r - fb;
        fresults[i] = compute_fract_range(fa, fb);
        
        /* Unsigned fract tests */
        unsigned _Fract ufa = (unsigned _Fract)((seed1 + i * 900) % 1000) / 1000.0ur;
        unsigned _Fract ufb = (unsigned _Fract)((seed2 + i * 1000) % 1000) / 1000.0ur;
        ufa = (unsigned _Fract)0.9999999ur - ufa;
        ufb = (unsigned _Fract)0.9999999ur - ufb;
        ufresults[i] = compute_unsigned_fract_range(ufa, ufb);
        
        /* Mix with integer promotions */
        int int_val = (seed1 + i) % 256;
        results[i] = results[i] + (_Accum)int_val * (_Accum)0.5k;
        
        /* Conditional assignment based on overflow-like check */
        if (results[i] > (_Accum)0.999k) {
            results[i] = (_Accum)0.999k;  /* Simulate saturation */
        }
        
        if (uresults[i] > (unsigned _Accum)0.999uk) {
            uresults[i] = (unsigned _Accum)0.999uk;
        }
    }
    
    /* Force compiler to analyze all paths */
    consume_result(results, sizeof(results));
    consume_result(uresults, sizeof(uresults));
    consume_result(lresults, sizeof(lresults));
    consume_result(fresults, sizeof(fresults));
    consume_result(ufresults, sizeof(ufresults));
    
    /* Return hash of results to prevent optimization */
    int hash = 0;
    for (i = 0; i < 10; i++) {
        hash ^= *(int*)&results[i];
        hash ^= *(int*)&uresults[i];
        hash ^= *(int*)&lresults[i];
        hash ^= *(int*)&fresults[i];
        hash ^= *(int*)&ufresults[i];
    }
    
    return hash & 0xFF;
}
