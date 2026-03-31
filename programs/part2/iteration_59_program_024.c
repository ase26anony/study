/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Function with fixed-point operations that should trigger range checking */
static accum_t process_fixed_point(sfract_t a, sfract_t b, int shift) {
    /* These volatile variables prevent constant folding */
    volatile sfract_t v_a = a;
    volatile sfract_t v_b = b;
    volatile int v_shift = shift;
    
    /* Multiplication that may overflow short _Fract range */
    sfract_t mul_result;
    {
        /* Force separate evaluation */
        MEMORY_BARRIER();
        sfract_t tmp1 = v_a;
        sfract_t tmp2 = v_b;
        mul_result = tmp1 * tmp2;  /* FIXED_MULT_P */
        MEMORY_BARRIER();
    }
    
    /* Convert to wider type for shift operation */
    accum_t widened = (accum_t)mul_result;
    
    /* Left shift operation - FIXED_LSHIFT_EXPR */
    accum_t shifted;
    {
        MEMORY_BARRIER();
        accum_t tmp = widened;
        shifted = tmp << v_shift;  /* This should trigger shift logic with bounds checking */
        MEMORY_BARRIER();
    }
    
    /* Another multiplication with promotion */
    accum_t final;
    {
        MEMORY_BARRIER();
        /* Mix with integer to force promotion */
        long int_promotion = 3L;
        accum_t tmp = shifted;
        final = tmp * (accum_t)int_promotion;  /* Wider intermediate */
        MEMORY_BARRIER();
    }
    
    return final;
}

/* Function that creates overflow scenarios */
static void test_overflow_scenarios(int iterations) {
    /* Array of fixed-point values with varying magnitudes */
    sfract_t sf_array[] = {0.1r, 0.5r, 0.8r, 0.9r, 0.95r, 0.99r};
    int sf_count = sizeof(sf_array) / sizeof(sf_array[0]);
    
    /* Volatile to prevent compile-time computation */
    volatile int vol_iter = iterations;
    
    accum_t total = 0.0k;
    
    for (int i = 0; i < vol_iter && i < 10; i++) {
        /* Use different indices to create varying values */
        int idx1 = i % sf_count;
        int idx2 = (i + 1) % sf_count;
        int shift = (i % 4) + 1;  /* Shift values 1-4 */
        
        /* Process with potential overflow */
        accum_t result = process_fixed_point(sf_array[idx1], sf_array[idx2], shift);
        
        /* Assign to narrower type to force range check */
        {
            MEMORY_BARRIER();
            saccum_t narrowed;
            /* This assignment may require saturation checking */
            narrowed = (saccum_t)result;
            total += (accum_t)narrowed;
            MEMORY_BARRIER();
        }
        
        /* Additional shift on accumulated value */
        if (i % 2 == 0) {
            MEMORY_BARRIER();
            accum_t tmp = total;
            total = tmp << 1;  /* Another FIXED_LSHIFT_EXPR */
            MEMORY_BARRIER();
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile accum_t sink = total;
    (void)sink;
}

/* Test with _Accum types specifically */
static void test_accum_operations(void) {
    accum_t acc1 = 0.5k;
    accum_t acc2 = 0.7k;
    accum_t acc3 = 0.9k;
    
    /* Chain of operations that may overflow */
    for (int i = 0; i < 5; i++) {
        MEMORY_BARRIER();
        volatile int shift = i;
        
        /* Multiplication then shift */
        accum_t mul = acc1 * acc2;  /* FIXED_MULT_P */
        accum_t shifted = mul << shift;  /* FIXED_LSHIFT_EXPR */
        
        /* Another multiplication with different operand */
        accum_t final = shifted * acc3;
        
        /* Force assignment to check bounds */
        saccum_t narrowed;
        MEMORY_BARRIER();
        narrowed = (saccum_t)final;  /* May trigger saturation check */
        MEMORY_BARRIER();
        
        /* Modify values for next iteration */
        acc1 = acc1 * 0.8k;
        acc2 = acc2 * 0.9k;
    }
}

/* Mixed fixed-point types */
static void test_mixed_types(int seed) {
    /* Use seed to create non-constant values */
    volatile int v_seed = seed;
    
    fract_t f1 = (fract_t)(v_seed % 100) / 100.0r;
    laccum_t la1 = (laccum_t)(v_seed % 200) / 200.0lk;
    
    /* Operations between different fixed-point types */
    for (int i = 0; i < 3; i++) {
        MEMORY_BARRIER();
        
        /* Multiplication with type conversion */
        laccum_t intermediate = (laccum_t)f1 * la1;
        
        /* Left shift with wide type */
        laccum_t shifted = intermediate << (i + 2);
        
        /* Convert back to narrower type - may need bounds check */
        accum_t converted;
        MEMORY_BARRIER();
        converted = (accum_t)shifted;
        MEMORY_BARRIER();
        
        /* Update values */
        f1 = f1 * 0.75r;
        la1 = la1 * 0.85lk;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make iterations non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 1) iterations = 1;
    if (iterations > 20) iterations = 20;
    
    printf("Testing fixed-point overflow scenarios with %d iterations\n", iterations);
    
    /* Test various overflow scenarios */
    test_overflow_scenarios(iterations);
    
    /* Test _Accum specific operations */
    test_accum_operations();
    
    /* Test mixed types */
    test_mixed_types(iterations);
    
    /* Additional test with array operations */
    {
        #define ARRAY_SIZE 8
        sfract_t sf_arr[ARRAY_SIZE];
        saccum_t sa_arr[ARRAY_SIZE];
        
        /* Initialize arrays */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sf_arr[i] = (sfract_t)((i + 1) * 0.1r);
        }
        
        /* Perform operations that may overflow */
        for (int i = 0; i < ARRAY_SIZE - 1; i++) {
            MEMORY_BARRIER();
            /* Multiplication in loop */
            sfract_t prod = sf_arr[i] * sf_arr[i + 1];
            
            /* Shift operation */
            accum_t widened = (accum_t)prod;
            accum_t shifted = widened << (i % 3 + 1);
            
            /* Store in narrower type array */
            MEMORY_BARRIER();
            sa_arr[i] = (saccum_t)shifted;  /* Potential overflow check */
            MEMORY_BARRIER();
        }
        
        /* Use array values */
        volatile saccum_t sum = 0.0k;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += sa_arr[i];
        }
        (void)sum;
    }
    
    printf("Fixed-point tests completed\n");
    return 0;
}
