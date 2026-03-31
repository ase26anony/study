/* fixed-point-test.c
 * Designed to trigger fixed-value.cc lines 264-277
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding with volatile variables */
volatile short _Fract v_sf1 = 0.5r;
volatile short _Fract v_sf2 = 0.8r;
volatile _Accum v_acc1 = 0.5k;
volatile _Accum v_acc2 = 0.7k;
volatile long _Fract v_lf1 = 0.9lr;
volatile long _Fract v_lf2 = 0.95lr;

/* Function to force fixed-point arithmetic with unknown values */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c, int shift) {
    /* Multiple fixed-point operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed-type multiplication */
    _Accum temp2 = temp1 << shift;  /* Left shift - triggers FIXED_LSHIFT_EXPR */
    
    /* Force integer promotion */
    long _Fract temp3 = c * 256;    /* Multiplication with integer constant */
    
    /* Narrowing conversion with potential overflow */
    short _Fract temp4 = (_Fract)temp3;  /* Cast to narrower type */
    
    /* Another shift operation */
    _Accum temp5 = temp2 << 1;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return temp5 + (_Accum)temp4;
}

/* Another function with saturation-prone operations */
static short _Fract saturate_multiply(long _Fract a, long _Fract b) {
    /* Multiplication that could exceed short _Fract range */
    long _Fract product = a * b;
    
    /* Explicit shift to trigger bounds checking */
    product = product << 1;
    
    /* Conversion to narrower type - may trigger saturation logic */
    short _Fract result = (short _Fract)product;
    
    asm volatile ("" : : : "memory");
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Arrays of fixed-point values */
    short _Fract sf_array[10];
    _Accum acc_array[10];
    long _Fract lf_array[10];
    
    /* Initialize arrays with volatile values to prevent constant folding */
    for (int i = 0; i < 10; i++) {
        sf_array[i] = v_sf1 * (short _Fract)i;
        acc_array[i] = v_acc1 * (_Accum)i;
        lf_array[i] = v_lf1 * (long _Fract)i;
    }
    
    _Accum total = 0k;
    
    /* Loop with multiple fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        int shift_amount = i % 4;  /* Variable shift amount */
        
        /* Operation 1: Mixed-type multiplication with shift */
        _Accum val1 = process_fixed_point(
            sf_array[i], 
            acc_array[i], 
            lf_array[i], 
            shift_amount
        );
        
        /* Operation 2: Multiplication that may overflow */
        short _Fract val2 = saturate_multiply(
            lf_array[i] * v_lf2,
            lf_array[(i + 1) % 10] * v_lf1
        );
        
        /* Operation 3: Direct shift on _Accum */
        _Accum val3 = acc_array[i] << (shift_amount + 1);
        
        /* Operation 4: Integer promotion scenario */
        long _Fract val4 = lf_array[i] * 65536;  /* Large integer constant */
        
        /* Narrowing conversion that may trigger saturation */
        short _Fract val5 = (short _Fract)val4;
        
        /* More complex expression with multiple operations */
        _Accum val6 = (val1 * val3) << 2;
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
        
        /* Accumulate results */
        total += val1 + (_Accum)val2 + val3 + (_Accum)val5 + val6;
        
        /* Modify array values for next iteration */
        sf_array[i] = sf_array[i] * v_sf2;
        acc_array[i] = acc_array[i] << 1;
        lf_array[i] = lf_array[i] * 0.99lr;
    }
    
    /* Additional test cases with extreme values */
    unsigned short _Fract usf1 = 0.999ur;  /* Near maximum */
    unsigned short _Fract usf2 = 0.999ur;
    
    /* Multiplication that could overflow unsigned range */
    unsigned short _Fract usf_product = usf1 * usf2;
    
    /* Shift operation on unsigned fixed-point */
    usf_product = usf_product << 1;
    
    /* Convert to signed with potential issues */
    short _Fract converted = (short _Fract)usf_product;
    
    /* Final accumulation */
    total += (_Accum)converted;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %k\n", total);
    
    return 0;
}
