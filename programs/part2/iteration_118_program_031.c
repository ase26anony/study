/* Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-test.c -o fixed-test */

#include <stdio.h>

/* Fixed-point type declarations covering various modes */
typedef short _Fract sf;
typedef _Fract f;
typedef long _Fract lf;
typedef short _Accum sa;
typedef _Accum a;
typedef long _Accum la;
typedef unsigned short _Fract usf;
typedef unsigned _Fract uf;
typedef unsigned long _Fract ulf;
typedef unsigned short _Accum usa;
typedef unsigned _Accum ua;
typedef unsigned long _Accum ula;

/* Saturated versions */
typedef _Sat short _Fract ssf;
typedef _Sat _Fract sfx;
typedef _Sat long _Fract slf;
typedef _Sat short _Accum ssa;
typedef _Sat _Accum sax;
typedef _Sat long _Accum sla;
typedef _Sat unsigned short _Fract susf;
typedef _Sat unsigned _Fract suf;
typedef _Sat unsigned long _Fract sulf;
typedef _Sat unsigned short _Accum susa;
typedef _Sat unsigned _Accum sua;
typedef _Sat unsigned long _Accum sula;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointStruct {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    sua sat_unsigned_accum;
    la long_accum;
};

/* Array initialized with fixed-point constants */
static const struct FixedPointStruct fp_array[] = {
    { .short_fract = 0.5hr, .accum = 0.5k, .unsigned_fract = 0.999999r,
      .sat_unsigned_accum = 0.999999999uk, .long_accum = 0.5lk },
    { .short_fract = -0.5hr, .accum = -0.5k, .unsigned_fract = 0.0r,
      .sat_unsigned_accum = 0.0uk, .long_accum = -0.5lk },
    { .short_fract = 0.999999hr, .accum = 0.999999999k,
      .unsigned_fract = 0.999999r, .sat_unsigned_accum = 0.999999999uk,
      .long_accum = 0.999999999999999lk }
};

/* Compile-time conditional using __builtin_constant_p */
#define CHECK_CONSTANT(expr) \
    (__builtin_constant_p(expr) ? (expr) : 0)

/* Force compile-time evaluation with ternary operator */
static const sf compile_time_fract = 
    (CHECK_CONSTANT(0.75hr * 0.75hr) > 0.5hr) ? 0.999999hr : 0.0hr;

