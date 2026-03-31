/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8lr;

/* Function with fixed-point parameters to force range checking */
static short _Fract process_fixed(short _Fract a, short _Fract b, int shift) {
    /* Multiplication that may overflow when assigned to narrower type */
    short _Fract result;
    
    /* Force intermediate calculation with potential overflow */
    long _Fract temp = (long _Fract)a * (long _Fract)b;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum acc_val = (_Accum)temp;
    acc_val = acc_val << shift;
    
    /* Another memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Assignment to narrower type - may trigger saturation check */
    result = (short _Fract)acc_val;
    
    return result;
}

/* Function with accum types and shifting */
static _Accum process_accum(_Accum a, _Accum b, int shift1, int shift2) {
    _Accum result = a;
    
    /* Multiple shift operations */
    result = result << shift1;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Multiplication with potential overflow */
    result = result * b;
    
    /* Another shift */
    result = result << shift2;
    
    return result;
}

/* Process array of fixed-point values */
static void process_array(short _Fract *arr, int size, int shift) {
    for (int i = 0; i < size - 1; i++) {
        /* Multiplication that may exceed range */
        long _Fract temp = (long _Fract)arr[i] * (long _Fract)arr[i + 1];
        
        /* Left shift on fixed-point */
        _Accum shifted = (_Accum)temp << shift;
        
        /* Assignment back with potential overflow */
        arr[i] = (short _Fract)shifted;
        
        /* Memory barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize various fixed-point arrays */
    short _Fract sf_arr[10];
    _Accum accum_arr[10];
    long _Fract lf_arr[10];
    
    /* Initialize with values that may cause overflow when multiplied */
    for (int i = 0; i < 10; i++) {
        /* Use volatile source to prevent compile-time computation */
        sf_arr[i] = (short _Fract)(volatile_source * i);
        accum_arr[i] = (_Accum)(volatile_source * i * 2);
        lf_arr[i] = (long _Fract)(volatile_source * i * 4);
    }
    
    /* Mix of operations in loops to prevent optimization */
    short _Fract total_sf = 0.0r;
    _Accum total_accum = 0.0k;
    
    for (int iter = 0; iter < iterations; iter++) {
        int shift = iter % 4;  /* Variable shift amount */
        
        /* Process array with shifting */
        process_array(sf_arr, 10, shift);
        
        /* Individual operations with function calls */
        for (int i = 0; i < 9; i++) {
            /* Call function that does multiplication and shifting */
            short _Fract r = process_fixed(sf_arr[i], sf_arr[i + 1], shift);
            
            /* Accumulate result */
            total_sf += r;
            
            /* Memory barrier */
            if (i % 4 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Process accum types */
        for (int i = 0; i < 9; i++) {
            _Accum r = process_accum(accum_arr[i], accum_arr[i + 1], 
                                    shift, (shift + 1) % 4);
            
            /* Convert and add to total */
            total_accum += r;
        }
        
        /* Mix with integer promotions */
        for (int i = 0; i < 10; i++) {
            /* Fixed-point multiplied by integer constant */
            long _Fract temp = lf_arr[i] * 3;
            
            /* Left shift operation */
            temp = temp << (shift + 1);
            
            /* Assign back with potential overflow */
            lf_arr[i] = temp;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result sf: %f\n", (double)total_sf);
    printf("Result accum: %f\n", (double)total_accum);
    
    /* Final mixed operation that could trigger the specific condition */
    {
        /* Create a scenario with maximum values */
        unsigned short _Fract max_usf = 0x1.0r - 0x0.0001r;
        unsigned short _Fract usf2 = 0x0.9999r;
        
        /* Multiplication that could overflow the unsigned range */
        unsigned long _Fract temp_ul = (unsigned long _Fract)max_usf * 
                                      (unsigned long _Fract)usf2;
        
        /* Left shift to potentially exceed bounds */
        temp_ul = temp_ul << 1;
        
        /* Try to assign to narrower type - may trigger saturation */
        unsigned short _Fract result_usf = (unsigned short _Fract)temp_ul;
        
        printf("Unsigned result: %f\n", (double)result_usf);
    }
    
    return 0;
}
