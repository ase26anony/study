/* fixed-point-coverage.c */
#include <stdio.h>

/* Use C23 fixed-point types with GCC extensions */
#if __STDC_VERSION__ >= 202311L
#define USE_C23_TYPES 1
#else
#define USE_C23_TYPES 0
#endif

/* Force compile-time evaluation */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
    unsigned long _Accum ula;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.99999hr, -0.99999r, 0.99999ur, 255.99999k, -32767.99999lk, 4294967295.99999ulk},
    {0.5hr, -0.5r, 0.5ur, 127.99999k, -16383.99999lk, 2147483647.99999ulk},
    {0.0hr, 0.0r, 0.0ur, 0.0k, 0.0lk, 0.0ulk}
};

/* Function to trigger range calculations through conversions */
static int convert_and_check(unsigned _Sat _Fract val) {
    /* This conversion should trigger range checking */
    int int_val = (int)val;
    float float_val = (float)val;
    double double_val = (double)val;
    
    /* Use in conditional to prevent optimization */
    return (int_val > 0) ? (int)(float_val * 100) : (int)(double_val * 100);
}

int main(void) {
    /* Declare variables with extreme values */
    unsigned short _Fract max_ushort_fract = 0.99999hr;
    signed _Fract min_signed_fract = -0.99999r;
    unsigned _Sat _Fract sat_fract = 0.99999ur;
    signed _Sat _Accum sat_accum = 255.99999k;
    long _Accum long_accum = -32767.99999lk;
    unsigned long _Accum ulong_accum = 4294967295.99999ulk;
    
    /* Compile-time constant expressions that should trigger range checks */
#if USE_C23_TYPES
    static const signed _Sat _Fract compile_time_sat = EVAL_CONST(0.99999r + 0.00001r);
    static const unsigned _Accum compile_time_accum = EVAL_CONST(255.99999k * 2.0k);
#else
    /* GCC extension syntax */
    static const _Sat _Fract compile_time_sat = EVAL_CONST(0.99999r + 0.00001r);
    static const unsigned _Accum compile_time_accum = EVAL_CONST(255.99999k * 2.0k);
#endif
    
    /* Force constant folding with ternary operator */
    const signed _Fract folded_const = 
        (compile_time_sat > 0.5r) ? 0.99999r : -0.99999r;
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(folded_const * 2.0r + 1.0r);
    if (idx < 0) idx = 0;
    if (idx > 2) idx = 2;
    
    struct FixedPointData data = init_data[idx];
    
    /* Loop with fixed iterations for unrolling */
    volatile int results[6] = {0}; /* volatile to prevent elimination */
    
    for (int i = 0; i < 3; i++) {
        /* Operations that may overflow/underflow */
        sat_fract = sat_fract + 0.5ur;  /* Should saturate */
        sat_accum = sat_accum * 1.5k;   /* May overflow */
        long_accum = long_accum - 16384.0lk; /* Underflow check */
        ulong_accum = ulong_accum / 0.5ulk;  /* Overflow check */
        
        /* Mix with integer arithmetic */
        int temp = (int)sat_fract + (int)(long_accum / 1000.0lk);
        
        /* Conditional based on fixed-point comparison */
        if (sat_fract > 0.75ur) {
            max_ushort_fract = max_ushort_fract * 0.9hr;
        } else {
            min_signed_fract = min_signed_fract + 0.1r;
        }
        
        /* Store results to prevent dead code elimination */
        results[0] = convert_and_check(sat_fract);
        results[1] = (int)(sat_accum);
        results[2] = (int)(long_accum / 100.0lk);
        results[3] = (int)(ulong_accum / 1000000.0ulk);
        results[4] = (int)(max_ushort_fract * 1000);
        results[5] = (int)(min_signed_fract * -1000);
    }
    
    /* Use the compile-time constants */
    results[0] += (int)(compile_time_sat * 100);
    results[1] += (int)(compile_time_accum / 10);
    
    /* Print to prevent optimization */
    printf("Results: ");
    for (int i = 0; i < 6; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    /* Use struct members */
    printf("Struct: usf=%d, sf=%d, usatf=%d, sata=%d, la=%d, ula=%d\n",
           (int)(data.usf * 1000),
           (int)(data.sf * 1000),
           (int)(data.usatf * 1000),
           (int)(data.sata),
           (int)(data.la / 100),
           (int)(data.ula / 1000000));
    
    return 0;
}
