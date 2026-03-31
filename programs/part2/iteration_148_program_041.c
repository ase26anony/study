/* test_fixed.c - Program to exercise GCC's fixed-point range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions for clarity */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;

/* ========== SECTION 1: Complex Fixed-Point Operations ========== */

/* Function with inter-procedural range analysis */
static inline sat_fract_t add_with_saturation(sat_fract_t a, sat_fract_t b) {
    /* This addition can saturate - triggers range analysis */
    return a + b;
}

/* Function with multiplication and shift */
static inline sat_accum_t scale_and_shift(sat_accum_t x, sat_accum_t factor, int shift) {
    /* Complex expression requiring precise overflow analysis */
    return (x * factor) >> shift;
}

/* Function that forces range comparison */
static inline int is_in_range(fract_t val, fract_t min, fract_t max) {
    /* This comparison triggers the uncovered if condition logic */
    return (val >= min) && (val <= max);
}

/* ========== SECTION 2: Loop-Based Range Analysis ========== */

/* Loop with fixed-point induction variable */
void test_loop_range(int iterations) {
    sat_fract_t total = 0.0r;
    
    /* Loop where range of f affects total's range */
    for (fract_t f = 0.1r; f < 0.9r && iterations > 0; f += 0.1r, iterations--) {
        total = add_with_saturation(total, f);
        
        /* Conditional based on fixed-point comparison */
        if (total > 0.5r) {
            total = total * 0.8r;  /* Prevent saturation */
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile fract_t dummy = total;
    (void)dummy;
}

/* Array reduction with fixed-point values */
sat_accum_t array_reduction(const sat_accum_t* arr, int size) {
    sat_accum_t sum = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression that could overflow */
        sum = sum + (arr[i] * (sat_accum_t)(i * 0.1k));
        
        /* Ternary operator with fixed-point operands */
        sum = (sum > 10.0k) ? 10.0k : sum;
    }
    
    return sum;
}

/* ========== SECTION 3: Saturation Boundary Tests ========== */

/* Explicit saturation tests */
void test_saturation_boundaries(void) {
    /* Operations designed to hit saturation boundaries */
    sat_fract_t max_fract = 0.999999r;
    sat_fract_t min_fract = -0.999999r;
    
    /* These should trigger saturation logic */
    sat_fract_t s1 = max_fract + 0.1r;      /* Should saturate to max */
    sat_fract_t s2 = min_fract - 0.1r;      /* Should saturate to min */
    
    /* Multiplication near boundaries */
    sat_fract_t s3 = max_fract * 1.1r;      /* Should saturate */
    sat_fract_t s4 = max_fract * max_fract; /* Complex range analysis */
    
    /* Use results */
    volatile sat_fract_t vs1 = s1, vs2 = s2, vs3 = s3, vs4 = s4;
    (void)vs1; (void)vs2; (void)vs3; (void)vs4;
}

/* Mixed saturated/unsaturated operations */
void test_mixed_types(fract_t unsaturated, sat_fract_t saturated) {
    /* Assignment from unsaturated to saturated requires range check */
    sat_fract_t s1 = unsaturated;
    
    /* Mixed-type expression */
    sat_fract_t s2 = saturated + (sat_fract_t)unsaturated;
    
    /* Complex conditional */
    if (s1 > s2) {
        s1 = s1 * 0.5r;
    } else {
        s2 = s2 * 1.5r;  /* Could cause saturation */
    }
    
    volatile sat_fract_t v1 = s1, v2 = s2;
    (void)v1; (void)v2;
}

/* ========== SECTION 4: Compiler Intrinsics ========== */

/* Using builtins for overflow detection */
void test_builtin_overflow(void) {
    sat_fract_t a = 0.7r;
    sat_fract_t b = 0.6r;
    sat_fract_t result;
    int overflow;
    
    /* Builtin overflow check with fixed-point */
    overflow = __builtin_add_overflow(a, b, &result);
    
    /* Use the result */
    if (overflow) {
        result = 0.999999r;  /* Manual saturation */
    }
    
    volatile sat_fract_t vr = result;
    (void)vr;
}

/* Inline assembly to create hard-to-analyze value flows */
sat_accum_t asm_fixed_mul(sat_accum_t a, sat_accum_t b) {
    sat_accum_t result;
    
    /* Assembly with fixed-point constraints */
    __asm__ volatile (
        "fmul %0, %1, %2"
        : "=r"(result)
        : "r"(a), "r"(b)
    );
    
    return result;
}

/* ========== SECTION 5: Control Flow Diversity ========== */

/* Switch statement with fixed-point comparisons */
void test_switch_fixed(fract_t value) {
    int category = 0;
    
    /* Determine category based on fixed-point ranges */
    if (value < 0.25r) category = 1;
    else if (value < 0.5r) category = 2;
    else if (value < 0.75r) category = 3;
    else category = 4;
    
    /* Switch on the category */
    switch (category) {
        case 1:
            value = value * 2.0r;  /* Could overflow */
            break;
        case 2:
            value = value / 2.0r;
            break;
        case 3:
            value = value + 0.25r;  /* Could saturate */
            break;
        case 4:
            value = value - 0.25r;
            break;
    }
    
    volatile fract_t vv = value;
    (void)vv;
}

/* Loop with fixed-point exit condition */
void test_fixed_loop_condition(sat_accum_t initial, int max_iter) {
    sat_accum_t val = initial;
    int i = 0;
    
    /* Loop condition depends on fixed-point value */
    while (val > 0.1k && i < max_iter) {
        /* Complex update that requires range analysis */
        val = val * 0.9k - 0.05k;
        
        /* Nested ternary with fixed-point */
        val = (val < -5.0k) ? -5.0k : 
              (val > 5.0k) ? 5.0k : val;
        
        i++;
    }
    
    volatile sat_accum_t vval = val;
    (void)vval;
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char** argv) {
    /* Use command-line arguments for runtime variability */
    int iterations = 10;
    int array_size = 20;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 20;
        if (array_size > 100) array_size = 100;
    }
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n",
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    sat_accum_t* accum_array = (sat_accum_t*)malloc(array_size * sizeof(sat_accum_t));
    if (!accum_array) return 1;
    
    /* Fill array with values that will exercise range analysis */
    for (int i = 0; i < array_size; i++) {
        /* Varying values to create diverse ranges */
        accum_array[i] = (sat_accum_t)((i % 10) * 0.2k - 0.9k);
    }
    
    /* ===== Execute all tests ===== */
    
    /* 1. Loop-based range analysis */
    test_loop_range(iterations);
    
    /* 2. Array reduction */
    sat_accum_t reduction_result = array_reduction(accum_array, array_size);
    
    /* 3. Saturation boundaries */
    test_saturation_boundaries();
    
    /* 4. Mixed type operations */
    test_mixed_types(0.3r, 0.8r);
    test_mixed_types(-0.7r, 0.4r);
    
    /* 5. Builtin overflow */
    test_builtin_overflow();
    
    /* 6. Assembly operation */
    sat_accum_t asm_result = asm_fixed_mul(0.7k, 0.6k);
    
    /* 7. Switch with fixed-point */
    test_switch_fixed(0.3r);
    test_switch_fixed(0.6r);
    test_switch_fixed(0.9r);
    
    /* 8. Loop with fixed-point condition */
    test_fixed_loop_condition(3.0k, iterations);
    test_fixed_loop_condition(-3.0k, iterations);
    
    /* ===== Final checksum calculation ===== */
    
    /* Complex final expression using all results */
    sat_accum_t checksum = reduction_result + asm_result;
    
    /* Force range analysis with conditional */
    if (checksum > 10.0k) {
        checksum = 10.0k;
    } else if (checksum < -10.0k) {
        checksum = -10.0k;
    }
    
    /* Additional complex expression */
    for (int i = 0; i < 5; i++) {
        checksum = checksum * 0.95k + 0.1k;
    }
    
    /* Print final result (convert to float for display) */
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(accum_array);
    
    return 0;
}
