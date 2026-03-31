/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks in GCC
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c -o test.o
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
    (void)sink;
}

/* Hash function to return meaningful value */
__attribute__((noinline))
int compute_hash(_Accum *arr, int n) {
    int hash = 0;
    for (int i = 0; i < n; i++) {
        /* Access bits to create hash */
        union {
            _Accum f;
            int i;
        } u;
        u.f = arr[i];
        hash ^= u.i ^ (i * 0x5bd1e995);
    }
    return hash;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[16] = {0k};
    int result_idx = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = vi;
    volatile unsigned int useed = vu;
    volatile long lseed = vl;
    
    /* Test 1: Signed accumulative types near maximum range */
    for (int i = 0; i < 4; i++) {
        /* Values that approach max representable _Accum */
        _Accum a = (_Accum)(seed + i) / 256k;  /* Varying input */
        _Accum b = 0.999999999k;  /* Very close to max */
        
        /* Multiplication that could overflow */
        _Accum prod = a * b;
        
        /* Left shift that could overflow */
        _Accum shifted = prod << 2;
        
        /* Complex expression with intermediate range checks */
        _Accum temp = shifted * 0.5k;
        _Accum final_val = temp + 0.25k;
        
        results[result_idx++] = final_val;
        
        /* Force range analysis with conditional */
        _Accum c = (seed > 100) ? 0.999999999k : -0.999999999k;
        _Accum d = c * c;  /* Square of near-max/min value */
        results[result_idx++] = d;
    }
    
    /* Test 2: Unsigned fractional types */
    volatile unsigned short us = useed;
    for (int i = 0; i < 3; i++) {
        /* Values very close to 1.0 */
        unsigned _Fract uf = (unsigned _Fract)(us + i) / 256ur;
        if (uf > 0.9ur) {
            uf = 0.9999999ur;  /* Max representable */
        }
        
        /* Operation that could wrap */
        unsigned _Fract uf2 = uf + 0.0000001ur;
        
        /* Convert to _Accum for storage */
        results[result_idx++] = (_Accum)uf2;
        
        /* Multiplication near overflow */
        unsigned _Fract uf3 = uf * 0.9999999ur;
        results[result_idx++] = (_Accum)uf3;
    }
    
    /* Test 3: Signed fractional types with negative values */
    volatile short vs = seed;
    for (int i = 0; i < 3; i++) {
        _Fract sf = (_Fract)(vs + i) / 128r;
        
        /* Push to extremes */
        if (sf > 0.5r) sf = 0.9999999r;
        if (sf < -0.5r) sf = -0.9999999r;
        
        /* Complex expression mixing operations */
        _Fract sf2 = sf * sf;  /* Square - always positive but could saturate */
        _Fract sf3 = sf2 - sf; /* Could overflow in subtraction */
        
        /* Left shift on fractional (scaled integer operation) */
        long _Fract lf = (long _Fract)sf3;
        lf = lf << 1;  /* Potential overflow */
        
        results[result_idx++] = (_Accum)lf;
    }
    
    /* Test 4: Mixed integer and fixed-point with promotions */
    volatile long long vll = lseed;
    for (int i = 0; i < 2; i++) {
        /* Integer to fixed-point conversion with scaling */
        int int_val = (vll + i * 1000) % 10000;
        _Accum scaled = (_Accum)int_val * 0.001k;  /* Could overflow if int_val large */
        
        /* Fixed-point to integer with overflow potential */
        _Accum large_fixed = 999.999999k;
        int converted = (int)(large_fixed * 1000k);  /* Explicit conversion */
        
        /* Use in conditional expression */
        _Accum mixed = (converted > 500000) ? 0.999999999k : scaled;
        
        /* Chain of operations */
        _Accum chain = mixed;
        chain = chain * 1.5k;
        chain = chain << 1;  /* Left shift */
        chain = chain / 0.75k;
        
        results[result_idx++] = chain;
    }
    
    /* Test 5: Saturation-like checks (emulating overflow detection) */
    volatile int counter = seed % 8;
    for (int i = 0; i < 2; i++) {
        _Accum x = 0.5k + (_Accum)counter * 0.1k;
        _Accum y = 0.8k;
        
        /* Multiplication that might need range checking */
        _Accum z = x * y;
        
        /* Check if we're at maximum (like the uncovered code's comparison) */
        _Accum max_val = 0.999999999k;
        _Accum min_val = -0.999999999k;
        
        /* Conditional based on range comparison */
        _Accum clamped;
        if (z > max_val) {
            clamped = max_val;
        } else if (z < min_val) {
            clamped = min_val;
        } else {
            clamped = z;
        }
        
        /* Additional shift that could overflow */
        clamped = clamped << 1;
        
        results[result_idx++] = clamped;
        
        counter++;
    }
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 16) result_idx = 16;
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    
    /* Return hash of results to make execution meaningful */
    return compute_hash(results, result_idx) & 0xFF;
}
