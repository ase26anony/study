/* test_fixed_point.c - Target coverage of fixed-value.cc lines 264-277 */
/* Compile with: gcc -O3 -ffixed-point -fdump-tree-all -o test_fixed test_fixed_point.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat short _Fract sat_short_fract_t;

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline sat_fract_t sat_add_with_overflow(sat_fract_t a, sat_fract_t b) {
    /* This should trigger saturation analysis */
    sat_fract_t result = a + b;
    
    /* Complex expression requiring range analysis */
    sat_fract_t scaled = (result * 0.5r) >> 1;
    
    /* Conditional based on range analysis */
    if (result > 0.9r) {
        return scaled + 0.1r;
    } else if (result < -0.9r) {
        return scaled - 0.1r;
    }
    return result;
}

/* Function 2: Nested range propagation */
static inline sat_accum_t accumulate_range(sat_accum_t base, fract_t multiplier, int shift) {
    /* Multi-step calculation requiring range tracking */
    sat_accum_t temp = base * (sat_accum_t)multiplier;
    
    /* Shift operation that can cause overflow/underflow */
    if (shift > 0) {
        temp = temp >> shift;
    } else {
        temp = temp << (-shift);
    }
    
    /* Ternary operator with fixed-point operands */
    return (temp > 0.5k) ? temp - 0.1k : temp + 0.1k;
}

/* Function 3: Loop-based range analysis */
static sat_accum_t loop_reduction(fract_t* array, int size, sat_accum_t initial) {
    sat_accum_t total = initial;
    
    for (int i = 0; i < size; i++) {
        /* Induction variable affects range */
        fract_t scale = (fract_t)i / (fract_t)size;
        
        /* Complex expression in loop */
        sat_accum_t contribution = (sat_accum_t)array[i] * (0.5k + (sat_accum_t)scale);
        
        /* Conditional update based on range */
        if (contribution + total > 10.0k) {
            total = 10.0k;  /* Should saturate */
        } else if (contribution + total < -10.0k) {
            total = -10.0k; /* Should saturate */
        } else {
            total += contribution;
        }
    }
    
    return total;
}

/* Function 4: Switch statement with fixed-point conditions */
static fract_t switch_based_on_range(sat_fract_t value) {
    /* Switch on discretized range */
    int range;
    
    if (value > 0.8r) range = 0;
    else if (value > 0.6r) range = 1;
    else if (value > 0.4r) range = 2;
    else if (value > 0.2r) range = 3;
    else if (value > -0.2r) range = 4;
    else if (value > -0.4r) range = 5;
    else if (value > -0.6r) range = 6;
    else if (value > -0.8r) range = 7;
    else range = 8;
    
    switch (range) {
        case 0: return 1.0r;
        case 1: return 0.8r;
        case 2: return 0.6r;
        case 3: return 0.4r;
        case 4: return 0.0r;
        case 5: return -0.4r;
        case 6: return -0.6r;
        case 7: return -0.8r;
        default: return -1.0r;
    }
}

/* Function 5: Built-in overflow checks */
static int check_mul_overflow(sat_accum_t a, sat_accum_t b, sat_accum_t* res) {
    /* Use builtin for overflow detection */
    return __builtin_mul_overflow(a, b, res);
}

/* Function 6: Mixed saturation types */
static sat_short_fract_t mixed_saturation_ops(fract_t a, sat_fract_t b) {
    /* Mix different fixed-point types */
    sat_short_fract_t s1 = (sat_short_fract_t)a;
    sat_short_fract_t s2 = (sat_short_fract_t)b;
    
    /* Operations that may saturate */
    sat_short_fract_t sum = s1 + s2;
    sat_short_fract_t prod = sum * (sat_short_fract_t)0.5r;
    
    /* Shift operation */
    return prod >> 2;
}

