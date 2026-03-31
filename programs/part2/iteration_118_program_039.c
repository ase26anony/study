/* Test program for fixed-point range calculations in GCC */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original -o fixed_test fixed_test.c */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Accum ua;
    signed long _Accum sla;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum ssata;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.99999hr, -0.99999r, 255.99999uk, -32767.99999lk, 0.99999ur, 127.99999k},
    {0.5hr, -0.5r, 128.5uk, -16384.5lk, 0.5ur, 64.5k},
    {0.0hr, 0.0r, 0.0uk, 0.0lk, 0.0ur, 0.0k}
};

/* Function to test boundary conditions */
static inline signed long _Accum test_boundary_ops(signed long _Accum a, signed long _Accum b) {
    /* These operations may trigger range checks */
    signed long _Accum result = a * b;
    
    /* Force saturation check */
    if (result > 16384.0lk) {
        return 16384.0lk;
    } else if (result < -16384.0lk) {
        return -16384.0lk;
    }
    
    return result;
}

int main(void) {
    /* Declare fixed-point variables with extreme values */
    const unsigned short _Fract max_ushort_fract = 0.99999hr;
    const signed _Fract min_signed_fract = -0.99999r;
    const unsigned _Accum max_unsigned_accum = 255.99999uk;
    const signed long _Accum min_signed_long_accum = -32767.99999lk;
    
    /* Saturation types with boundary values */
    unsigned _Sat _Fract sat_uf = 0.99999ur;
    signed _Sat _Accum sat_sa = 127.99999k;
    
    /* Test compile-time constant folding */
    #if __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    /* This block only compiles if certain builtins are available */
    const signed _Fract const_folded = EVAL_CONST(0.75r * 0.75r);
    volatile signed _Fract keep_alive = const_folded;
    #endif
    
    /* Complex expression that should trigger range checks */
    const signed long _Accum complex_expr = EVAL_CONST(
        (min_signed_long_accum * 0.5lk) + 
        (max_unsigned_accum * 2.0k)
    );
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(max_ushort_fract * 3);
    if (idx >= 0 && idx < 3) {
        volatile struct FixedPointData data = init_data[idx];
        (void)data; /* Prevent unused warning */
    }
    
    /* Loop with fixed-point operations */
    signed _Accum accum = 0.0k;
    for (int i = 0; i < 4; i++) {
        /* Operations that may overflow */
        accum = accum + 64.0k;
        
        /* Conditional based on fixed-point comparison */
        if (accum > 127.0k) {
            accum = 127.0k;
        }
        
        /* Mix with integer arithmetic */
        int int_val = (int)(accum * 2);
        (void)int_val;
        
        /* Test saturation */
        sat_uf = sat_uf + 0.5ur;  /* Should saturate to 1.0ur */
        sat_sa = sat_sa * 2.0k;   /* Should saturate to max */
    }
    
    /* Test boundary function */
    signed long _Accum boundary_test = test_boundary_ops(
        min_signed_long_accum,
        0.99999lk
    );
    
    /* Conversions that may trigger range checks */
    float as_float = (float)max_unsigned_accum;
    int as_int = (int)min_signed_long_accum;
    unsigned _Accum back_from_float = (unsigned _Accum)as_float;
    
    /* Prevent dead code elimination */
    volatile float v_float = as_float;
    volatile int v_int = as_int;
    volatile unsigned _Accum v_accum = back_from_float;
    volatile signed _Accum v_saccum = accum;
    volatile signed long _Accum v_boundary = boundary_test;
    volatile unsigned _Sat _Fract v_satuf = sat_uf;
    volatile signed _Sat _Accum v_satsa = sat_sa;
    
    /* Use all volatile variables to prevent optimization */
    printf("Results: %f %d\n", v_float, v_int);
    
    return 0;
}

/* Additional compile-time tests using ternary operators */
#ifdef __cplusplus
constexpr unsigned short _Fract cpp_const_fract = true ? 0.99999hr : 0.0hr;
#else
static const unsigned short _Fract c_const_fract = 1 ? 0.99999hr : 0.0hr;
#endif

/* Test extreme boundary cases in global initializers */
static const struct {
    signed long _Accum near_min;
    signed long _Accum near_max;
    unsigned _Sat _Fract saturated_sum;
} boundaries = {
    .near_min = -32767.99999lk + 0.00001lk,
    .near_max = 32767.99999lk - 0.00001lk,
    .saturated_sum = 0.99999ur + 0.00001ur  /* Should saturate to 1.0ur */
};
