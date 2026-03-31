/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Force GCC to process fixed-point arithmetic in middle-end */
/* Use volatile to prevent constant folding */
volatile short _Fract v_sf1 = 0.5r;
volatile short _Fract v_sf2 = 0.8r;
volatile _Accum v_acc1 = 0.5k;
volatile _Accum v_acc2 = 1.2k;
volatile long _Fract v_lf1 = 0.9lr;
volatile long _Fract v_lf2 = 0.95lr;

/* Function with fixed-point operations that may overflow */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c, int shift)
{
    /* Multiple fixed-point operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different types */
    _Accum temp2 = temp1 << shift;  /* FIXED_LSHIFT_EXPR - triggers shift logic */
    
    /* Mix with integer promotion */
    long _Fract temp3 = c * 256;    /* Integer promotion in multiplication */
    
    /* Narrowing conversion that requires range checking */
    short _Fract temp4 = (_Fract)temp3;  /* Potential overflow */
    
    /* Another multiplication that could saturate */
    _Accum result = temp2 * (_Accum)temp4;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Another function focusing on saturation scenarios */
static short _Fract saturating_multiply(short _Fract a, short _Fract b)
{
    /* Direct multiplication that might need saturation checking */
    short _Fract result = a * b;
    
    /* Additional shift to trigger more logic */
    _Accum widened = (_Accum)result << 3;
    
    /* Narrow back with potential overflow */
    result = (short _Fract)widened;
    
    asm volatile ("" : : : "memory");
    return result;
}

int main(int argc, char *argv[])
{
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 10) iterations = 10;
    
    /* Arrays of fixed-point values */
    short _Fract sf_array[10];
    _Accum acc_array[10];
    long _Fract lf_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
        acc_array[i] = (_Accum)(i * 0.2k);
        lf_array[9 - i] = (long _Fract)((9 - i) * 0.15lr);
    }
    
    _Accum total_acc = 0k;
    short _Fract total_sf = 0r;
    
    /* Loop with varying operations to prevent compile-time evaluation */
    for (int i = 0; i < iterations; i++) {
        /* Read volatile values to get unknown inputs */
        short _Fract a = v_sf1;
        _Accum b = v_acc1;
        long _Fract c = v_lf1;
        
        /* Vary shift amount */
        int shift = i % 4;
        
        /* Call function with fixed-point operations */
        _Accum result1 = process_fixed_point(a, b, c, shift);
        
        /* Array-based operations */
        short _Fract sf_result = sf_array[i] * sf_array[9 - i];
        
        /* Shift operation on _Accum - directly triggers FIXED_LSHIFT_EXPR */
        _Accum shifted = acc_array[i] << (shift + 1);
        
        /* Narrowing conversion that may overflow */
        short _Fract narrowed = (short _Fract)lf_array[i];
        
        /* Another multiplication with potential overflow */
        _Accum mult_result = shifted * (_Accum)narrowed;
        
        /* Call saturation function */
        short _Fract sat_result = saturating_multiply(sf_result, narrowed);
        
        /* Accumulate results */
        total_acc += result1 + mult_result;
        total_sf += sf_result + sat_result;
        
        /* Modify volatile values slightly */
        if (i % 2 == 0) {
            asm volatile ("" : "+m" (v_sf1), "+m" (v_acc1));
        }
    }
    
    /* Additional test cases with edge values */
    /* These values are more likely to cause overflow */
    short _Fract edge1 = 0.99r;
    short _Fract edge2 = 0.98r;
    
    /* Multiplication that could overflow short _Fract range */
    for (int i = 0; i < 3; i++) {
        short _Fract product = edge1 * edge2;
        _Accum widened_product = (_Accum)product << 4;  /* Significant shift */
        
        /* Try to assign back to narrower type */
        short _Fract narrowed_again = (short _Fract)widened_product;
        
        total_sf += product + narrowed_again;
        
        /* Increase edge values */
        edge1 = (short _Fract)(edge1 * 1.1r);
        edge2 = (short _Fract)(edge2 * 1.05r);
    }
    
    /* Test with unsigned fixed-point types as well */
    unsigned short _Fract usf1 = 0.8ur;
    unsigned short _Fract usf2 = 0.9ur;
    
    for (int i = 0; i < 2; i++) {
        unsigned short _Fract usf_result = usf1 * usf2;
        unsigned _Accum uacc_result = (unsigned _Accum)usf_result << 2;
        
        /* Convert back with potential issues */
        usf_result = (unsigned short _Fract)uacc_result;
        
        asm volatile ("" : : : "memory");
    }
    
    /* Print something to prevent dead code elimination */
    printf("Result: %f %f\n", (double)total_acc, (double)total_sf);
    
    return 0;
}
