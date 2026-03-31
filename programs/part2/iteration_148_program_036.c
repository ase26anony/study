/* test_fixed_point.c - Program to trigger fixed-point range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract slfract_t;
typedef _Sat short _Accum ssaccum_t;

/* Test functions with various fixed-point operations */

/* Function 1: Complex fixed-point arithmetic with saturation */
static inline sfract_t sat_add_multiply(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t sum = a + b;
    sfract_t product = sum * c;
    
    /* Force conditional based on range analysis */
    if (product > 0.8r) {
        return product >> 2;  /* Shift may cause underflow */
    } else {
        return product << 1;  /* Shift may cause overflow */
    }
}

/* Function 2: Accumulator with loop-based range propagation */
static saccum_t accumulate_range(int iterations, fract_t base) {
    saccum_t total = 0.0k;
    fract_t increment = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        total = total + (increment * 2.0k) - (0.5k >> i);
        
        /* Update increment with saturation check */
        if (increment < 0.9r) {
            increment = increment + 0.1r;
        } else {
            increment = 0.1r;  /* Reset to cause range variation */
        }
    }
    
    return total;
}

/* Function 3: Fixed-point array reduction with overflow checks */
static sfract_t array_reduction(const sfract_t* arr, int size) {
    sfract_t sum = 0.0r;
    sfract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Use builtins for overflow detection */
        sfract_t temp;
        if (__builtin_add_overflow(sum, arr[i], &temp)) {
            /* Force saturation boundary */
            sum = 0.999999r;  /* Max representable */
        } else {
            sum = temp;
        }
        
        /* Multiplication with potential overflow */
        product = product * arr[i];
        
        /* Shift operation requiring range analysis */
        if (i % 2 == 0) {
            product = product >> 1;
        } else {
            product = product << 1;
        }
    }
    
    /* Ternary with fixed-point operands */
    return (sum > 0.5r) ? sum : product;
}

/* Function 4: Nested fixed-point operations with control flow */
static fract_t nested_operations(fract_t a, fract_t b, int selector) {
    switch (selector) {
        case 0:
            return a + b;
        case 1:
            return a - b;
        case 2:
            return a * b;
        case 3:
            /* Division requires careful range analysis */
            return (b != 0.0r) ? (a / b) : 0.0r;
        case 4:
            /* Complex shift expression */
            return (a << 2) + (b >> 2);
        default:
            return a;
    }
}

/* Function 5: Mixed saturation types */
static void mixed_saturation_ops(ssaccum_t* result, slfract_t a, fract_t b) {
    /* Mix saturated and unsaturated types */
    sfract_t sat_b = b;  /* Conversion may saturate */
    *result = a * sat_b;
    
    /* Force overflow/underflow boundaries */
    if (*result > 0.9K) {
        *result = *result + 0.2K;  /* Should saturate */
    } else if (*result < -0.9K) {
        *result = *result - 0.2K;  /* Should saturate */
    }
}

/* Function 6: Assembly with fixed-point constraints */
static sfract_t asm_fixed_point(sfract_t x, sfract_t y) {
    sfract_t result;
    
    /* Use asm to create hard-to-analyze value flow */
    asm volatile (
        "add %[res], %[x], %[y]\n\t"
        : [res] "=r" (result)
        : [x] "r" (x), [y] "r" (y)
    );
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Initialize with command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 8;
    fract_t base_value = (argc > 3) ? (fract_t)atof(argv[3]) : 0.1r;
    
    if (iterations <= 0) iterations = 10;
    if (array_size <= 0) array_size = 8;
    
    printf("Testing fixed-point range analysis\n");
    printf("Iterations: %d, Array size: %d, Base: %f\n", 
           iterations, array_size, (float)base_value);
    
    /* Test 1: Saturation boundaries */
    sfract_t sat_max = 0.999999r;
    sfract_t sat_min = -0.999999r;
    
    printf("\nTest 1 - Saturation boundaries:\n");
    sfract_t test1 = sat_max + 0.1r;  /* Should saturate */
    printf("  Max + 0.1 = %f\n", (float)test1);
    
    test1 = sat_min - 0.1r;  /* Should saturate */
    printf("  Min - 0.1 = %f\n", (float)test1);
    
    /* Test 2: Complex arithmetic with function calls */
    printf("\nTest 2 - Complex arithmetic:\n");
    for (int i = 0; i < 5; i++) {
        sfract_t a = (i * 0.2r) - 0.5r;
        sfract_t b = 0.3r;
        sfract_t c = 0.7r;
        
        sfract_t result = sat_add_multiply(a, b, c);
        printf("  sat_add_multiply(%.2f, %.2f, %.2f) = %.6f\n",
               (float)a, (float)b, (float)c, (float)result);
    }
    
    /* Test 3: Accumulator with loop */
    printf("\nTest 3 - Accumulator with loop:\n");
    saccum_t accum_result = accumulate_range(iterations, base_value);
    printf("  accumulate_range(%d, %.2f) = %.6f\n",
           iterations, (float)base_value, (float)accum_result);
    
    /* Test 4: Array operations */
    printf("\nTest 4 - Array reduction:\n");
    sfract_t* arr = (sfract_t*)malloc(array_size * sizeof(sfract_t));
    for (int i = 0; i < array_size; i++) {
        arr[i] = (i % 2 == 0) ? 0.8r : -0.6r;
    }
    
    sfract_t array_result = array_reduction(arr, array_size);
    printf("  array_reduction(size=%d) = %.6f\n", array_size, (float)array_result);
    
    /* Test 5: Nested operations with switch */
    printf("\nTest 5 - Nested operations:\n");
    for (int sel = 0; sel < 5; sel++) {
        fract_t a = 0.7r;
        fract_t b = (sel == 3) ? 0.0r : 0.3r;  /* Test division by zero case */
        
        fract_t nested_result = nested_operations(a, b, sel);
        printf("  nested_operations(%.2f, %.2f, %d) = %.6f\n",
               (float)a, (float)b, sel, (float)nested_result);
    }
    
    /* Test 6: Mixed saturation types */
    printf("\nTest 6 - Mixed saturation types:\n");
    ssaccum_t mixed_result;
    slfract_t long_fract = 0.999999lr;
    fract_t regular_fract = 0.5r;
    
    mixed_saturation_ops(&mixed_result, long_fract, regular_fract);
    printf("  mixed_saturation_ops(%.6f, %.6f) = %.6f\n",
           (float)long_fract, (float)regular_fract, (float)mixed_result);
    
    /* Test 7: Assembly operations */
    printf("\nTest 7 - Assembly operations:\n");
    sfract_t asm_result = asm_fixed_point(0.4r, 0.3r);
    printf("  asm_fixed_point(0.4, 0.3) = %.6f\n", (float)asm_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    fract_t checksum = 0.0r;
    
    /* Complex expression involving all test results */
    checksum = (fract_t)test1 + 
               (fract_t)array_result + 
               (fract_t)accum_result + 
               (fract_t)mixed_result +
               (fract_t)asm_result;
    
    /* Additional operations to trigger range analysis */
    for (int i = 0; i < iterations; i++) {
        checksum = checksum * 1.01r;
        checksum = checksum - 0.005r;
        
        /* Conditional that depends on range analysis */
        if (checksum > 0.5r) {
            checksum = checksum >> 1;
        } else {
            checksum = checksum << 1;
        }
    }
    
    printf("  Final checksum: %.6f\n", (float)checksum);
    
    /* Cleanup */
    free(arr);
    
    return 0;
}
