/* test_fixed.c - Program to trigger fixed-point range analysis in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract;
typedef _Fract fract;
typedef _Sat _Accum sat_accum;
typedef _Accum accum;
typedef _Sat short _Fract sat_short_fract;
typedef short _Fract short_fract;

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline sat_accum fp_complex_calc(sat_accum a, sat_accum b, fract c) {
    /* Operations that require range analysis */
    sat_accum t1 = a * b;  /* Multiplication with potential overflow */
    sat_accum t2 = t1 >> 3; /* Right shift causing underflow */
    sat_accum t3 = t2 + (sat_accum)c; /* Addition with different types */
    
    /* Conditional based on range analysis */
    if (t3 > 0.5k) {
        return t3 * 2.0k;  /* Could overflow */
    } else {
        return t3 / 2.0k;  /* Could underflow */
    }
}

/* Test function 2: Loop-based range propagation */
static sat_fract fp_loop_reduction(int iterations, fract base) {
    sat_fract total = 0.0r;
    fract step = 0.1r;
    
    /* Loop where induction variable affects fixed-point range */
    for (fract f = base; f < 0.9r; f += step) {
        /* Complex expression requiring range analysis */
        sat_fract temp = f * f;  /* Square - stays in [0, 0.81] */
        temp = temp + 0.1r;      /* Now in [0.1, 0.91] - may saturate */
        total = total + temp;    /* Accumulation - may saturate */
        
        if (iterations-- <= 0) break;
    }
    
    /* Ternary operator with fixed-point operands */
    return (total > 0.5r) ? total : 0.5r - total;
}

/* Test function 3: Array operations with fixed-point */
static sat_accum fp_array_product(const fract arr[], int size) {
    sat_accum product = 1.0k;
    
    for (int i = 0; i < size; i++) {
        /* Multiplication that could overflow/underflow */
        product = product * (sat_accum)arr[i];
        
        /* Conditional that depends on range analysis */
        if (product < 0.1k && product > -0.1k) {
            /* Near zero - shift to avoid underflow */
            product = product << 2;
        } else if (product > 0.8k || product < -0.8k) {
            /* Near limits - shift to avoid overflow */
            product = product >> 2;
        }
    }
    
    return product;
}

/* Test function 4: Using builtins for overflow detection */
static int fp_overflow_checks(sat_fract a, sat_fract b, sat_fract *result) {
    int overflow = 0;
    
    /* Builtin overflow check */
    overflow |= __builtin_add_overflow((int)a, (int)b, (int*)result);
    
    /* Direct operations that should saturate */
    sat_fract sum = a + b;
    sat_fract prod = a * b;
    
    /* Mix saturated and unsaturated */
    fract unsat_sum = (fract)sum + (fract)prod;
    
    /* Range-dependent conditional */
    if (sum > 0.9r || prod > 0.8r) {
        *result = sum;
        return 1;
    }
    
    *result = (sat_fract)unsat_sum;
    return overflow;
}

/* Test function 5: Switch statement with fixed-point conditions */
static int fp_switch_demo(sat_accum val) {
    int category = 0;
    
    /* Switch based on fixed-point range analysis */
    if (val < -0.5k) {
        category = 1;
    } else if (val >= -0.5k && val < 0.0k) {
        category = 2;
    } else if (val >= 0.0k && val < 0.5k) {
        category = 3;
    } else {
        category = 4;  /* val >= 0.5k, could be saturated */
    }
    
    switch (category) {
        case 1:
            return (val * 2.0k) < -0.9k ? -1 : 0;
        case 2:
            return (val + 0.5k) > 0.0k ? 1 : 0;
        case 3:
            return (val * 3.0k) > 1.0k ? 2 : 0;  /* Should saturate */
        case 4:
            return (val / 2.0k) < 0.5k ? 3 : 0;
        default:
            return -1;
    }
}

/* Test function 6: Nested function calls with fixed-point */
static inline sat_fract fp_inner(fract a, fract b) {
    /* Operations that require precise range analysis */
    sat_fract t = (sat_fract)a * (sat_fract)b;
    t = t + (sat_fract)(a + b);
    return t;
}

