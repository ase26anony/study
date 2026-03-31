/* test_fixed_point.c - Target coverage of fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef short _Fract hfract_t;
typedef _Sat short _Fract shfract_t;

/* Test functions with various fixed-point operations */

/* Function 1: Complex saturated addition with range analysis */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t sum = a + b;
    /* Conditional that depends on range analysis */
    if (sum > 0.9r) {
        return sum + c;
    } else {
        return sum - c;
    }
}

/* Function 2: Multiplication with overflow potential */
static inline saccum_t mult_with_shift(saccum_t x, saccum_t y, int shift) {
    saccum_t prod = x * y;
    /* Shift operation that requires range analysis */
    if (shift > 0) {
        prod = prod >> shift;
    } else if (shift < 0) {
        prod = prod << (-shift);
    }
    return prod;
}

/* Function 3: Division with saturation boundaries */
static inline sfract_t div_sat_boundary(sfract_t num, sfract_t den) {
    /* Division near saturation boundaries */
    sfract_t result = num / den;
    
    /* This comparison should trigger the uncovered range check */
    if (result > 0.95r || result < -0.95r) {
        return result * 0.5r;
    }
    return result;
}

/* Function 4: Loop-based accumulation with range propagation */
static accum_t loop_accumulation(fract_t* arr, int n) {
    accum_t total = 0.0k;
    for (int i = 0; i < n; i++) {
        /* Complex expression requiring range analysis */
        total = total + (accum_t)arr[i] * (0.5k + (i % 2 ? 0.25k : -0.25k));
        
        /* Conditional that depends on accumulated range */
        if (total > 10.0k) {
            total = total * 0.9k;
        } else if (total < -10.0k) {
            total = total * 0.9k;
        }
    }
    return total;
}

/* Function 5: Mixed-type operations triggering conversions */
static sfract_t mixed_type_ops(hfract_t a, sfract_t b, saccum_t c) {
    /* Mix different fixed-point types */
    sfract_t result = (sfract_t)a + b;
    
    /* Use builtins for overflow detection */
    int overflow = 0;
    sfract_t temp;
    
    /* This should trigger range analysis for the multiplication */
    if (__builtin_mul_overflow(result, (sfract_t)0.8r, &temp)) {
        overflow = 1;
    } else {
        result = temp;
    }
    
    /* Ternary operator with fixed-point operands */
    return overflow ? (sfract_t)0.5r : result + (sfract_t)(c * 0.1k);
}

/* Function 6: Array reduction with saturation */
static saccum_t sat_array_reduce(saccum_t* arr, int n) {
    saccum_t sum = 0.0k;
    saccum_t max_val = -1.0k;  /* Start at minimum */
    saccum_t min_val = 1.0k;   /* Start at maximum */
    
    for (int i = 0; i < n; i++) {
        sum = sum + arr[i];
        
        /* Update min/max - these comparisons need range analysis */
        if (arr[i] > max_val) max_val = arr[i];
        if (arr[i] < min_val) min_val = arr[i];
        
        /* Complex conditional that should trigger the uncovered code */
        if (sum > max_val * 2.0k || sum < min_val * 2.0k) {
            sum = sum * 0.5k;
        }
    }
    return sum;
}

/* Function 7: Switch based on fixed-point comparisons */
static fract_t switch_fixed_point(fract_t val) {
    /* Switch where cases depend on fixed-point range */
    switch ((int)(val * 10.0r)) {
        case 0: return val + 0.1r;
        case 1: return val * 2.0r;
        case 2: return val / 2.0r;
        case 3: return val - 0.1r;
        case 4: return val * val;
        case 5: return val / 1.5r;
        case 6: return val + 0.2r;
        case 7: return val * 0.8r;
        case 8: return val - 0.2r;
        case 9: return val / 0.8r;
        default: return 0.5r;
    }
}

/* Function 8: Nested loops with fixed-point induction */
static saccum_t nested_loop_test(int outer, int inner) {
    saccum_t result = 0.0k;
    
    for (int i = 0; i < outer; i++) {
        fract_t inner_acc = 0.0r;
        
        for (int j = 0; j < inner; j++) {
            /* This increment needs range analysis */
            inner_acc = inner_acc + 0.1r;
            
            /* Conditional that depends on the accumulated value */
            if (inner_acc > 0.5r) {
                inner_acc = inner_acc - 0.05r;
            }
        }
        
        result = result + (saccum_t)inner_acc * (i % 2 ? 0.5k : -0.5k);
        
        /* This comparison should trigger the uncovered range check */
        if (result > (saccum_t)1.0k || result < (saccum_t)-1.0k) {
            result = result * 0.9k;
        }
    }
    return result;
}

/* Function 9: Using asm to create hard-to-analyze values */
static sfract_t asm_fixed_point(sfract_t a, sfract_t b) {
    sfract_t result;
    
    /* Inline asm that modifies fixed-point value */
    asm volatile (
        "/* Some dummy asm that touches the value */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Force range analysis after asm */
    if (result > 0.8r) {
        return result * 0.75r;
    }
    return result + 0.1r;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int array_size = 100;
    int loop_count = 10;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size < 10) array_size = 10;
        if (array_size > 1000) array_size = 1000;
    }
    if (argc > 2) {
        loop_count = atoi(argv[2]);
        if (loop_count < 1) loop_count = 1;
        if (loop_count > 100) loop_count = 100;
    }
    
    /* Initialize arrays */
    fract_t* frac_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    saccum_t* accum_array = (saccum_t*)malloc(array_size * sizeof(saccum_t));
    
    if (!frac_array || !accum_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with values that will trigger various range conditions */
    for (int i = 0; i < array_size; i++) {
        /* Values near saturation boundaries */
        frac_array[i] = (fract_t)((i % 10) * 0.1r);
        accum_array[i] = (saccum_t)((i % 20 - 10) * 0.1k);
    }
    
    /* Run test functions */
    sfract_t test1 = sat_add_range(0.8r, 0.3r, 0.1r);
    saccum_t test2 = mult_with_shift(0.7k, 1.5k, 2);
    sfract_t test3 = div_sat_boundary(0.99r, 0.5r);
    accum_t test4 = loop_accumulation(frac_array, array_size);
    sfract_t test5 = mixed_type_ops(0.5hr, 0.6r, 5.0k);
    saccum_t test6 = sat_array_reduce(accum_array, array_size);
    fract_t test7 = switch_fixed_point(0.3r);
    saccum_t test8 = nested_loop_test(loop_count, loop_count / 2);
    sfract_t test9 = asm_fixed_point(0.7r, 0.2r);
    
    /* Complex expression combining all results */
    saccum_t final_result = 
        (saccum_t)test1 * 0.1k +
        test2 * 0.2k +
        (saccum_t)test3 * 0.3k +
        test4 * 0.4k +
        (saccum_t)test5 * 0.5k +
        test6 * 0.6k +
        (saccum_t)test7 * 0.7k +
        test8 * 0.8k +
        (saccum_t)test9 * 0.9k;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", (float)final_result);
    printf("Test1: %f\n", (float)test1);
    printf("Test2: %f\n", (float)test2);
    printf("Test3: %f\n", (float)test3);
    printf("Test4: %f\n", (float)test4);
    printf("Test5: %f\n", (float)test5);
    printf("Test6: %f\n", (float)test6);
    printf("Test7: %f\n", (float)test7);
    printf("Test8: %f\n", (float)test8);
    printf("Test9: %f\n", (float)test9);
    
    /* Cleanup */
    free(frac_array);
    free(accum_array);
    
    return 0;
}
