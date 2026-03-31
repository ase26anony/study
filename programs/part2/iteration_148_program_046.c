/* test_fixed_point.c - Comprehensive fixed-point test targeting GCC's fixed-value.cc lines 264-277 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define HOST_BITS_PER_DOUBLE_INT if not already defined */
#ifndef HOST_BITS_PER_DOUBLE_INT
#define HOST_BITS_PER_DOUBLE_INT (sizeof(long long) * 8)
#endif

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline _Sat _Fract sat_fract_add_mul(_Sat _Fract a, _Sat _Fract b, _Sat _Fract c) {
    /* This should trigger range analysis for saturation */
    _Sat _Fract tmp = a + b;
    /* Multiplication can overflow even when addition doesn't */
    return tmp * c;
}

/* Test function 2: Accumulator with shifting */
static inline _Sat _Accum accum_shift_ops(_Sat _Accum x, int shift) {
    /* Shifting fixed-point values requires careful range analysis */
    _Sat _Accum result;
    if (shift > 0) {
        result = x >> shift;
    } else {
        result = x << (-shift);
    }
    return result;
}

/* Test function 3: Mixed saturation types */
static inline _Sat _Fract mixed_saturation(_Fract a, _Sat _Fract b) {
    /* Mixing saturated and unsaturated types triggers conversion checks */
    _Sat _Fract sat_a = a;  /* Implicit saturation conversion */
    return sat_a + b;
}

/* Test function 4: Range analysis with conditional */
static inline int range_check_conditional(_Sat _Accum x, _Sat _Accum y) {
    /* This should force evaluation of range comparisons */
    _Sat _Accum prod = x * y;
    
    /* Create condition similar to the uncovered code logic */
    if (prod > 0.5k) {
        return 1;
    } else if (prod < -0.5k) {
        return -1;
    }
    return 0;
}

/* Test function 5: Loop-based range propagation */
static _Sat _Accum loop_range_propagation(int iterations, _Sat _Accum base) {
    _Sat _Accum result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        /* Complex expression that depends on loop variable */
        _Sat _Accum term = base * (_Sat _Accum)i;
        result = result + (term >> 2);  /* Shift within loop */
        
        /* Conditional that depends on accumulated value */
        if (result > 0.8k) {
            result = result * 0.5k;  /* Scale down if too large */
        }
    }
    return result;
}

/* Test function 6: Array reduction with fixed-point */
static _Sat _Fract array_reduction(_Sat _Fract arr[], int size) {
    _Sat _Fract sum = 0.0r;
    for (int i = 0; i < size; i++) {
        sum = sum + arr[i];
        /* Multiplication that could saturate */
        if (i % 3 == 0) {
            sum = sum * 1.1r;  /* Intentional potential overflow */
        }
    }
    return sum;
}

/* Test function 7: Using builtins for overflow detection */
static int builtin_overflow_test(_Sat _Fract a, _Sat _Fract b, _Sat _Fract *res) {
    /* Use builtin to check for overflow */
    return __builtin_add_overflow(a, b, res);
}

/* Test function 8: Switch based on fixed-point comparisons */
static int switch_fixed_point(_Sat _Accum val) {
    int result = 0;
    
    /* Switch where cases depend on fixed-point range analysis */
    switch (range_check_conditional(val, 0.5k)) {
        case 1:
            result = val * 2.0k;  /* Scale up */
            break;
        case -1:
            result = val * 0.5k;  /* Scale down */
            break;
        default:
            result = val;  /* Keep as is */
            break;
    }
    return result;
}

/* Test function 9: Ternary operator with fixed-point */
static _Sat _Fract ternary_fixed_point(_Sat _Fract a, _Sat _Fract b, int flag) {
    /* Ternary with fixed-point in both branches */
    return flag ? (a * 1.5r) : (b * 0.5r);
}

