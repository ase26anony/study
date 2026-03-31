/* test_fixed.c - Comprehensive fixed-point test for GCC coverage */
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

/* Test functions with complex fixed-point operations */

/* Function 1: Saturation boundary testing */
static inline sfract_t test_saturation_boundary(sfract_t a, sfract_t b, int shift)
{
    /* Operations designed to hit saturation boundaries */
    sfract_t result = a + b;
    
    /* Multiplication near saturation limits */
    result = result * 0.999999r;
    
    /* Shift operations that can cause overflow/underflow */
    if (shift > 0) {
        result = result << shift;
    } else if (shift < 0) {
        result = result >> (-shift);
    }
    
    /* Conditional based on range analysis */
    if (result > 0.9r) {
        return 0.999999r;  /* Max saturation */
    } else if (result < -0.9r) {
        return -0.999999r; /* Min saturation */
    }
    
    return result;
}

/* Function 2: Accumulator range propagation */
static inline saccum_t accum_range_propagation(saccum_t base, int iterations)
{
    saccum_t total = 0.0k;
    
    /* Loop with fixed-point induction variable */
    for (saccum_t i = 0.1k; i < base && i < 10.0k; i += 0.1k) {
        /* Complex expression requiring range analysis */
        saccum_t temp = (i * base) / 2.0k;
        
        /* Shift operation */
        temp = temp >> 2;
        
        /* Conditional that depends on range */
        if (temp > 5.0k || temp < -5.0k) {
            total += temp * 0.5k;
        } else {
            total += temp;
        }
        
        /* Early exit based on saturation */
        if (total >= 127.999k || total <= -128.0k) {
            break;
        }
    }
    
    return total;
}

/* Function 3: Mixed-type operations */
static inline accum_t mixed_type_operations(fract_t a, accum_t b, int use_sat)
{
    /* Convert between different fixed-point types */
    accum_t result = (accum_t)a * b;
    
    /* Ternary operator with fixed-point operands */
    result = use_sat ? 
        (result > 50.0k ? 50.0k : (result < -50.0k ? -50.0k : result)) :
        result;
    
    /* Complex shift operation */
    result = result << 1;
    result = result >> 2;
    
    /* Built-in overflow check */
    accum_t check;
    if (__builtin_mul_overflow(result, 2.0k, &check)) {
        return use_sat ? (result > 0 ? 127.999k : -128.0k) : result;
    }
    
    return result;
}

/* Function 4: Array reduction with fixed-point */
static sfract_t array_reduction(const sfract_t* arr, int size)
{
    sfract_t sum = 0.0r;
    sfract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Operations that can saturate */
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Range-dependent conditional */
        if (sum > 0.8r || sum < -0.8r) {
            sum = sum * 0.5r;
        }
        
        /* Check for underflow in product */
        if (product < 0.001r && product > -0.001r && product != 0.0r) {
            product = 0.0r;
        }
    }
    
    /* Final complex expression */
    return (sum + product) / 2.0r;
}

/* Function 5: Switch based on fixed-point comparison */
static int fixed_point_switch(sfract_t value)
{
    /* Switch where cases depend on fixed-point range */
    switch ((int)(value * 100.0r)) {
        case 0 ... 25:
            return 1;
        case 26 ... 50:
            return 2;
        case 51 ... 75:
            return 3;
        case 76 ... 100:
            return 4;
        default:
            /* Should trigger saturation analysis for out-of-range */
            return value > 0 ? 5 : 0;
    }
}

/* Function 6: Nested loops with fixed-point */
static saccum_t nested_loop_test(int outer, int inner)
{
    saccum_t matrix[10][10];
    saccum_t total = 0.0k;
    
    /* Initialize matrix with values that can saturate */
    for (int i = 0; i < 10 && i < outer; i++) {
        for (int j = 0; j < 10 && j < inner; j++) {
            matrix[i][j] = ((saccum_t)i - 5.0k) * ((saccum_t)j - 5.0k) * 0.1k;
        }
    }
    
    /* Perform operations that require range analysis */
    for (int i = 0; i < 10 && i < outer; i++) {
        saccum_t row_sum = 0.0k;
        for (int j = 0; j < 10 && j < inner; j++) {
            row_sum += matrix[i][j];
            
            /* Shift operation within loop */
            matrix[i][j] = matrix[i][j] << 1;
            matrix[i][j] = matrix[i][j] >> 2;
        }
        
        /* Conditional that depends on accumulated range */
        if (row_sum > 10.0k || row_sum < -10.0k) {
            total += row_sum * 0.25k;
        } else {
            total += row_sum;
        }
        
        /* Check for saturation */
        if (total >= 127.999k || total <= -128.0k) {
            break;
        }
    }
    
    return total;
}

