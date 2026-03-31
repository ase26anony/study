/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Force fixed-point arithmetic with saturation checking */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_acc = 0.5k;

/* Function with fixed-point operations that may overflow */
static long _Fract process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;                     /* FIXED_MULT_P */
    _Accum temp2 = temp1 << shift;            /* FIXED_LSHIFT_EXPR - triggers bounds setup */
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Convert with potential overflow */
    long _Fract result = temp2;
    
    /* More operations to create wider intermediate */
    _Accum temp3 = result * 2.0k;
    short _Fract narrow_result = temp3;       /* Narrow conversion - may trigger saturation check */
    
    return narrow_result * 1.5lr;             /* Return as long fract */
}

/* Another function focusing on shift operations */
static _Accum fixed_shift_chain(_Accum base, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Left shift that could overflow - directly triggers FIXED_LSHIFT_EXPR logic */
        result = result << 1;
        
        /* Multiplication to create wide intermediate */
        result = result * 0.75k;
        
        /* Memory barrier to keep operations separate */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Array operations with fixed-point */
static void array_fixed_ops(short _Fract arr[], int size) {
    _Accum accumulator = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Mix with integer promotion */
        accumulator = accumulator + (arr[i] * i);
        
        /* Shift operation */
        if (i % 2 == 0) {
            accumulator = accumulator << 1;
        }
        
        /* Complex multiplication that could overflow */
        short _Fract temp = arr[i] * 0.9r;
        accumulator = accumulator + (temp * 2.0k);
    }
    
    /* Final conversion that might trigger saturation check */
    short _Fract final = accumulator;
    asm volatile ("" : : "r"(final) : "memory");
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize fixed-point arrays */
    short _Fract sf_array[10];
    _Accum acc_array[10];
    
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (i * 0.1r);
        acc_array[i] = (i * 0.2k);
    }
    
    /* Test 1: Fixed-point multiplication with potential overflow */
    printf("Test 1: Fixed-point multiplication chain\n");
    long _Fract total = 0.0lr;
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent compile-time evaluation */
        short _Fract a = volatile_sf + (i * 0.05r);
        _Accum b = volatile_acc + (i * 0.1k);
        
        /* This call should trigger fixed-value arithmetic with bounds checking */
        long _Fract result = process_fixed_point(a, b, i);
        total = total + result;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Test 2: Shift chain operations */
    printf("Test 2: Fixed-point shift chain\n");
    _Accum shift_result = 0.0k;
    
    for (int i = 0; i < iterations; i++) {
        _Accum base = 0.5k + (i * 0.1k);
        shift_result = shift_result + fixed_shift_chain(base, i + 1);
        
        /* Prevent optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Test 3: Array operations */
    printf("Test 3: Array fixed-point operations\n");
    array_fixed_ops(sf_array, iterations + 2);
    
    /* Test 4: Direct overflow scenarios */
    printf("Test 4: Direct overflow tests\n");
    
    /* Near-maximum values that could overflow on multiplication */
    short _Fract near_max = 0.999r;  /* Close to maximum */
    _Accum large_accum = 100.0k;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that could exceed bounds */
        _Accum product = near_max * large_accum;
        
        /* Left shift that could overflow */
        _Accum shifted = product << (i + 1);
        
        /* Convert to narrower type - may trigger saturation check */
        short _Fract narrowed = shifted;
        
        /* Chain operations */
        large_accum = large_accum * 0.9k;
        
        asm volatile ("" : : : "memory");
    }
    
    /* Test 5: Mixed-width operations */
    printf("Test 5: Mixed-width fixed-point\n");
    long _Fract lf1 = 0.999999lr;  /* Very close to 1 */
    long _Fract lf2 = 0.999999lr;
    
    /* Multiplication of two near-1 values stays near 1, but in wider intermediate */
    long _Fract lf_product = lf1 * lf2;
    
    /* Convert to short fract - this conversion may trigger bounds checking */
    short _Fract sf_from_lf = lf_product;
    
    /* Additional shift on the result */
    _Accum acc_from_sf = sf_from_lf;
    _Accum shifted_acc = acc_from_sf << 3;  /* Left shift 3 positions */
    
    /* Final print to ensure side effects */
    printf("Results: total=%ld, shift_result=%ld, sf_from_lf=%d, shifted_acc=%ld\n",
           (long)(total * 1000000), 
           (long)(shift_result * 1000),
           (int)(sf_from_lf * 1000),
           (long)(shifted_acc * 1000));
    
    return 0;
}
