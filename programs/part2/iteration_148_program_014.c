/* test_fixed.c - Test program to trigger fixed-value.cc range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;

/* Fixed-point constants */
#define FRACT_MAX 0.999999r
#define FRACT_MIN (-1.0r)
#define ACCUM_MAX 32767.999999k
#define ACCUM_MIN (-32768.0k)

/* Test 1: Complex fixed-point expressions with saturation */
static inline sat_fract_t complex_sat_expr(sat_fract_t a, sat_fract_t b, int shift) {
    /* This will trigger range analysis for saturation */
    sat_fract_t temp = a + b;
    temp = temp * 0.75r;
    
    /* Shift operation that may cause overflow/underflow */
    if (shift > 0) {
        /* Simulate shift with multiplication */
        for (int i = 0; i < shift; i++) {
            temp = temp * 2.0r;
        }
    } else if (shift < 0) {
        for (int i = 0; i < -shift; i++) {
            temp = temp * 0.5r;
        }
    }
    
    /* Conditional based on range analysis */
    if (temp > 0.8r) {
        return temp * 0.9r;
    } else if (temp < -0.8r) {
        return temp * 1.1r;
    }
    return temp;
}

/* Test 2: Fixed-point operations in loops with range propagation */
static sat_accum_t loop_range_analysis(int iterations, sat_accum_t base) {
    sat_accum_t result = 0.0k;
    sat_accum_t step = 0.5k;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression that depends on loop variable */
        sat_accum_t term = base + (i * step);
        term = term * term;  /* Square it */
        
        /* Shift operation that requires range analysis */
        if (i % 3 == 0) {
            term = term >> 2;  /* Right shift 2 bits */
        } else if (i % 3 == 1) {
            term = term << 1;  /* Left shift 1 bit */
        }
        
        result = result + term;
        
        /* This condition will trigger the uncovered range check */
        if (result > 10000.0k || result < -10000.0k) {
            /* Force saturation boundary check */
            result = (result > 0) ? ACCUM_MAX : ACCUM_MIN;
        }
    }
    
    return result;
}

/* Test 3: Array reduction with fixed-point types */
static fract_t array_reduction(const fract_t* arr, int size) {
    fract_t sum = 0.0r;
    fract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Complex conditional that depends on range analysis */
        if (sum > 0.9r && product < -0.1r) {
            /* This branch should be analyzed for reachability */
            sum = sum - 0.2r;
        }
    }
    
    /* Ternary operator with fixed-point operands */
    return (sum > 0.5r) ? sum : product;
}

/* Test 4: Using builtins for overflow detection */
static int builtin_overflow_test(sat_fract_t a, sat_fract_t b, sat_fract_t* res) {
    int overflow = 0;
    
    /* Use builtin for overflow detection */
    overflow |= __builtin_add_overflow(a, b, res);
    
    sat_fract_t temp;
    overflow |= __builtin_mul_overflow(*res, 1.5r, &temp);
    
    /* Shift that might overflow */
    *res = temp;
    return overflow;
}

/* Test 5: Switch statement with fixed-point conditions */
static int switch_fixed_point(sat_accum_t val) {
    int result = 0;
    
    /* Switch on range-based conditions */
    switch ((val > 1000.0k) ? 1 : 
            (val < -1000.0k) ? 2 : 
            (val == 0.0k) ? 3 : 0) {
        case 1:
            result = 1;
            val = val * 0.5k;
            break;
        case 2:
            result = 2;
            val = val * 2.0k;
            break;
        case 3:
            result = 3;
            val = 0.25k;
            break;
        default:
            result = 0;
            val = val + 0.1k;
    }
    
    return result;
}

/* Test 6: Inter-procedural range analysis */
static inline sat_fract_t helper1(sat_fract_t x) {
    return x * 0.75r;
}

static inline sat_fract_t helper2(sat_fract_t x) {
    return x + 0.25r;
}

static sat_fract_t interprocedural_analysis(sat_fract_t a, sat_fract_t b) {
    sat_fract_t t1 = helper1(a);
    sat_fract_t t2 = helper2(b);
    
    /* This complex expression requires range analysis across calls */
    sat_fract_t result = (t1 > t2) ? (t1 - t2) : (t2 - t1);
    
    /* Force the specific condition from uncovered lines */
    if (result > FRACT_MAX || (result == FRACT_MAX && result > 0.0r)) {
        result = FRACT_MAX;
    }
    
    return result;
}

/* Test 7: Mixed saturated/unsaturated operations */
static void mixed_saturation_test(void) {
    fract_t unsat = 0.7r;
    sat_fract_t sat = 0.8r;
    
    /* Mix types - requires range analysis for conversion */
    sat_fract_t mixed = sat + (sat_fract_t)unsat;
    
    /* Operations that should hit saturation */
    for (int i = 0; i < 5; i++) {
        mixed = mixed + 0.3r;
    }
    
    /* Use asm to create hard-to-analyze value flow */
    sat_fract_t asm_result;
    asm volatile (
        "/* asm block for fixed-point */"
        : "=r" (asm_result)
        : "0" (mixed)
    );
    
    /* Conditional that depends on saturation state */
    if (asm_result == FRACT_MAX || asm_result == FRACT_MIN) {
        printf("Saturation boundary reached\n");
    }
}

/* Main test driver */
int main(int argc, char** argv) {
    int iterations = 10;
    int array_size = 20;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 20;
    }
    
    printf("Running fixed-point tests with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize arrays */
    fract_t* fract_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract_t)((i % 10) * 0.1r);
    }
    
    /* Run test 1: Complex expressions */
    sat_fract_t test1_result = 0.5r;
    for (int i = 0; i < iterations; i++) {
        test1_result = complex_sat_expr(test1_result, 0.3r, i % 4);
    }
    printf("Test 1 result: %f\n", (float)test1_result);
    
    /* Run test 2: Loop range analysis */
    sat_accum_t test2_result = loop_range_analysis(iterations, 100.0k);
    printf("Test 2 result: %f\n", (float)test2_result);
    
    /* Run test 3: Array reduction */
    fract_t test3_result = array_reduction(fract_array, array_size);
    printf("Test 3 result: %f\n", (float)test3_result);
    
    /* Run test 4: Builtin overflow */
    sat_fract_t overflow_res;
    int overflow = builtin_overflow_test(0.9r, 0.8r, &overflow_res);
    printf("Test 4: overflow=%d, result=%f\n", overflow, (float)overflow_res);
    
    /* Run test 5: Switch statement */
    int switch_result = switch_fixed_point(test2_result);
    printf("Test 5 switch result: %d\n", switch_result);
    
    /* Run test 6: Inter-procedural */
    sat_fract_t test6_result = interprocedural_analysis(0.6r, 0.7r);
    printf("Test 6 result: %f\n", (float)test6_result);
    
    /* Run test 7: Mixed saturation */
    mixed_saturation_test();
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = 0.0k;
    checksum = checksum + (sat_accum_t)test1_result;
    checksum = checksum + test2_result;
    checksum = checksum + (sat_accum_t)test3_result;
    checksum = checksum + (sat_accum_t)overflow_res;
    checksum = checksum + (sat_accum_t)test6_result;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    free(fract_array);
    return 0;
}
