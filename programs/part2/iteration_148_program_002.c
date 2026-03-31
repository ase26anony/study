/* test_fixed.c - Program to trigger fixed-value.cc range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef _Sat short _Fract ssfract_t;
typedef _Sat long _Accum slaccum_t;

/* Test functions that perform fixed-point operations */
static inline sfract_t add_saturated(sfract_t a, sfract_t b) {
    return a + b;
}

static inline saccum_t multiply_saturated(saccum_t a, saccum_t b) {
    return a * b;
}

static inline accum_t shift_accum(accum_t a, int shift) {
    return a >> shift;
}

static inline sfract_t complex_expr(sfract_t a, sfract_t b, sfract_t c) {
    /* Complex expression requiring range analysis */
    return (a + b) * c;
}

/* Function with conditional based on fixed-point range */
static int check_range(saccum_t val) {
    /* This should trigger the range comparison logic */
    if (val > 0.8k) {
        return 1;
    } else if (val < -0.8k) {
        return -1;
    }
    return 0;
}

/* Loop-based range analysis test */
static saccum_t loop_accumulation(int iterations, fract_t base) {
    saccum_t total = 0.0k;
    for (int i = 0; i < iterations; i++) {
        fract_t increment = base * (fract_t)i;
        total = total + (saccum_t)increment;
        
        /* Conditional that depends on accumulated value */
        if (total > 0.9k) {
            total = 0.9k;  /* Force saturation */
        } else if (total < -0.9k) {
            total = -0.9k;
        }
    }
    return total;
}

/* Array reduction with fixed-point */
static sfract_t array_reduction(sfract_t arr[], int size) {
    sfract_t sum = 0.0r;
    for (int i = 0; i < size; i++) {
        sum = add_saturated(sum, arr[i]);
        
        /* Complex conditional to trigger range analysis */
        if (sum > 0.95r || sum < -0.95r) {
            /* This should trigger the saturation boundary checks */
            sum = (sum > 0) ? 0.99r : -0.99r;
        }
    }
    return sum;
}

/* Mixed-type operations */
static accum_t mixed_operations(fract_t a, accum_t b, int shift) {
    accum_t result = (accum_t)a * b;
    result = shift_accum(result, shift);
    
    /* Ternary operator with fixed-point operands */
    return (result > 0.5k) ? result * 0.5k : result * 2.0k;
}

/* Function using builtins for overflow detection */
static int detect_overflow(saccum_t a, saccum_t b, saccum_t *res) {
    /* Use builtin for overflow detection */
    return __builtin_add_overflow(a, b, res);
}

/* Switch statement based on fixed-point ranges */
static const char* range_category(sfract_t val) {
    switch ((int)(val * 100.0r)) {
        case 0 ... 25: return "low";
        case 26 ... 50: return "medium-low";
        case 51 ... 75: return "medium-high";
        case 76 ... 99: return "high";
        default: return "saturated";
    }
}

/* Main test function */
int main(int argc, char *argv[]) {
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
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize arrays */
    sfract_t sfract_array[array_size];
    fract_t fract_array[array_size];
    
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract_t)((i % 10) * 0.1r);
        sfract_array[i] = (sfract_t)fract_array[i];
    }
    
    /* Test 1: Saturation boundary tests */
    printf("\nTest 1: Saturation boundaries\n");
    sfract_t sat_max = 0.999999r;
    sfract_t sat_min = -0.999999r;
    
    /* These should trigger saturation logic */
    sfract_t test1 = add_saturated(sat_max, 0.1r);
    sfract_t test2 = add_saturated(sat_min, -0.1r);
    printf("  sat_max + 0.1r = %f (as float)\n", (float)test1);
    printf("  sat_min - 0.1r = %f (as float)\n", (float)test2);
    
    /* Test 2: Multiplication near boundaries */
    printf("\nTest 2: Multiplication near boundaries\n");
    saccum_t acc1 = 0.9k;
    saccum_t acc2 = 1.1k;
    saccum_t mult_result = multiply_saturated(acc1, acc2);
    printf("  0.9k * 1.1k = %f (as float)\n", (float)mult_result);
    
    /* Test 3: Loop accumulation */
    printf("\nTest 3: Loop accumulation\n");
    saccum_t loop_result = loop_accumulation(iterations, 0.15r);
    printf("  Loop result = %f (as float)\n", (float)loop_result);
    
    /* Test 4: Array reduction */
    printf("\nTest 4: Array reduction\n");
    sfract_t reduction_result = array_reduction(sfract_array, array_size);
    printf("  Array sum = %f (as float)\n", (float)reduction_result);
    
    /* Test 5: Range checking */
    printf("\nTest 5: Range checking\n");
    saccum_t test_vals[] = {0.5k, 0.9k, -0.9k, 1.5k, -1.5k};
    for (int i = 0; i < 5; i++) {
        int range = check_range(test_vals[i]);
        printf("  check_range(%f) = %d\n", (float)test_vals[i], range);
    }
    
    /* Test 6: Mixed operations */
    printf("\nTest 6: Mixed operations\n");
    accum_t mixed_result = mixed_operations(0.7r, 0.8k, 2);
    printf("  Mixed result = %f (as float)\n", (float)mixed_result);
    
    /* Test 7: Builtin overflow detection */
    printf("\nTest 7: Builtin overflow detection\n");
    saccum_t overflow_res;
    int has_overflow = detect_overflow(0.8k, 0.3k, &overflow_res);
    printf("  Overflow detection (0.8k + 0.3k): overflow=%d, result=%f\n", 
           has_overflow, (float)overflow_res);
    
    /* Test 8: Switch based on ranges */
    printf("\nTest 8: Switch-based categorization\n");
    sfract_t test_values[] = {0.1r, 0.4r, 0.6r, 0.8r, 1.5r};
    for (int i = 0; i < 5; i++) {
        const char* category = range_category(test_values[i]);
        printf("  %f -> %s\n", (float)test_values[i], category);
    }
    
    /* Test 9: Complex expressions with shifts */
    printf("\nTest 9: Complex expressions with shifts\n");
    accum_t shift_test = 0.75k;
    for (int shift = 0; shift < 5; shift++) {
        accum_t shifted = shift_accum(shift_test, shift);
        printf("  0.75k >> %d = %f\n", shift, (float)shifted);
    }
    
    /* Test 10: Nested complex expressions */
    printf("\nTest 10: Nested complex expressions\n");
    sfract_t a = 0.5r;
    sfract_t b = 0.3r;
    sfract_t c = 0.8r;
    sfract_t complex_result = complex_expr(a, b, c);
    printf("  (0.5r + 0.3r) * 0.8r = %f\n", (float)complex_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    float checksum = (float)test1 + (float)test2 + (float)mult_result +
                    (float)loop_result + (float)reduction_result +
                    (float)mixed_result + (float)overflow_res +
                    (float)complex_result;
    printf("  Total checksum = %f\n", checksum);
    
    return 0;
}
