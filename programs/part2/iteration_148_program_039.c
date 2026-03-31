/* test_fixed.c - Program to exercise fixed-point range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;

/* Test functions with various fixed-point operations */

/* Function 1: Simple saturated addition that may overflow */
static inline sfract_t sat_add(sfract_t a, sfract_t b) {
    return a + b;  /* Should trigger saturation analysis */
}

/* Function 2: Multiplication with shift - requires precise range analysis */
static inline saccum_t mul_shift(saccum_t x, saccum_t y, int shift) {
    saccum_t prod = x * y;
    return prod >> shift;  /* Shift may cause overflow/underflow */
}

/* Function 3: Division with saturation check */
static inline sfract_t safe_div(sfract_t num, sfract_t den) {
    /* This condition should trigger range comparison logic */
    if (den == 0.0r) {
        return 0.0r;
    }
    return num / den;
}

/* Function 4: Complex expression with multiple operations */
static inline accum_t complex_expr(accum_t a, accum_t b, accum_t c) {
    accum_t t1 = a * b;
    accum_t t2 = t1 >> 3;
    accum_t t3 = c / 2.0k;
    return t2 + t3;
}

/* Function 5: Array reduction with saturation */
static sfract_t array_sum(sfract_t arr[], int n) {
    sfract_t total = 0.0r;
    for (int i = 0; i < n; i++) {
        total = total + arr[i];  /* May saturate */
    }
    return total;
}

/* Function 6: Nested loops with fixed-point induction */
static saccum_t nested_loop_test(int iterations) {
    saccum_t result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        fract_t inner = 0.1r;
        for (int j = 0; j < 5; j++) {
            result = result + (_Accum)inner;
            inner = inner + 0.1r;
        }
    }
    return result;
}

/* Function 7: Switch based on fixed-point comparison */
static int switch_test(sfract_t val) {
    switch ((int)(val * 10.0r)) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
        case 3: return 4;
        case 4: return 5;
        case 5: return 6;
        case 6: return 7;
        case 7: return 8;
        case 8: return 9;
        default: return 10;
    }
}

/* Function 8: Ternary operator with fixed-point */
static sfract_t ternary_test(sfract_t a, sfract_t b, int flag) {
    return flag ? (a + b) : (a - b);
}

/* Function 9: Builtin overflow check */
static int check_overflow(saccum_t *result, saccum_t a, saccum_t b) {
    return __builtin_add_overflow(a, b, result);
}

/* Function 10: Mixed saturation types */
static void mixed_saturation(sfract_t *out, fract_t in1, sfract_t in2) {
    *out = in1 + in2;  /* Mixing sat and non-sat types */
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 10;
    int seed = 42;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    /* Initialize arrays */
    sfract_t sf_arr[20];
    fract_t f_arr[20];
    saccum_t sa_arr[10];
    
    for (int i = 0; i < 20; i++) {
        float r = (float)rand() / RAND_MAX;
        sf_arr[i] = (_Fract)r;
        f_arr[i] = (_Fract)r;
        if (i < 10) {
            sa_arr[i] = (_Accum)(r * 2.0 - 1.0);
        }
    }
    
    /* Test 1: Saturated addition near boundaries */
    sfract_t max_fract = 0.999999r;
    sfract_t test1 = sat_add(max_fract, 0.1r);
    printf("Test1 (saturated add): %f\n", (float)test1);
    
    /* Test 2: Multiplication with shift */
    saccum_t test2 = mul_shift(0.5k, 2.0k, 1);
    printf("Test2 (mul shift): %f\n", (float)test2);
    
    /* Test 3: Division with zero check */
    sfract_t test3 = safe_div(0.5r, 0.0r);
    printf("Test3 (safe div): %f\n", (float)test3);
    
    /* Test 4: Complex expression */
    accum_t test4 = complex_expr(0.25k, 4.0k, 0.5k);
    printf("Test4 (complex expr): %f\n", (float)test4);
    
    /* Test 5: Array reduction - may saturate */
    sfract_t test5 = array_sum(sf_arr, 20);
    printf("Test5 (array sum): %f\n", (float)test5);
    
    /* Test 6: Nested loops */
    saccum_t test6 = nested_loop_test(iterations);
    printf("Test6 (nested loops): %f\n", (float)test6);
    
    /* Test 7: Switch statement */
    int test7 = switch_test(0.75r);
    printf("Test7 (switch): %d\n", test7);
    
    /* Test 8: Ternary operator */
    sfract_t test8 = ternary_test(0.3r, 0.4r, iterations % 2);
    printf("Test8 (ternary): %f\n", (float)test8);
    
    /* Test 9: Builtin overflow check */
    saccum_t overflow_result;
    int overflow_flag = check_overflow(&overflow_result, 0.9k, 0.2k);
    printf("Test9 (overflow check): result=%f, overflow=%d\n", 
           (float)overflow_result, overflow_flag);
    
    /* Test 10: Mixed saturation */
    sfract_t mixed_result;
    mixed_saturation(&mixed_result, 0.7r, 0.3r);
    printf("Test10 (mixed saturation): %f\n", (float)mixed_result);
    
    /* Additional boundary tests */
    
    /* Test 11: Direct saturation boundary test */
    _Sat _Fract s_max = 0.999999r;
    _Sat _Fract s_min = -0.999999r;
    s_max = s_max + 0.1r;  /* Should saturate to max */
    s_min = s_min - 0.1r;  /* Should saturate to min */
    printf("Test11 (boundary sat): max=%f, min=%f\n", (float)s_max, (float)s_min);
    
    /* Test 12: Bit shift operations */
    _Sat _Accum shift_test = 0.5k;
    for (int i = 0; i < 5; i++) {
        shift_test = shift_test >> 1;
    }
    printf("Test12 (shift chain): %f\n", (float)shift_test);
    
    /* Test 13: Loop with fixed-point condition */
    fract_t loop_cond = 0.0r;
    int counter = 0;
    while (loop_cond < 0.9r) {
        loop_cond = loop_cond + 0.1r;
        counter++;
    }
    printf("Test13 (loop cond): final=%f, iterations=%d\n", 
           (float)loop_cond, counter);
    
    /* Test 14: Asm volatile to create hard-to-analyze flows */
    accum_t asm_input = 0.25k;
    accum_t asm_output;
    asm volatile (
        "/* %0 = %1 * 2 */"
        : "=r" (asm_output)
        : "r" (asm_input)
    );
    printf("Test14 (asm): %f\n", (float)asm_output);
    
    /* Final checksum calculation to prevent dead code elimination */
    accum_t checksum = (_Accum)test1 + (_Accum)test2 + (_Accum)test3 +
                      test4 + (_Accum)test5 + test6 + (_Accum)test8 +
                      overflow_result + (_Accum)mixed_result +
                      (_Accum)s_max + (_Accum)s_min + shift_test +
                      (_Accum)loop_cond + asm_output;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    return 0;
}
