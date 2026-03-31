/* test_fixed.c - Test program to trigger fixed-value.cc uncovered lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all test_fixed.c -o test_fixed */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract sat_long_fract_t;

/* Complex fixed-point operations in static inline functions */
static inline sat_fract_t add_with_saturation(sat_fract_t a, sat_fract_t b) {
    /* This addition may saturate, triggering range analysis */
    return a + b;
}

static inline sat_accum_t multiply_saturating(sat_accum_t x, sat_accum_t y) {
    /* Multiplication that requires precise overflow analysis */
    return x * y;
}

static inline fract_t shift_fract(fract_t x, int shift) {
    /* Bit shifts on fixed-point values */
    return shift > 0 ? x >> shift : x << (-shift);
}

/* Function with conditional depending on fixed-point range */
static int check_range(sat_fract_t val) {
    /* This condition forces evaluation of range comparison logic */
    if (val > 0.9r) {
        return 1;  /* In upper saturation region */
    } else if (val < -0.9r) {
        return -1; /* In lower saturation region */
    }
    return 0;      /* In normal range */
}

/* Loop-based range analysis test */
static sat_accum_t accumulate_array(const fract_t* arr, int n) {
    sat_accum_t total = 0.0k;
    for (int i = 0; i < n; i++) {
        /* Range of total evolves with each iteration */
        total = total + (sat_accum_t)arr[i];
        
        /* Conditional that depends on accumulating range */
        if (total > 10.0k || total < -10.0k) {
            /* Force consideration of saturation boundaries */
            total = total * 0.5k;
        }
    }
    return total;
}

/* Nested function calls for inter-procedural analysis */
static sat_fract_t complex_expression(fract_t a, fract_t b, fract_t c) {
    sat_fract_t sat_a = a;
    sat_fract_t sat_b = b;
    
    /* Expression designed to hit saturation boundaries */
    sat_fract_t temp = add_with_saturation(sat_a, sat_b);
    temp = add_with_saturation(temp, c);
    
    /* Multiplication that could overflow */
    temp = temp * 1.5r;
    
    /* Shift operation */
    temp = shift_fract(temp, 2);
    
    return temp;
}

/* Test with GCC builtins for overflow detection */
static int test_builtin_overflow(sat_accum_t a, sat_accum_t b, sat_accum_t* res) {
    /* Use overflow builtins with fixed-point types */
    return __builtin_add_overflow(a, b, res);
}

/* Switch statement with fixed-point control */
static const char* classify_fract(sat_fract_t val) {
    /* Switch on range classification */
    switch (check_range(val)) {
        case 1:  return "HIGH";
        case -1: return "LOW";
        case 0:  return "MID";
        default: return "ERROR";
    }
}

/* Ternary operator with fixed-point operands */
static sat_fract_t select_fract(sat_fract_t a, sat_fract_t b, int flag) {
    /* Both branches have different range implications */
    return flag ? (a * 2.0r) : (b / 2.0r);
}

/* Main test function with diverse operations */
static void run_fixed_point_tests(int iterations, fract_t init_val) {
    /* Initialize arrays */
    const int ARRAY_SIZE = 100;
    fract_t fract_array[ARRAY_SIZE];
    sat_accum_t accum_array[ARRAY_SIZE];
    
    /* Fill arrays with values that approach saturation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fract_array[i] = init_val + (i * 0.01r);
        accum_array[i] = (sat_accum_t)(i * 0.1k);
    }
    
    /* Test 1: Array accumulation with saturation */
    sat_accum_t total = accumulate_array(fract_array, ARRAY_SIZE);
    printf("Accumulation result: %f\n", (float)total);
    
    /* Test 2: Complex expression evaluation */
    sat_fract_t complex_result = complex_expression(
        0.7r, 0.3r, -0.2r);
    printf("Complex expression: %f (%s)\n", 
           (float)complex_result, classify_fract(complex_result));
    
    /* Test 3: Builtin overflow detection */
    sat_accum_t overflow_res;
    int had_overflow = test_builtin_overflow(0.8k, 0.9k, &overflow_res);
    printf("Overflow test: %d, result: %f\n", had_overflow, (float)overflow_res);
    
    /* Test 4: Loop with evolving ranges */
    sat_fract_t loop_var = 0.5r;
    for (int i = 0; i < iterations; i++) {
        /* Operations that push toward saturation */
        loop_var = add_with_saturation(loop_var, 0.2r);
        loop_var = multiply_saturating(loop_var, 1.1r);
        
        /* Ternary selection affecting range */
        loop_var = select_fract(loop_var, -loop_var, i % 2);
        
        /* Conditional based on current range */
        if (loop_var > 0.95r || loop_var < -0.95r) {
            loop_var = loop_var * 0.5r;
        }
    }
    printf("Loop result: %f\n", (float)loop_var);
    
    /* Test 5: Shift operations */
    fract_t shifted = 0.75r;
    for (int shift = 1; shift <= 4; shift++) {
        shifted = shift_fract(shifted, shift);
        printf("After shift %d: %f\n", shift, (float)shifted);
    }
    
    /* Test 6: Mixed saturated/unsaturated operations */
    fract_t unsaturated = 0.6r;
    sat_fract_t saturated = 0.6r;
    
    /* Assignment that requires range check */
    saturated = unsaturated * 2.0r;  /* Should saturate near 1.0 */
    unsaturated = saturated;         /* Conversion back */
    
    printf("Mixed types - saturated: %f, unsaturated: %f\n",
           (float)saturated, (float)unsaturated);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = 0.0k;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = checksum + (sat_accum_t)fract_array[i];
        checksum = checksum + accum_array[i];
    }
    printf("Final checksum: %f\n", (float)checksum);
}

/* Inline assembly to create hard-to-analyze value flows */
static sat_fract_t asm_fract_operation(sat_fract_t a, sat_fract_t b) {
    sat_fract_t result;
    /* Use asm volatile to obscure value ranges from optimizer */
    asm volatile (
        "/* Fixed-point operation */"
        : "=r"(result)
        : "r"(a), "r"(b)
    );
    return result;
}

int main(int argc, char** argv) {
    /* Use command-line arguments for runtime variability */
    int iterations = 10;
    float init_val_float = 0.5f;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        init_val_float = atof(argv[2]);
    }
    
    /* Convert to fixed-point, ensuring proper range */
    fract_t init_val = (fract_t)init_val_float;
    
    printf("Running fixed-point tests with iterations=%d, init_val=%f\n",
           iterations, (float)init_val);
    
    /* Run the main test suite */
    run_fixed_point_tests(iterations, init_val);
    
    /* Additional test with inline assembly */
    sat_fract_t asm_result = asm_fract_operation(0.7r, 0.3r);
    printf("Assembly operation result: %f\n", (float)asm_result);
    
    return 0;
}