/* Test function 10: Nested function calls for inter-procedural analysis */
static _Sat _Accum nested_range_analysis(_Sat _Accum x) {
    /* Multiple transformations that require range tracking */
    _Sat _Accum y = accum_shift_ops(x, 3);
    _Sat _Accum z = y * 0.75k;
    
    /* Conditional that depends on complex expression */
    if (z > 0.9k || z < -0.9k) {
        return z * 0.5k;
    }
    return z;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 10;
    int array_size = 20;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 5) array_size = 20;
    }
    
    /* Initialize fixed-point arrays */
    _Sat _Fract fract_array[array_size];
    _Sat _Accum accum_array[array_size];
    
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (_Sat _Fract)(i * 0.05r);
        accum_array[i] = (_Sat _Accum)(i * 0.1k);
    }
    
    /* Test 1: Saturation boundary tests */
    printf("Test 1: Saturation boundaries\n");
    _Sat _Fract max_fract = 0.999999r;
    _Sat _Fract min_fract = -1.0r;
    
    /* Operations designed to hit saturation */
    _Sat _Fract sat_test1 = max_fract + 0.1r;  /* Should saturate */
    _Sat _Fract sat_test2 = min_fract - 0.1r;  /* Should saturate */
    _Sat _Fract sat_test3 = max_fract * 1.1r;  /* Should saturate */
    
    printf("  max+0.1: %f\n", (float)sat_test1);
    printf("  min-0.1: %f\n", (float)sat_test2);
    printf("  max*1.1: %f\n", (float)sat_test3);
    
    /* Test 2: Complex arithmetic with shifting */
    printf("\nTest 2: Shift operations\n");
    _Sat _Accum accum_val = 0.5k;
    for (int i = 0; i < 5; i++) {
        _Sat _Accum shifted = accum_shift_ops(accum_val, i - 2);
        printf("  0.5k shift %d: %f\n", i - 2, (float)shifted);
    }
    
    /* Test 3: Mixed type conversions */
    printf("\nTest 3: Mixed type conversions\n");
    _Fract unsat_fract = 0.8r;
    _Sat _Fract sat_fract = 0.7r;
    _Sat _Fract mixed_result = mixed_saturation(unsat_fract, sat_fract);
    printf("  Mixed result: %f\n", (float)mixed_result);
    
    /* Test 4: Range check conditional */
    printf("\nTest 4: Range check conditionals\n");
    int range_result = range_check_conditional(0.6k, 0.9k);
    printf("  Range check (0.6k * 0.9k): %d\n", range_result);
    
    /* Test 5: Loop-based propagation */
    printf("\nTest 5: Loop range propagation\n");
    _Sat _Accum loop_result = loop_range_propagation(iterations, 0.2k);
    printf("  Loop result (%d iterations): %f\n", iterations, (float)loop_result);
    
    /* Test 6: Array reduction */
    printf("\nTest 6: Array reduction\n");
    _Sat _Fract reduction_result = array_reduction(fract_array, array_size);
    printf("  Array reduction: %f\n", (float)reduction_result);
    
    /* Test 7: Builtin overflow */
    printf("\nTest 7: Builtin overflow detection\n");
    _Sat _Fract builtin_res;
    int overflow = builtin_overflow_test(0.9r, 0.2r, &builtin_res);
    printf("  Overflow check (0.9r + 0.2r): overflow=%d, result=%f\n", 
           overflow, (float)builtin_res);
    
    /* Test 8: Switch statement */
    printf("\nTest 8: Switch based on fixed-point\n");
    int switch_result = switch_fixed_point(0.3k);
    printf("  Switch result: %d\n", switch_result);
    
    /* Test 9: Ternary operator */
    printf("\nTest 9: Ternary operator\n");
    _Sat _Fract ternary_result = ternary_fixed_point(0.4r, 0.6r, 1);
    printf("  Ternary result: %f\n", (float)ternary_result);
    
    /* Test 10: Nested analysis */
    printf("\nTest 10: Nested range analysis\n");
    _Sat _Accum nested_result = nested_range_analysis(0.8k);
    printf("  Nested result: %f\n", (float)nested_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    _Sat _Accum checksum = 0.0k;
    
    /* Complex expression that uses all test results */
    checksum = checksum + (_Sat _Accum)sat_test1;
    checksum = checksum + (_Sat _Accum)sat_test2;
    checksum = checksum + (_Sat _Accum)sat_test3;
    checksum = checksum + accum_val;
    checksum = checksum + (_Sat _Accum)mixed_result;
    checksum = checksum + (_Sat _Accum)range_result;
    checksum = checksum + loop_result;
    checksum = checksum + (_Sat _Accum)reduction_result;
    checksum = checksum + (_Sat _Accum)builtin_res;
    checksum = checksum + (_Sat _Accum)switch_result;
    checksum = checksum + (_Sat _Accum)ternary_result;
    checksum = checksum + nested_result;
    
    /* Force range analysis with conditional */
    if (checksum > 10.0k) {
        checksum = checksum * 0.1k;
    } else if (checksum < -10.0k) {
        checksum = checksum * 0.1k;
    }
    
    printf("Final checksum: %f\n", (float)checksum);
    
    return 0;
}
