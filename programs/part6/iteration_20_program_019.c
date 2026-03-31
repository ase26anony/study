/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis in GCC's fixed-value.cc
 * Specifically targets lines 264-277 that check overflow against max/min bounds
 */

/* Prevent constant folding of critical values */
volatile int vi = 0;
volatile unsigned int vu = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Helper to get near-maximum values */
#define NEAR_MAX_SFRACT 0.9999999hr
#define NEAR_MAX_FRACT 0.9999999r
#define NEAR_MAX_LFRACT 0.9999999999999999lr
#define NEAR_MAX_USFRACT 0.9999999uhr
#define NEAR_MAX_UFRACT 0.9999999ur
#define NEAR_MAX_ULFRACT 0.9999999999999999ulr
#define NEAR_MAX_ACCUM 0.999999999k
#define NEAR_MAX_LACCUM 0.9999999999999999lk
#define NEAR_MAX_UACCUM 0.999999999uk
#define NEAR_MAX_ULACCUM 0.9999999999999999ulk

int main(void) {
    /* Array to accumulate results - prevents optimization */
    _Accum results[32];
    int result_idx = 0;
    
    /* Initialize with volatile seeds to prevent constant propagation */
    volatile int seed1 = vi;
    volatile int seed2 = vu;
    
    /* Test 1: Signed accumulative types near maximum */
    for (int i = 0; i < 4; i++) {
        /* Use different near-max values based on iteration */
        _Accum a = NEAR_MAX_ACCUM - (_Accum)(seed1 % 100) * 0.0000001k;
        _Accum b = NEAR_MAX_ACCUM - (_Accum)(seed2 % 100) * 0.0000001k;
        
        /* Multiplication that could overflow the fixed-point range */
        _Accum prod = a * b;
        
        /* Left shift to potentially exceed maximum */
        _Accum shifted = prod << (1 + (i & 1));
        
        /* Complex expression with intermediate range calculation */
        _Accum temp = (a + b) / 2.0k;
        _Accum final_val = temp * shifted;
        
        results[result_idx++] = final_val;
        
        /* Update seeds to vary ranges */
        seed1 = seed1 * 1103515245 + 12345;
        seed2 = seed2 * 1664525 + 1013904223;
    }
    
    /* Test 2: Unsigned fractional types at boundaries */
    volatile unsigned short seed3 = vu;
    for (int i = 0; i < 4; i++) {
        unsigned _Fract uf = NEAR_MAX_UFRACT - (unsigned _Fract)(seed3 % 100) * 0.0000001ur;
        
        /* Operations that could wrap around */
        unsigned _Fract uf2 = uf + (unsigned _Fract)(0.0000001ur * (i + 1));
        unsigned _Fract uf3 = uf2 * uf;
        
        /* Cast to signed and back to trigger range analysis */
        _Fract sf = (_Fract)uf3;
        sf = sf * 0.9999999r;
        
        results[result_idx++] = (_Accum)sf;
        
        seed3 = seed3 * 1103515245 + 12345;
    }
    
    /* Test 3: Mixed signed types with negative values */
    volatile int seed4 = vi;
    for (int i = 0; i < 4; i++) {
        _Fract f1 = (seed4 & 1) ? 0.9999999r : -0.9999999r;
        _Fract f2 = (seed4 & 2) ? 0.5r : -0.75r;
        
        /* Multiplication near ±1.0 boundary */
        _Fract prod_f = f1 * f2;
        
        /* Left shift that requires range checking */
        long _Accum la = (long _Accum)prod_f;
        la = la << (2 + (seed4 & 3));
        
        /* Conditional expression with fixed-point */
        _Accum cond_result = (seed4 > 0) ? (_Accum)la : (_Accum)(-la);
        
        results[result_idx++] = cond_result;
        
        seed4 = seed4 * 1664525 + 1013904223;
    }
    
    /* Test 4: Explicit overflow checks using saturation logic */
    volatile long seed5 = vi;
    for (int i = 0; i < 4; i++) {
        long _Accum la1 = NEAR_MAX_LACCUM - (long _Accum)(seed5 % 1000) * 0.0000000000000001lk;
        long _Accum la2 = NEAR_MAX_LACCUM - (long _Accum)((seed5 >> 8) % 1000) * 0.0000000000000001lk;
        
        /* This multiplication mathematically exceeds the fixed-point range */
        long _Accum big_prod = la1 * la2;
        
        /* Additional shift to ensure range analysis is needed */
        big_prod = big_prod << 1;
        
        /* Manual overflow check - forces compiler to consider ranges */
        int overflow = 0;
        if (la1 > 0.9lk && la2 > 0.9lk) {
            /* This condition should trigger the uncovered range comparison */
            long _Accum test = la1 * 1.1lk;
            if (test > NEAR_MAX_LACCUM) {
                overflow = 1;
            }
        }
        
        results[result_idx++] = (_Accum)(big_prod * (overflow ? 0.5k : 1.0k));
        
        seed5 = seed5 * 1103515245 + 12345;
    }
    
    /* Test 5: Integer promotions with fixed-point */
    volatile int seed6 = vi;
    for (int i = 0; i < 4; i++) {
        int int_val = seed6 % 256;
        
        /* Conversion from integer to fixed-point with multiplication */
        _Accum from_int = (_Accum)int_val * 0.1k;
        
        /* Chain of operations to build complex range */
        _Accum chain = from_int;
        for (int j = 0; j < 3; j++) {
            chain = chain * 0.9999999k;
            chain = chain << 1;
        }
        
        results[result_idx++] = chain;
        
        seed6 = seed6 * 1664525 + 1013904223;
    }
    
    /* Test 6: Array-based computations with varying ranges */
    volatile int seed7 = vi;
    _Fract farray[8];
    for (int i = 0; i < 8; i++) {
        farray[i] = (_Fract)((seed7 % 2000 - 1000) / 1000.0);
        seed7 = seed7 * 1103515245 + 12345;
    }
    
    for (int i = 0; i < 4; i++) {
        /* Access array with volatile-like pattern */
        _Fract elem1 = farray[(i * 2) % 8];
        _Fract elem2 = farray[(i * 2 + 1) % 8];
        
        /* Operation that might overflow depending on array contents */
        _Accum combined = (_Accum)elem1 * (_Accum)elem2 * 2.0k;
        
        results[result_idx++] = combined;
    }
    
    /* Ensure all results are used */
    consume(results, sizeof(results));
    
    /* Create a simple hash of results to return */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Convert fixed-point to integer for hashing */
        int val = (int)(results[i] * 1000);
        hash = hash * 31 + val;
    }
    
    return hash & 0xFF;
}
