/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0x7FFF;
static volatile int vi4 = 0x8000;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_values(const void *ptr, int size) {
    volatile char sink;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
}

/* Helper to create complex data-dependent expressions */
__attribute__((noinline, noipa))
unsigned int hash_buffer(const void *ptr, int size) {
    const unsigned char *p = (const unsigned char *)ptr;
    unsigned int hash = 0x811c9dc5;
    for (int i = 0; i < size; i++) {
        hash ^= p[i];
        hash *= 0x01000193;
    }
    return hash;
}

int main(void) {
    /* Array to accumulate results and prevent optimization */
    _Accum results[16];
    unsigned _Fract uf_results[16];
    long _Accum la_results[8];
    
    /* Initialize with boundary values */
    _Accum max_accum = 0.9999999999999999k;  /* Near max for _Accum */
    _Accum min_accum = -1.0k;                /* Min for _Accum */
    unsigned _Fract max_ufract = 0.9999999999999999ur; /* Near max for unsigned _Fract */
    long _Accum max_laccum = 0.9999999999999999999999999999999999lk; /* Near max for long _Accum */
    
    /* Use volatile seeds to prevent constant propagation */
    volatile int seed1 = vi1;
    volatile int seed2 = vi2;
    volatile int seed3 = vi3;
    volatile int seed4 = vi4;
    
    int result_idx = 0;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Create data-dependent fixed-point values */
        int base = seed1 + i * seed2;
        
        /* Test 1: Signed _Accum multiplication near overflow boundary */
        _Accum a1 = (_Accum)base * 0.0000152587890625k; /* 1/65536 */
        _Accum a2 = max_accum - a1;
        
        /* This multiplication should trigger overflow range check */
        _Accum product = a2 * a2;
        results[result_idx++] = product;
        
        /* Test 2: Left shift of fixed-point (simulated via multiplication) */
        /* Shifting left by 1 is equivalent to *2, can overflow */
        _Accum shifted = a2 * 2.0k;
        results[result_idx++] = shifted;
        
        /* Test 3: Unsigned _Fract operations near 1.0 */
        unsigned _Fract uf1 = max_ufract - (unsigned _Fract)(i * 0.0000001ur);
        unsigned _Fract uf2 = (unsigned _Fract)(seed3 & 0xFF) * 0.00392156862745098ur; /* 1/255 */
        
        /* Addition that could overflow/wrap */
        unsigned _Fract uf_sum = uf1 + uf2;
        uf_results[i] = uf_sum;
        
        /* Test 4: Long _Accum with extreme values */
        long _Accum la1 = max_laccum;
        long _Accum la2 = (long _Accum)(seed4 & 0x7FFF) * 0.00003051850947599719lk;
        
        /* Multiplication that should trigger the specific range comparison */
        long _Accum la_product = la1 * la2;
        la_results[i] = la_product;
        
        /* Test 5: Mixed-type expressions with conditional operator */
        /* This creates complex value ranges for analysis */
        _Accum cond_val = (i & 1) ? max_accum : min_accum;
        cond_val = cond_val * (_Accum)((seed1 + i) & 0x7F) * 0.0078125k; /* 1/128 */
        results[result_idx++] = cond_val;
        
        /* Test 6: Explicit overflow check simulation */
        /* Force compiler to consider overflow possibility */
        _Accum test_val = max_accum;
        for (int j = 0; j < (seed2 & 3); j++) {
            test_val = test_val * 1.5k;  /* Definitely overflows */
        }
        results[result_idx++] = test_val;
    }
    
    /* Additional boundary case tests outside loop */
    
    /* Test 7: Direct maximum value operations */
    _Accum direct_max = max_accum;
    _Accum direct_square = direct_max * direct_max;  /* Should overflow */
    results[result_idx++] = direct_square;
    
    /* Test 8: Minimum value operations */
    _Accum direct_min = min_accum;
    _Accum min_square = direct_min * direct_min;  /* 1.0, should be representable */
    results[result_idx++] = min_square;
    
    /* Test 9: Near-boundary left shift via multiplication */
    /* Equivalent to << 2 */
    _Accum shift_test = max_accum * 4.0k;  /* Definitely overflows */
    results[result_idx++] = shift_test;
    
    /* Test 10: Integer to fixed-point conversion with overflow */
    int large_int = seed3 * 2;
    _Accum from_int = (_Accum)large_int;  /* May overflow depending on seed3 */
    results[result_idx++] = from_int;
    
    /* Ensure all results are used */
    consume_values(results, sizeof(results));
    consume_values(uf_results, sizeof(uf_results));
    consume_values(la_results, sizeof(la_results));
    
    /* Return hash of results to prevent optimization */
    unsigned int hash = 0;
    hash ^= hash_buffer(results, sizeof(results));
    hash ^= hash_buffer(uf_results, sizeof(uf_results));
    hash ^= hash_buffer(la_results, sizeof(la_results));
    
    return (int)(hash & 0x7FFFFFFF);
}
