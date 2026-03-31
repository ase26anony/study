/* test_fixed_range.c - Target fixed-value.cc lines 264-277 */
#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 100;
static volatile int vi4 = -100;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline)) void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) sink = cp[i];
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[8];
    unsigned _Fract uf_results[8];
    
    /* Initialize with edge-case values */
    _Accum a_max = 0.999999999k;      /* Near max for _Accum */
    _Accum a_min = -0.999999999k;     /* Near min for _Accum */
    unsigned _Fract uf_max = 0.9999999ur; /* Near max for unsigned _Fract */
    _Fract f_max = 0.9999999r;        /* Near max for signed _Fract */
    _Fract f_min = -0.9999999r;       /* Near min for signed _Fract */
    
    /* Long accum types for wider range */
    long _Accum la_max = 0.99999999999999999lk;
    long _Accum la_min = -0.99999999999999999lk;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Use volatile to prevent static analysis */
        int idx = vi1 + i;
        
        /* 1. Multiplication near overflow boundaries */
        _Accum temp1;
        if (idx & 1) {
            /* This multiplication could overflow */
            temp1 = a_max * (_Accum)(0.9k + (idx * 0.0001k));
        } else {
            temp1 = a_min * (_Accum)(-0.9k - (idx * 0.0001k));
        }
        
        /* 2. Left shift operations (converted to multiplication) */
        /* For fixed-point, left shift increases value */
        _Accum temp2 = temp1;
        for (int shift = 0; shift < (idx % 4); shift++) {
            /* Shift-left by 1 is multiply by 2 */
            temp2 = temp2 * 2.0k;
        }
        
        /* 3. Complex expression mixing types */
        unsigned _Fract uf_temp = uf_max;
        if (idx > 4) {
            /* Operations that could wrap unsigned fract */
            uf_temp = uf_temp + (_Fract)(0.0000001r * idx);
        }
        
        /* 4. Cast between integer and fixed-point with scaling */
        int int_val = vi2 + idx * vi3;
        _Accum from_int = (_Accum)int_val * 0.125k;  /* Division by 8 */
        
        /* 5. Conditional expression with fixed-point */
        _Accum cond_result = (int_val > 0) ? 
            (from_int * a_max) : (from_int * a_min);
        
        /* 6. Long accum operations - wider range checks */
        long _Accum la_temp = la_max;
        if (idx % 3 == 0) {
            la_temp = la_temp * 0.999999999lk;
        } else if (idx % 3 == 1) {
            la_temp = la_min * 0.999999999lk;
        }
        
        /* 7. Accumulate results with saturation-like behavior */
        results[i] = temp2 + cond_result * 0.5k;
        
        /* 8. Unsigned fract with wrap-around potential */
        uf_results[i] = uf_temp;
        for (int j = 0; j < (i % 3); j++) {
            uf_results[i] = uf_results[i] * uf_max;
        }
        
        /* 9. Explicit overflow check simulation */
        /* This should trigger range comparison logic */
        _Accum check_val = results[i];
        _Accum max_check = 0.999999999k;
        _Accum min_check = -0.999999999k;
        
        /* Conditional that requires range analysis */
        if (check_val > max_check) {
            results[i] = max_check;
        } else if (check_val < min_check) {
            results[i] = min_check;
        }
        
        /* 10. Mixed-width operations */
        short _Fract sf = 0.9999hr;
        _Fract widened = (_Fract)sf * f_max;
        results[i] = results[i] + (_Accum)widened * 0.1k;
    }
    
    /* 11. Final complex expression that depends on all previous */
    _Accum final_acc = 0.0k;
    for (int i = 0; i < 8; i++) {
        /* Chain multiplications that could overflow */
        final_acc = final_acc + results[i];
        if (i > 0) {
            final_acc = final_acc * (_Accum)(0.99k + (i * 0.001k));
        }
    }
    
    /* 12. Array operations with fixed-point */
    _Fract f_array[4];
    for (int i = 0; i < 4; i++) {
        f_array[i] = f_max * (_Fract)(i * 0.25r);
        if (vi4 + i < 0) {
            f_array[i] = f_min * (_Fract)(-i * 0.25r);
        }
    }
    
    /* Prevent optimization */
    consume(results, sizeof(results));
    consume(uf_results, sizeof(uf_results));
    consume(f_array, sizeof(f_array));
    consume(&final_acc, sizeof(final_acc));
    
    /* Return hash of results */
    int hash = 0;
    unsigned char *p = (unsigned char *)results;
    for (unsigned int i = 0; i < sizeof(results); i++) {
        hash = (hash * 31) + p[i];
    }
    
    return hash & 0xFF;
}
