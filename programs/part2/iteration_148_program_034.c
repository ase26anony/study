/* test_fixed_point.c - Comprehensive test to trigger fixed-value.cc range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract;
typedef _Fract fract;
typedef _Sat _Accum sat_accum;
typedef _Accum accum;

/* Global arrays for inter-procedural analysis */
static sat_fract g_sat_fract_arr[32];
static fract g_fract_arr[32];
static sat_accum g_sat_accum_arr[16];
static accum g_accum_arr[16];

/* ========== HELPER FUNCTIONS FOR RANGE PROPAGATION ========== */

/* Function that performs saturated addition with range-dependent branching */
static inline sat_fract sat_fract_add(sat_fract a, sat_fract b) {
    /* This addition may saturate, requiring range analysis */
    sat_fract result = a + b;
    
    /* Conditional that depends on range analysis */
    if (a > 0.5r && b > 0.3r) {
        /* This branch should be taken when both values are in upper range */
        result = result + 0.1r;  /* May cause saturation */
    } else if (a < -0.5r && b < -0.3r) {
        /* This branch for negative saturation */
        result = result - 0.1r;
    }
    
    return result;
}

/* Function with complex fixed-point arithmetic requiring range analysis */
static inline sat_accum complex_accum_op(sat_accum x, sat_accum y, int shift) {
    /* Multiplication that may overflow */
    sat_accum prod = x * y;
    
    /* Shift operation that requires precise range tracking */
    if (shift > 0) {
        prod = prod >> shift;
    } else if (shift < 0) {
        prod = prod << (-shift);
    }
    
    /* Ternary operator with fixed-point operands */
    return (prod > 0.5k) ? (prod * 2.0k) : (prod / 2.0k);
}

/* Function that mixes saturated and non-saturated types */
static fract mixed_type_op(fract a, sat_fract b) {
    /* Conversion from saturated to non-saturated requires range check */
    fract temp = b;  /* Implicit conversion - may trigger range analysis */
    
    /* Operation that could overflow but won't saturate */
    return a * temp * 1.5r;
}

/* ========== LOOP-BASED RANGE ANALYSIS TESTS ========== */

/* Test 1: Loop with fixed-point induction variable */
static sat_accum test_loop_induction(int iterations) {
    sat_accum total = 0.0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (fract f = 0.1r; f < 0.9r && iterations > 0; f += 0.1r, iterations--) {
        /* Range of 'f' changes each iteration */
        sat_accum converted = f;  /* Conversion requires range analysis */
        total = total + converted * 2.0k;
        
        /* Conditional that depends on accumulated range */
        if (total > 10.0k) {
            total = total >> 1;  /* Shift to prevent overflow */
        }
    }
    
    return total;
}

/* Test 2: Array reduction with fixed-point values */
static sat_fract test_array_reduction(int size) {
    sat_fract sum = 0.0r;
    
    /* Initialize array with values that may cause saturation */
    for (int i = 0; i < size && i < 32; i++) {
        g_sat_fract_arr[i] = (i % 2 == 0) ? 0.7r : -0.7r;
    }
    
    /* Reduction that requires analyzing ranges across iterations */
    for (int i = 0; i < size && i < 32; i++) {
        sum = sum + g_sat_fract_arr[i];
        
        /* This conditional should trigger the uncovered range check */
        if (sum > 0.8r || sum < -0.8r) {
            /* Force evaluation of saturation boundaries */
            sum = (sum > 0) ? 0.9r : -0.9r;
        }
    }
    
    return sum;
}

/* Test 3: Nested loops with fixed-point computations */
static accum test_nested_loops(int outer, int inner) {
    accum result = 1.0k;
    
    for (int i = 0; i < outer; i++) {
        fract inner_acc = 0.5r;
        
        for (int j = 0; j < inner; j++) {
            /* Complex expression requiring range analysis */
            inner_acc = inner_acc * 1.1r - 0.05r;
            
            /* Switch based on fixed-point comparison */
            switch ((int)(inner_acc * 10)) {
                case 0 ... 3:
                    result = result * 0.8k;
                    break;
                case 4 ... 6:
                    result = result + 0.2k;
                    break;
                case 7 ... 10:
                    result = result - 0.3k;
                    break;
            }
        }
        
        /* Shift operation that may underflow/overflow */
        if (i % 2 == 0) {
            result = result >> 2;
        } else {
            result = result << 1;
        }
    }
    
    return result;
}

/* ========== SATURATION BOUNDARY TESTS ========== */