/* Function 7: Complex expression tree */
static sat_accum_t complex_expression_tree(sat_accum_t a, sat_accum_t b, sat_accum_t c) {
    /* Deep expression tree requiring range analysis */
    sat_accum_t t1 = (a + b) * 0.25k;
    sat_accum_t t2 = (b - c) * 0.75k;
    sat_accum_t t3 = (a * c) >> 2;
    
    /* Nested conditional */
    if ((t1 + t2) > 5.0k) {
        return t3 + 1.0k;
    } else if ((t1 + t2) < -5.0k) {
        return t3 - 1.0k;
    } else {
        /* Further complex expression */
        sat_accum_t result = (t1 * t2) / 2.0k;
        
        /* Check for overflow using builtin */
        sat_accum_t final;
        if (__builtin_add_overflow(result, t3, &final)) {
            return (result > 0) ? 7.0k : -7.0k;
        }
        return final;
    }
}

/* Main test driver */
int main(int argc, char** argv) {
    int iterations = 100;
    int array_size = 50;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 10) array_size = 10;
        if (array_size > 200) array_size = 200;
    }
    
    /* Initialize arrays */
    fract_t* frac_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    sat_accum_t* accum_array = (sat_accum_t*)malloc(array_size * sizeof(sat_accum_t));
    
    if (!frac_array || !accum_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with values that will trigger range analysis */
    for (int i = 0; i < array_size; i++) {
        /* Values designed to approach saturation boundaries */
        frac_array[i] = ((fract_t)i / (fract_t)array_size) * 2.0r - 1.0r;
        accum_array[i] = ((sat_accum_t)i * 0.1k) - 5.0k;
    }
    
    sat_accum_t total_accum = 0.0k;
    sat_fract_t total_fract = 0.0r;
    
    /* Main test loop */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs to explore different paths */
        fract_t base_frac = (fract_t)iter / (fract_t)iterations;
        sat_accum_t base_accum = (sat_accum_t)iter * 0.05k;
        
        /* Test 1: Saturation boundaries */
        sat_fract_t sat_result = sat_add_with_overflow(
            (sat_fract_t)base_frac, 
            (sat_fract_t)(0.9r - base_frac)
        );
        
        /* Test 2: Range propagation */
        sat_accum_t range_result = accumulate_range(
            base_accum,
            base_frac,
            iter % 5 - 2  /* Vary shift between -2 and 2 */
        );
        
        /* Test 3: Loop reduction */
        sat_accum_t loop_result = loop_reduction(
            frac_array,
            array_size,
            base_accum
        );
        
        /* Test 4: Switch based on range */
        fract_t switch_result = switch_based_on_range(sat_result);
        
        /* Test 5: Overflow checks */
        sat_accum_t mul_result;
        int overflow = check_mul_overflow(range_result, loop_result, &mul_result);
        
        /* Test 6: Mixed saturation types */
        sat_short_fract_t mixed_result = mixed_saturation_ops(
            switch_result,
            sat_result
        );
        
        /* Test 7: Complex expression tree */
        sat_accum_t complex_result = complex_expression_tree(
            range_result,
            loop_result,
            mul_result
        );
        
        /* Accumulate results (ensures no dead code elimination) */
        total_accum += range_result + loop_result + complex_result;
        if (!overflow) {
            total_accum += mul_result;
        }
        total_fract += sat_result + (sat_fract_t)switch_result + (sat_fract_t)mixed_result;
        
        /* Modify arrays slightly each iteration */
        if (iter % 10 == 0) {
            for (int i = 0; i < array_size; i++) {
                frac_array[i] = frac_array[i] * 0.95r;
                accum_array[i] = accum_array[i] * 0.95k;
            }
        }
    }
    
    /* Final checksum calculation */
    sat_accum_t final_checksum = total_accum + (sat_accum_t)total_fract;
    
    /* Convert to float for printing (prevents dead code elimination) */
    printf("Final checksum: %f\n", (float)final_checksum);
    printf("Fract total: %f\n", (float)total_fract);
    printf("Accum total: %f\n", (float)total_accum);
    
    /* Cleanup */
    free(frac_array);
    free(accum_array);
    
    return 0;
}
