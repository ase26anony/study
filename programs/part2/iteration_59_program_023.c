/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point operations that may overflow */
static volatile short _Fract get_volatile_fract(void) {
    /* Use volatile to prevent constant folding */
    volatile short _Fract v = 0.7r;
    return v;
}

/* Function that performs fixed-point multiplication with potential overflow */
_Accum fixed_mult_with_overflow(_Fract a, _Fract b) {
    /* This multiplication may overflow the intermediate result */
    _Accum result = (_Accum)a * (_Accum)b;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Function with left shift operations on fixed-point types */
short _Fract fixed_shift_operations(short _Fract base, int shift) {
    short _Fract temp = base;
    
    /* Left shift on fixed-point (FIXED_LSHIFT_EXPR) */
    temp = temp << shift;
    
    /* Another memory barrier */
    asm volatile ("" : : : "memory");
    
    return temp;
}

/* Main test function with various fixed-point operations */
void test_fixed_point_operations(int iterations) {
    /* Declare various fixed-point types */
    short _Fract sf1, sf2, sf_result;
    _Accum acc1, acc2, acc_result;
    long _Fract lf1, lf2;
    
    /* Initialize with values that may cause overflow when multiplied */
    sf1 = 0.9r;
    sf2 = 0.95r;
    acc1 = 0.8k;
    acc2 = 0.9k;
    lf1 = 0.99lr;
    lf2 = 0.98lr;
    
    /* Array to store results and prevent optimization */
    short _Fract results[10];
    
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Vary the values slightly each iteration */
        sf1 = sf1 * 0.99r;
        sf2 = sf2 * 1.01r;
        
        /* Operation 1: Multiplication that may overflow intermediate */
        acc_result = (_Accum)sf1 * (_Accum)sf2;
        
        /* Operation 2: Assign to narrower type (potential overflow) */
        sf_result = (short _Fract)acc_result;
        
        /* Operation 3: Left shift on fixed-point */
        sf_result = fixed_shift_operations(sf_result, i % 3);
        
        /* Operation 4: Mixed-type multiplication */
        acc_result = acc_result * (_Accum)(i + 1);
        
        /* Operation 5: Another potential overflow scenario */
        lf1 = lf1 * lf2;
        sf_result = (short _Fract)lf1;  /* Narrowing conversion */
        
        /* Store result to array */
        results[i] = sf_result;
        
        /* Use volatile function to get non-constant values */
        if (i % 2 == 0) {
            sf1 = get_volatile_fract();
        }
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract sum = 0r;
    for (int i = 0; i < iterations && i < 10; i++) {
        sum += results[i];
    }
    
    /* Print something to ensure code isn't optimized away */
    printf("Result: %hd\n", (short)(sum * 1000r));
}

/* Additional test with saturation conversions */
void test_saturation_conversions(void) {
    /* These operations should trigger saturation logic with -fsat-conversion */
    _Accum large_acc = 0.999k;
    short _Fract narrow_sf;
    
    /* Multiple conversions that may require saturation */
    for (int i = 0; i < 5; i++) {
        /* Scale up */
        large_acc = large_acc * 1.1k;
        
        /* Convert to narrower type (may overflow) */
        narrow_sf = (short _Fract)large_acc;
        
        /* Left shift operation */
        narrow_sf = narrow_sf << 1;
        
        /* Another conversion back */
        large_acc = (_Accum)narrow_sf * 0.5k;
        
        asm volatile ("" : : : "memory");
    }
    
    volatile short _Fract output = narrow_sf;
    printf("Saturation test: %hd\n", (short)(output * 1000r));
}

/* Test with array operations */
void test_array_operations(int size) {
    if (size > 100) size = 100;
    
    _Accum accum_array[100];
    short _Fract fract_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        accum_array[i] = (_Accum)i / size;
        fract_array[i] = (short _Fract)(i % 10) / 10r;
    }
    
    /* Perform operations that mix array elements */
    for (int i = 1; i < size; i++) {
        /* Multiplication that may have wide intermediate results */
        _Accum temp = accum_array[i] * (_Accum)fract_array[i-1];
        
        /* Left shift */
        temp = temp << (i % 4);
        
        /* Store back with potential overflow */
        fract_array[i] = (short _Fract)temp;
        
        /* Chain operations */
        accum_array[i] = accum_array[i-1] * temp;
    }
    
    /* Final computation */
    _Accum total = 0k;
    for (int i = 0; i < size; i++) {
        total += accum_array[i] * (_Accum)fract_array[i];
    }
    
    volatile _Accum final_result = total;
    printf("Array test: %ld\n", (long)(final_result * 1000k));
}

int main(int argc, char *argv[]) {
    /* Use argc to make iteration count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 20) iterations = 20;
    
    printf("Testing fixed-point operations (iterations: %d)\n", iterations);
    
    /* Run various tests to trigger different fixed-point code paths */
    test_fixed_point_operations(iterations);
    test_saturation_conversions();
    test_array_operations(iterations * 2);
    
    /* Additional test with immediate values that might trigger specific patterns */
    {
        short _Fract a = 0.5r;
        short _Fract b = 0.8r;
        _Accum c = (_Accum)a * (_Accum)b;
        
        /* Multiple shifts */
        for (int i = 0; i < 3; i++) {
            c = c << 2;
            a = (short _Fract)c;
            c = (_Accum)a * 0.75k;
        }
        
        volatile _Accum v = c;
        printf("Final: %ld\n", (long)(v * 1000k));
    }
    
    return 0;
}