/* Function 7: Using builtins with fixed-point */
static void builtin_operations(sfract_t* a, sfract_t* b, sfract_t* result, int count)
{
    for (int i = 0; i < count; i++) {
        /* Use builtin overflow check */
        sfract_t temp;
        if (__builtin_add_overflow(a[i], b[i], &temp)) {
            result[i] = (a[i] > 0) ? 0.999999r : -0.999999r;
        } else {
            result[i] = temp;
        }
        
        /* Additional multiplication with overflow check */
        sfract_t mul_temp;
        if (__builtin_mul_overflow(result[i], 1.5r, &mul_temp)) {
            result[i] = (result[i] > 0) ? 0.999999r : -0.999999r;
        } else {
            result[i] = mul_temp;
        }
    }
}

/* Main test function */
int main(int argc, char** argv)
{
    /* Use command-line arguments for variability */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int use_saturation = argc > 2 ? atoi(argv[2]) : 1;
    int shift_amount = argc > 3 ? atoi(argv[3]) : 2;
    
    /* Initialize fixed-point arrays */
    sfract_t sfract_array[100];
    fract_t fract_array[100];
    saccum_t saccum_array[50];
    
    for (int i = 0; i < 100; i++) {
        sfract_array[i] = ((sfract_t)(i - 50) / 100.0r);
        fract_array[i] = ((fract_t)(i - 50) / 100.0r);
        if (i < 50) {
            saccum_array[i] = ((saccum_t)(i - 25) * 0.5k);
        }
    }
    
    /* Variable to accumulate results */
    accum_t total_accum = 0.0k;
    sfract_t total_fract = 0.0r;
    
    /* Test 1: Saturation boundaries */
    for (int i = 0; i < iterations && i < 100; i++) {
        sfract_t result = test_saturation_boundary(
            sfract_array[i], 
            sfract_array[99 - i], 
            shift_amount + (i % 5) - 2
        );
        total_fract += result;
    }
    
    /* Test 2: Accumulator range propagation */
    for (int i = 0; i < iterations && i < 10; i++) {
        saccum_t base = ((saccum_t)i - 5.0k) * 10.0k;
        saccum_t result = accum_range_propagation(base, iterations % 20);
        total_accum += result;
    }
    
    /* Test 3: Mixed type operations */
    for (int i = 0; i < iterations && i < 100; i++) {
        accum_t result = mixed_type_operations(
            fract_array[i],
            (accum_t)sfract_array[i] * 100.0k,
            use_saturation
        );
        total_accum += result;
    }
    
    /* Test 4: Array reduction */
    sfract_t reduction_result = array_reduction(sfract_array, 
        iterations < 100 ? iterations : 100);
    total_fract += reduction_result;
    
    /* Test 5: Switch statement */
    int switch_counts[6] = {0};
    for (int i = 0; i < iterations && i < 100; i++) {
        int sw = fixed_point_switch(sfract_array[i] + 0.5r);
        if (sw >= 0 && sw <= 5) switch_counts[sw]++;
    }
    
    /* Test 6: Nested loops */
    saccum_t nested_result = nested_loop_test(
        iterations % 10,
        (iterations / 10) % 10
    );
    total_accum += nested_result;
    
    /* Test 7: Builtin operations */
    sfract_t builtin_results[100];
    builtin_operations(sfract_array, fract_array, builtin_results, 
        iterations < 100 ? iterations : 100);
    
    for (int i = 0; i < 100 && i < iterations; i++) {
        total_fract += builtin_results[i];
    }
    
    /* Final checksum calculation to prevent dead code elimination */
    accum_t final_checksum = (accum_t)total_fract * 100.0k + total_accum;
    
    /* Convert to float for printing */
    printf("Final checksum: %f\n", (float)final_checksum);
    printf("Switch distribution: %d %d %d %d %d %d\n",
           switch_counts[0], switch_counts[1], switch_counts[2],
           switch_counts[3], switch_counts[4], switch_counts[5]);
    
    return 0;
}