/* Test 4: Explicit saturation boundary testing */
static void test_saturation_boundaries(void) {
    sat_fract max_fract = 0.999999r;
    sat_fract min_fract = -0.999999r;
    sat_accum max_accum = 0.999999999k;
    sat_accum min_accum = -0.999999999k;
    
    /* Operations designed to hit saturation boundaries */
    sat_fract s1 = max_fract + 0.1r;  /* Should saturate to max */
    sat_fract s2 = min_fract - 0.1r;  /* Should saturate to min */
    
    /* Multiplication near boundaries */
    sat_accum m1 = max_accum * 1.1k;  /* Should saturate */
    sat_accum m2 = min_accum * 1.1k;  /* Should saturate */
    
    /* Use results to prevent dead code elimination */
    g_sat_fract_arr[0] = s1;
    g_sat_fract_arr[1] = s2;
    g_sat_accum_arr[0] = m1;
    g_sat_accum_arr[1] = m2;
}

/* Test 5: Built-in overflow checks with fixed-point */
static void test_builtin_overflow(int argc, char **argv) {
    fract f1 = 0.8r;
    fract f2 = 0.7r;
    sat_fract sf1 = 0.9r;
    sat_fract sf2 = 0.8r;
    
    /* Use command-line arguments for runtime variability */
    if (argc > 1) {
        int seed = atoi(argv[1]);
        f1 = (seed % 100) / 100.0r;
        f2 = ((seed + 37) % 100) / 100.0r;
    }
    
    /* Complex expression that may overflow */
    fract complex_expr = (f1 * 2.0r + f2 * 1.5r) / 0.5r;
    
    /* Conditional that depends on the range analysis */
    if (complex_expr > 1.0r || complex_expr < -1.0r) {
        /* This branch should be analyzed for reachability */
        sf1 = sf1 + 0.2r;  /* Will saturate */
    }
    
    /* Store results */
    g_fract_arr[0] = complex_expr;
    g_sat_fract_arr[2] = sf1;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    /* Use command-line arguments for runtime variability */
    int loop_iterations = 10;
    int array_size = 16;
    
    if (argc > 1) {
        loop_iterations = atoi(argv[1]) % 20 + 5;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]) % 24 + 8;
    }
    
    printf("Starting fixed-point range analysis tests...\n");
    printf("Parameters: loops=%d, array_size=%d\n", loop_iterations, array_size);
    
    /* Initialize global arrays */
    memset(g_sat_fract_arr, 0, sizeof(g_sat_fract_arr));
    memset(g_fract_arr, 0, sizeof(g_fract_arr));
    memset(g_sat_accum_arr, 0, sizeof(g_sat_accum_arr));
    memset(g_accum_arr, 0, sizeof(g_accum_arr));
    
    /* Run all tests */
    sat_accum test1_result = test_loop_induction(loop_iterations);
    sat_fract test2_result = test_array_reduction(array_size);
    accum test3_result = test_nested_loops(loop_iterations / 2, loop_iterations);
    
    test_saturation_boundaries();
    test_builtin_overflow(argc, argv);
    
    /* Additional complex expressions to trigger range analysis */
    for (int i = 0; i < array_size && i < 16; i++) {
        /* Mix different fixed-point types */
        g_accum_arr[i] = complex_accum_op(
            (i % 2 == 0) ? 0.3k : -0.3k,
            (i % 3 == 0) ? 0.4k : -0.4k,
            i % 5
        );
        
        /* Function calls with mixed types */
        g_fract_arr[i] = mixed_type_op(
            (i % 7) / 10.0r,
            (i % 5) / 10.0r
        );
    }
    
    /* Final checksum calculation using all results */
    accum final_checksum = 0.0k;
    final_checksum += test1_result;
    final_checksum += test2_result;
    final_checksum += test3_result;
    
    for (int i = 0; i < 16 && i < array_size; i++) {
        final_checksum += g_accum_arr[i];
        final_checksum += g_fract_arr[i];
        final_checksum += g_sat_accum_arr[i];
        final_checksum += g_sat_fract_arr[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Test 1 result: %f\n", (float)test1_result);
    printf("Test 2 result: %f\n", (float)test2_result);
    printf("Test 3 result: %f\n", (float)test3_result);
    printf("Final checksum: %f\n", (float)final_checksum);
    
    /* Use inline assembly to create hard-to-analyze value flows */
    sat_fract asm_var = 0.5r;
    asm volatile (
        "/* Fixed-point asm block */"
        : "+r" (asm_var)
        :
        : "memory"
    );
    printf("ASM result: %f\n", (float)asm_var);
    
    return 0;
}
