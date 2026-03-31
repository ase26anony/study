/* test_fixed.c - Comprehensive fixed-point test targeting GCC's range analysis */
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

/* Test functions with different fixed-point operations */

/* Function 1: Saturation boundary testing */
static inline sfract_t test_saturation_boundary(sfract_t a, sfract_t b, int shift)
{
    /* Operations that should trigger saturation range checks */
    sfract_t result = a + b;
    
    /* Multiplication near saturation boundaries */
    result = result * 0.999999r;
    
    /* Shift operations that require precise range analysis */
    if (shift > 0) {
        /* This shift will be converted to multiplication internally */
        accum_t temp = (accum_t)result;
        temp = temp << shift;
        result = (sfract_t)(temp >> (shift + 4));
    }
    
    return result;
}

/* Function 2: Complex range propagation through loops */
static accum_t test_loop_range_propagation(int iterations, fract_t base)
{
    accum_t total = 0k;
    fract_t multiplier = 0.5r;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        fract_t current = base * (fract_t)i * multiplier;
        
        /* Conditional that depends on computed range */
        if (current > 0.7r && current < 0.9r) {
            total += (accum_t)current * 2.0k;
        } else if (current <= 0.3r) {
            total += (accum_t)current * 0.5k;
        } else {
            /* Ternary operator with fixed-point operands */
            total += (current > 0.5r) ? (accum_t)current : (accum_t)(current * 0.8r);
        }
        
        /* Update multiplier in a way that creates complex range */
        multiplier = multiplier * 0.99r + 0.01r;
    }
    
    return total;
}

/* Function 3: Array reduction with saturation */
static sfract_t test_array_saturation(sfract_t arr[], int size)
{
    sfract_t sum = 0r;
    sfract_t product = 0.999999r;
    
    for (int i = 0; i < size; i++) {
        /* Operations designed to hit saturation boundaries */
        sum = sum + arr[i];
        
        /* Multiplication that may overflow/saturate */
        if (i % 2 == 0) {
            product = product * arr[i];
        }
        
        /* Complex conditional based on saturation state */
        if (sum > 0.9r || sum < -0.9r) {
            /* Force re-evaluation of range */
            sum = sum * 0.5r;
        }
    }
    
    /* Final operation that requires precise range analysis */
    return (sum + product) * 0.75r;
}

/* Function 4: Mixed-type operations with builtins */
static saccum_t test_builtin_operations(saccum_t a, saccum_t b, saccum_t c)
{
    saccum_t result = 0k;
    
    /* Use builtins for overflow detection */
    saccum_t overflow_check;
    if (__builtin_add_overflow(a, b, &overflow_check)) {
        result = (a > 0k) ? 0.999999999k : -0.999999999k;
    } else {
        result = overflow_check;
    }
    
    /* Multiplication with potential overflow */
    saccum_t mul_result;
    if (__builtin_mul_overflow(result, c, &mul_result)) {
        /* This should trigger the uncovered saturation logic */
        if (result > 0k && c > 0k) {
            mul_result = 0.999999999k;
        } else if (result < 0k && c < 0k) {
            mul_result = 0.999999999k;
        } else {
            mul_result = -0.999999999k;
        }
    }
    
    /* Shift operation requiring range analysis */
    int shift_amount = (int)((result * 10k) + 10k);
    shift_amount = shift_amount % 16;
    
    /* This shift will be converted to multiplication/division */
    if (shift_amount > 0) {
        mul_result = mul_result << shift_amount;
        mul_result = mul_result >> (shift_amount + 2);
    }
    
    return mul_result;
}

/* Function 5: Nested conditionals with fixed-point */
static fract_t test_nested_conditionals(fract_t a, fract_t b, fract_t c, int mode)
{
    fract_t result;
    
    switch (mode % 4) {
        case 0:
            /* Complex comparison chain */
            if (a > 0.8r && b < 0.2r) {
                result = a * b;
            } else if (a + b > 0.9r || a + b < -0.9r) {
                result = (a + b) * 0.5r;
            } else {
                result = a - b;
            }
            break;
            
        case 1:
            /* Nested ternary operators */
            result = (a > 0.5r) ? 
                    ((b > 0.5r) ? a * b : a / (b + 0.1r)) :
                    ((c > 0.5r) ? b * c : c / (a + 0.1r));
            break;
            
        case 2:
            /* Multiple operations in condition */
            result = ((a * 2.0r) > (b * 3.0r)) ? 
                    (a * 1.5r) : (b * 1.2r);
            break;
            
        default:
            /* Complex expression requiring range analysis */
            result = (a + b + c) / 3.0r;
            if (result > 0.7r) {
                result = result * result;
            }
            break;
    }
    
    return result;
}