/* Test function that performs operations likely to trigger range checks */
static void test_fixed_point_operations(void) {
    /* Declare and initialize variables at representable limits */
    const usf max_usf = 0.999999hr;
    const uf max_uf = 0.999999r;
    const ulf max_ulf = 0.999999999999999lr;
    const usa max_usa = 0.999999999hk;
    const ua max_ua = 0.999999999k;
    const ula max_ula = 0.999999999999999lk;
    
    const sf min_sf = -0.999999hr;
    const f min_f = -0.999999r;
    const lf min_lf = -0.999999999999999lr;
    const sa min_sa = -0.999999999hk;
    const a min_a = -0.999999999k;
    const la min_la = -0.999999999999999lk;
    
    /* Saturated types with values that will overflow/underflow */
    ssf sat_sf = 0.999999hr;
    sfx sat_f = 0.999999r;
    sla sat_la = 0.999999999999999lk;
    susf sat_usf = 0.999999hr;
    sua sat_ua = 0.999999999k;
    
    /* Operations that may overflow and trigger range checks */
    volatile sf result_sf;  /* volatile to prevent elimination */
    volatile a result_a;
    volatile la result_la;
    volatile sua result_sua;
    
    /* Test 1: Multiplication near limits (may overflow) */
    result_sf = (sf)(max_usf * max_usf);  /* Cast from unsigned to signed */
    result_a = (a)(max_ua * max_ua);
    
    /* Test 2: Addition that may saturate */
    sat_sf = sat_sf + 0.5hr;  /* Should saturate to max */
    sat_f = sat_f + 0.5r;
    sat_la = sat_la + 0.5lk;
    
    /* Test 3: Subtraction from minimum values */
    sat_usf = sat_usf - 1.0hr;  /* Should saturate to 0 */
    sat_ua = sat_ua - 1.0k;
    
    /* Test 4: Complex expression with mixed types */
    result_la = (la)((max_ula / 2.0lk) * (min_la / 2.0lk));
    
    /* Test 5: Conversion between fixed-point and integer */
    int int_from_fract = (int)(max_uf * 100);
    float float_from_accum = (float)(min_a);
    
    /* Test 6: Array indexing using fixed-point calculations */
    int index = (int)(max_uf * (sizeof(fp_array)/sizeof(fp_array[0]) - 1));
    if (index >= 0 && index < (int)(sizeof(fp_array)/sizeof(fp_array[0]))) {
        result_sua = fp_array[index].sat_unsigned_accum;
    }
    
    /* Test 7: Loop with fixed-point operations (may be unrolled) */
    for (int i = 0; i < 3; i++) {
        /* Conditional based on fixed-point comparison */
        if (fp_array[i].accum > 0.0k) {
            result_a = fp_array[i].accum * 1.5k;  /* May overflow */
        } else {
            result_a = fp_array[i].accum * 0.5k;
        }
        
        /* Another conditional that uses the uncovered comparison logic */
        if (fp_array[i].long_accum < -0.999lk) {
            result_la = min_la;
        } else if (fp_array[i].long_accum > 0.999lk) {
            result_la = max_ula;  /* Mix signed and unsigned */
        }
    }
    
    /* Test 8: Shift-like operations using multiplication by powers of 2 */
    result_a = min_a * 4.0k;  /* Effectively left shift, may overflow */
    result_la = max_ula * 0.25lk;  /* Effectively right shift */
    
    /* Test 9: Use compile-time constant in expression */
    result_sf = compile_time_fract * 2.0hr;  /* May overflow at compile time */
    
    /* Test 10: Explicit overflow attempt with saturating types */
    sulf max_sulf = 0.999999999999999lr;
    sulf overflow_test = max_sulf + max_sulf;  /* Should saturate */
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %f %lld\n", int_from_fract, float_from_accum, 
           (long long)result_la);
}

/* Main function with compile-time evaluations */
int main(void) {
    /* Compile-time evaluation using #if */
#if (0.999999r > 0.5r)
    const uf compile_time_uf = 0.999999r;
#else
    const uf compile_time_uf = 0.0r;
#endif
    
    /* Another compile-time check */
#if ((int)(0.999999999k * 1000) > 500)
    const a compile_time_a = 0.999999999k;
#else
    const a compile_time_a = 0.0k;
#endif
    
    /* Use these in runtime calculations */
    volatile uf runtime_uf = compile_time_uf * 1.5r;  /* May overflow */
    volatile a runtime_a = compile_time_a + 0.5k;
    
    /* Perform the main test */
    test_fixed_point_operations();
    
    /* Additional test with extreme values */
    {
        /* These initializations should trigger range boundary checks */
        const sf extreme_sf = -0.999999hr;
        const uf extreme_uf = 0.999999r;
        const la extreme_la = -0.999999999999999lk;
        const ula extreme_ula = 0.999999999999999lk;
        
        /* Operations that directly test the uncovered comparison logic */
        volatile sf test1 = extreme_sf * extreme_sf;  /* Square of negative near -1 */
        volatile uf test2 = extreme_uf / 0.000001r;  /* Division by very small number */
        volatile la test3 = extreme_la - 0.000000000000001lk;  /* Subtract epsilon */
        volatile ula test4 = extreme_ula + 0.000000000000001lk;  /* Add epsilon */
        
        /* Mixed-type operation that requires careful range checking */
        volatile a test5 = (a)(extreme_uf) + (a)(extreme_sf);
    }
    
    return 0;
}
