/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure with fixed-point */
struct fixed_bitfield {
    unsigned int mantissa : 10;
    signed int exponent : 6;
    sfract_t frac_part;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bits(sfract_t input, int shift) {
    struct fixed_bitfield result;
    volatile sfract_t temp = input;
    
    /* Force range calculations through shifts */
    result.mantissa = ((unsigned int)(temp * 1024.0hr)) >> shift;
    result.exponent = shift;
    
    /* Create value that may exceed range */
    sfract_t shifted = temp;
    for (int i = 0; i < shift; i++) {
        shifted = shifted * 0.5hr;  /* Right shift simulation */
    }
    
    /* Explicit comparison triggering sgt/ugt */
    if (shifted > 0.25hr) {
        result.frac_part = shifted;
    } else if (shifted < -0.25hr) {
        result.frac_part = -shifted;
    } else {
        result.frac_part = 0.0hr;
    }
    
    return result;
}

/* Mixed integer/fixed-point operations */
static accum_t mixed_operations(accum_t base, int iterations) {
    volatile accum_t acc = base;
    volatile int int_bound = iterations;
    
    /* Loop forcing range widening */
    for (int i = 0; i < iterations; i++) {
        /* Multi-step computation with overflow potential */
        accum_t step = acc * 0.1lk;
        
        /* Conditional with explicit range comparison */
        if (step > 0.5lk && step < 2.0lk) {
            acc = acc + step;
        } else if (step <= -0.5lk || step >= 2.0lk) {
            acc = acc - 0.05lk;
        }
        
        /* Mixed-type comparison */
        if ((long)acc > int_bound) {
            acc = acc * 0.9lk;
        }
        
        /* Ternary operator with fixed-point */
        acc = (i % 3 == 0) ? acc * 1.1lk : acc * 0.9lk;
    }
    
    return acc;
}

/* Saturated arithmetic test */
static sat_fract_t test_saturation(fract_t a, fract_t b) {
    sat_fract_t sat_a = a;
    sat_fract_t sat_b = b;
    
    /* Operations that may saturate */
    sat_fract_t sum = sat_a + sat_b;
    sat_fract_t prod = sat_a * sat_b;
    
    /* Range comparisons on saturated values */
    volatile fract_t threshold = 0.75r;
    if (sum > threshold || prod > threshold) {
        return sum;
    } else if (sum < -threshold && prod < -threshold) {
        return prod;
    }
    
    return (sum + prod) * 0.5r;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize fixed-point variables spanning different ranges */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.25hr;
    fract_t f1 = 0.7r;
    fract_t f2 = -0.3r;
    accum_t acc1 = 1.5lk;
    accum_t acc2 = -2.7lk;
    ufract_t uf1 = 0.8ur;
    
    /* Test 1: Bit-field and shift operations */
    printf("Test 1: Bit-field operations\n");
    for (int i = 1; i <= 4; i++) {
        struct fixed_bitfield bf = process_fixed_bits(sf1, i);
        checksum += bf.mantissa + bf.exponent;
        sf1 = sf1 * 0.8hr;
    }
    
    /* Test 2: Loop-based range widening */
    printf("Test 2: Range widening loops\n");
    volatile int loop_bound = 10;
    for (int i = 0; i < loop_bound; i++) {
        acc1 = mixed_operations(acc1, i + 1);
        acc2 = mixed_operations(acc2, i + 2);
        
        /* Explicit comparison with computed bounds */
        accum_t max_bound = acc1 > acc2 ? acc1 : acc2;
        accum_t min_bound = acc1 < acc2 ? acc1 : acc2;
        
        if (max_bound > 5.0lk || min_bound < -5.0lk) {
            acc1 = acc1 * 0.5lk;
            acc2 = acc2 * 0.5lk;
        }
        
        /* Mixed integer/fixed-point context */
        if ((int)(acc1 * 1000lk) > 1500 || (int)(acc2 * 1000lk) < -1500) {
            checksum += i;
        }
    }
    
    /* Test 3: Saturation and overflow checks */
    printf("Test 3: Saturation tests\n");
    for (int i = 0; i < 5; i++) {
        sat_fract_t result = test_saturation(f1, f2);
        checksum += (int)(result * 1000r);
        
        /* Update values to trigger different paths */
        f1 = f1 * 1.2r;
        f2 = f2 * 1.1r;
        
        /* Force overflow checking */
        if (f1 > 0.9r || f1 < -0.9r) {
            f1 = 0.1r;
        }
        if (f2 > 0.9r || f2 < -0.9r) {
            f2 = -0.1r;
        }
    }
    
    /* Test 4: Complex conditional expressions */
    printf("Test 4: Complex conditionals\n");
    volatile fract_t v1 = 0.6r;
    volatile fract_t v2 = -0.4r;
    
    for (int i = 0; i < 8; i++) {
        /* Nested conditionals with mixed operations */
        fract_t test_val = v1 * v2 + (fract_t)i * 0.1r;
        
        if ((test_val > 0.0r && v1 > 0.3r) || 
            (test_val < 0.0r && v2 < -0.3r)) {
            v1 = v1 * 0.9r;
            v2 = v2 * 1.1r;
            checksum += 1;
        } else if (test_val == 0.0r || 
                  (v1 > 0.8r && v2 < -0.8r)) {
            v1 = 0.5r;
            v2 = -0.5r;
            checksum += 2;
        }
        
        /* Shift-like operation using multiplication */
        ufract_t shifted = uf1;
        for (int j = 0; j < 3; j++) {
            shifted = shifted * 0.25ur;  /* Simulate >> 2 */
        }
        
        if (shifted > 0.1ur || shifted < 0.01ur) {
            uf1 = uf1 * 0.8ur;
            checksum += 3;
        }
    }
    
    /* Final checksum computation */
    checksum += (int)(acc1 * 100lk) + (int)(acc2 * 100lk);
    checksum += (int)(f1 * 1000r) + (int)(f2 * 1000r);
    checksum += (int)(uf1 * 1000ur);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