/* Function 6: Inline assembly to create complex value flow */
static accum_t test_asm_flow(accum_t a, accum_t b)
{
    accum_t result;
    
    /* Use inline assembly to create hard-to-analyze dependencies */
    asm volatile (
        "add %[res], %[a], %[b]\n\t"
        "mul %[res], %[res], %[a]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b)
    );
    
    /* Follow up with operations that require range analysis */
    if (result > 0.5k && result < 0.8k) {
        result = result << 2;
        result = result >> 3;
    } else if (result < -0.5k) {
        result = result * 0.25k;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[])
{
    /* Initialize with command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int array_size = (argc > 2) ? atoi(argv[2]) : 50;
    int seed = (argc > 3) ? atoi(argv[3]) : 42;
    
    srand(seed);
    
    /* Initialize arrays */
    sfract_t sat_array[100];
    fract_t unsat_array[100];
    
    for (int i = 0; i < array_size && i < 100; i++) {
        /* Generate values near saturation boundaries */
        float val = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        sat_array[i] = (sfract_t)val;
        unsat_array[i] = (fract_t)val;
    }
    
    /* Test 1: Saturation boundary operations */
    printf("Test 1: Saturation boundaries\n");
    sfract_t sat_result = 0r;
    for (int i = 0; i < iterations; i++) {
        sfract_t a = (sfract_t)((i % 10) * 0.1r);
        sfract_t b = (sfract_t)(0.9r - (i % 5) * 0.1r);
        sat_result += test_saturation_boundary(a, b, i % 4);
    }
    printf("  Saturation result: %f\n", (float)sat_result);
    
    /* Test 2: Loop range propagation */
    printf("Test 2: Loop range propagation\n");
    accum_t loop_result = test_loop_range_propagation(
        iterations % 50 + 10, 
        (fract_t)(0.3r + (iterations % 10) * 0.05r)
    );
    printf("  Loop result: %f\n", (float)loop_result);
    
    /* Test 3: Array saturation */
    printf("Test 3: Array saturation\n");
    sfract_t array_result = test_array_saturation(sat_array, array_size);
    printf("  Array result: %f\n", (float)array_result);
    
    /* Test 4: Builtin operations */
    printf("Test 4: Builtin operations\n");
    saccum_t builtin_result = test_builtin_operations(
        0.5k,
        (saccum_t)(0.6k + (iterations % 10) * 0.01k),
        0.8k
    );
    printf("  Builtin result: %f\n", (float)builtin_result);
    
    /* Test 5: Nested conditionals */
    printf("Test 5: Nested conditionals\n");
    fract_t cond_result = test_nested_conditionals(
        0.4r,
        0.6r,
        0.8r,
        iterations
    );
    printf("  Conditional result: %f\n", (float)cond_result);
    
    /* Test 6: Assembly flow */
    printf("Test 6: Assembly flow\n");
    accum_t asm_result = test_asm_flow(
        0.3k,
        (accum_t)(0.4k + (iterations % 5) * 0.05k)
    );
    printf("  Assembly result: %f\n", (float)asm_result);
    
    /* Final checksum to prevent dead code elimination */
    accum_t final_checksum = (accum_t)sat_result 
                           + loop_result 
                           + (accum_t)array_result 
                           + builtin_result 
                           + (accum_t)cond_result 
                           + asm_result;
    
    /* Additional complex expression for range analysis */
    for (int i = 0; i < 10; i++) {
        /* This should trigger the specific uncovered code path */
        sfract_t a = (sfract_t)(0.999999r - i * 0.000001r);
        sfract_t b = (sfract_t)(0.000001r * i);
        
        /* Operation designed to hit the max_r/max_s comparison */
        sfract_t temp = a + b;
        if (temp > 0.999999r || temp < -0.999999r) {
            final_checksum += (accum_t)temp * 0.5k;
        }
    }
    
    printf("Final checksum: %f\n", (float)final_checksum);
    
    return (final_checksum > 0k) ? 0 : 1;
}
