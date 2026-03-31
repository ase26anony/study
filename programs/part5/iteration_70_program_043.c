#include <stdio.h>
#include <stdint.h>

/* Force aggressive optimization on specific functions */
__attribute__((optimize("O3")))
void test_short_fract_boundaries(void) {
    /* Test short _Fract near boundaries */
    volatile short _Fract sf_min = 0x8000;  /* Minimum: -1.0 */
    volatile short _Fract sf_max = 0x7FFF;  /* Maximum: ~0.999969 */
    volatile short _Fract sf_zero = 0x0000; /* Zero */
    
    /* Mixed precision operations to force range analysis */
    _Sat unsigned long _Accum ula = 0.5uk;
    _Sat signed long _Accum sla = -0.5k;
    
    /* Operations that approach boundaries */
    for (int i = 0; i < 10; i++) {
        /* Multiplication that could saturate */
        _Sat unsigned short _Fract usf = (_Sat unsigned short _Fract)(ula * i);
        
        /* Division near boundaries */
        _Sat signed short _Fract ssf = (_Sat signed short _Fract)(sla / (i + 1));
        
        /* Control flow dependent on fixed-point comparisons */
        if (usf > 0.8ur) {
            /* Force boundary comparison */
            volatile _Sat unsigned short _Fract temp = usf;
            if (temp == 0.999ur) {
                /* Boundary case */
                volatile int marker = 1;
            }
        }
        
        if (ssf < -0.8r) {
            /* Another boundary comparison */
            volatile _Sat signed short _Fract temp = ssf;
            if (temp == -1.0r) {
                /* Minimum boundary */
                volatile int marker = 2;
            }
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_operations(void) {
    /* Mixed precision types */
    volatile _Fract f1 = 0.25r;
    volatile _Accum a1 = 100.0k;
    volatile long _Accum la1 = -1000.0lk;
    
    /* Array of fixed-point values */
    _Sat signed short _Fract fract_array[4] = {0.5r, -0.5r, 0.75r, -0.75r};
    _Sat unsigned _Accum accum_array[4] = {0.1k, 0.2k, 0.3k, 0.4k};
    
    /* Operations that force range analysis */
    for (int i = 0; i < 4; i++) {
        /* Mixed precision multiplication */
        _Sat long _Accum result = (_Sat long _Accum)(fract_array[i] * accum_array[i]);
        
        /* Boundary comparisons */
        if (result > 0.5lk) {
            /* Force high-range comparison */
            volatile _Sat long _Accum temp = result;
            if (temp == 0.999lk) {
                volatile int marker = 3;
            }
        } else if (result < -0.5lk) {
            /* Force low-range comparison */
            volatile _Sat long _Accum temp = result;
            if (temp == -1.0lk) {
                volatile int marker = 4;
            }
        }
        
        /* Overflow checks with builtins */
        _Sat signed _Accum sa1 = 0.5k;
        _Sat signed _Accum sa2 = 0.6k;
        int overflow;
        
        /* These force overflow analysis */
        __builtin_mul_overflow(sa1, sa2, &overflow);
        __builtin_add_overflow(sa1, sa2, &overflow);
    }
}

/* Struct containing fixed-point values */
struct fixed_point_struct {
    _Sat unsigned short _Fract usf;
    _Sat signed _Accum sa;
    _Fract f;
    long _Accum la;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct fixed_point_struct fps[3];
    
    /* Initialize with boundary values */
    fps[0].usf = 0.999ur;  /* Near maximum */
    fps[0].sa = -0.999k;   /* Near minimum */
    fps[0].f = 0.0r;       /* Zero */
    fps[0].la = 0.5lk;     /* Middle */
    
    fps[1].usf = 0.0ur;    /* Minimum */
    fps[1].sa = 0.999k;    /* Near maximum */
    fps[1].f = -0.5r;      /* Negative */
    fps[1].la = -0.5lk;    /* Negative */
    
    /* Operations on struct members */
    for (int i = 0; i < 2; i++) {
        /* Mixed operations forcing range analysis */
        _Sat long _Accum mixed_result = 
            (_Sat long _Accum)(fps[i].usf * fps[i].sa) + 
            (_Sat long _Accum)(fps[i].f / fps[i].la);
        
        /* Complex boundary condition */
        if (mixed_result > 0.8lk || 
            (mixed_result == 0.8lk && fps[i].usf > 0.5ur)) {
            /* This should trigger the uncovered comparison logic */
            volatile _Sat long _Accum temp1 = mixed_result;
            volatile _Sat unsigned short _Fract temp2 = fps[i].usf;
            
            if (temp1 == 0.999lk && temp2 == 0.999ur) {
                volatile int marker = 5;
            }
        }
        
        if (mixed_result < -0.8lk ||
            (mixed_result == -0.8lk && fps[i].sa < -0.5k)) {
            /* Another boundary comparison */
            volatile _Sat long _Accum temp1 = mixed_result;
            volatile _Sat signed _Accum temp2 = fps[i].sa;
            
            if (temp1 == -1.0lk && temp2 == -1.0k) {
                volatile int marker = 6;
            }
        }
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundaries(void) {
    /* Extreme boundary values */
    volatile _Sat unsigned long _Accum ula_max = 0.999999999999999999ulk;
    volatile _Sat signed long _Accum sla_min = -1.0lk;
    volatile _Sat signed long _Accum sla_max = 0.999999999999999999lk;
    
    /* Operations that push to extremes */
    _Sat unsigned long _Accum ula_result = ula_max * 0.999ulk;
    _Sat signed long _Accum sla_result = sla_min / 0.5lk;
    
    /* Multiple comparisons to force all code paths */
    if (ula_result > 0.999ulk) {
        volatile int marker = 7;
    }
    
    if (sla_result < -1.0lk) {
        volatile int marker = 8;
    }
    
    if (ula_result == 0.99999999999999999ulk &&
        sla_result == -2.0lk) {
        volatile int marker = 9;
    }
    
    /* Test zero crossing */
    _Sat signed _Fract zero_test = 0.0r;
    for (int i = 0; i < 5; i++) {
        zero_test = zero_test + 0.2r - 0.2r;
        
        if (zero_test > 0.0r) {
            volatile int marker = 10;
        } else if (zero_test < 0.0r) {
            volatile int marker = 11;
        } else if (zero_test == 0.0r) {
            volatile int marker = 12;
        }
    }
}

int main(void) {
    volatile int result = 0;
    
    /* Execute all test functions */
    test_short_fract_boundaries();
    test_mixed_precision_operations();
    test_struct_operations();
    test_extreme_boundaries();
    
    /* Prevent dead code elimination */
    printf("Fixed-point tests completed. Result: %d\n", result);
    
    return 0;
}