static sat_accum fp_outer(sat_accum x, fract y, int count) {
    sat_accum result = x;
    
    for (int i = 0; i < count; i++) {
        /* Nested calls with different argument ranges */
        fract f_arg = (fract)(i % 100) / 100.0r;
        sat_fract inner = fp_inner(f_arg, y);
        
        /* Complex expression */
        result = result + (sat_accum)inner * 0.1k;
        
        /* Bit shift that could overflow */
        if (i % 3 == 0) {
            result = result << 1;
        } else if (i % 3 == 1) {
            result = result >> 1;
        }
        
        /* Range check that should trigger the uncovered code */
        if (result > 0.9k || result < -0.9k) {
            result = result / 2.0k;
        }
    }
    
    return result;
}

/* Test function 7: Assembly integration for hard-to-analyze flows */
static sat_accum fp_asm_mix(sat_accum a, sat_accum b) {
    sat_accum result;
    
    /* Assembly that creates opaque value flow */
    asm volatile (
        "/* Fixed-point manipulation %0 = (%1 + %2) / 2 */"
        : "=r"(result)
        : "r"(a), "r"(b)
    );
    
    /* Follow-up operations that need range analysis */
    result = result * 3.0k;
    
    /* Conditional on potentially unknown range */
    if (result > 0.0k) {
        return result >> 2;
    } else {
        return result << 2;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 10;
    int array_size = 8;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 8;
        if (array_size > 100) array_size = 100;
    }
    
    /* Initialize fixed-point arrays */
    fract f_array[100];
    for (int i = 0; i < array_size && i < 100; i++) {
        f_array[i] = (fract)i / (fract)array_size;
    }
    
    /* Test 1: Complex calculations */
    sat_accum test1 = fp_complex_calc(0.7k, 0.8k, 0.3r);
    printf("Test1 result: %f\n", (float)test1);
    
    /* Test 2: Loop reduction */
    sat_fract test2 = fp_loop_reduction(iterations, 0.0r);
    printf("Test2 result: %f\n", (float)test2);
    
    /* Test 3: Array product */
    sat_accum test3 = fp_array_product(f_array, array_size);
    printf("Test3 result: %f\n", (float)test3);
    
    /* Test 4: Overflow checks */
    sat_fract result4;
    int overflow = fp_overflow_checks(0.9r, 0.8r, &result4);
    printf("Test4 result: %f, overflow: %d\n", (float)result4, overflow);
    
    /* Test 5: Switch demo */
    int test5 = fp_switch_demo(0.6k);
    printf("Test5 category: %d\n", test5);
    
    /* Test 6: Nested calls */
    sat_accum test6 = fp_outer(0.1k, 0.2r, iterations);
    printf("Test6 result: %f\n", (float)test6);
    
    /* Test 7: Assembly mix */
    sat_accum test7 = fp_asm_mix(0.4k, 0.5k);
    printf("Test7 result: %f\n", (float)test7);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum checksum = test1 + (sat_accum)test2 + test3 + 
                        (sat_accum)result4 + (sat_accum)test5 + 
                        test6 + test7;
    
    /* Normalize checksum */
    checksum = checksum / (sat_accum)(7.0k);
    
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Additional edge cases to trigger saturation logic */
    sat_fract max_fract = 0.999999r;
    sat_fract saturated = max_fract + 0.1r;  /* Should saturate */
    printf("Saturation test: %f + 0.1 = %f\n", (float)max_fract, (float)saturated);
    
    sat_accum min_accum = -0.999999k;
    sat_accum underflow = min_accum - 0.1k;  /* Should saturate */
    printf("Underflow test: %f - 0.1 = %f\n", (float)min_accum, (float)underflow);
    
    /* Mixed-type operations */
    fract f1 = 0.5r;
    sat_accum a1 = 0.5k;
    sat_accum mixed = a1 * (sat_accum)f1 + 0.25k;
    printf("Mixed-type result: %f\n", (float)mixed);
    
    /* Bit shift operations on fixed-point */
    sat_accum shifted = 0.25k;
    shifted = shifted << 3;  /* Could overflow */
    printf("Shift test: 0.25 << 3 = %f\n", (float)shifted);
    
    return 0;
}
