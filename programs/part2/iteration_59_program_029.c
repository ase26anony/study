/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract v_sf1 = 0.7r;
volatile short _Fract v_sf2 = 0.8r;
volatile _Accum v_acc1 = 0.5k;
volatile _Accum v_acc2 = 1.2k;
volatile long _Fract v_lf1 = 0.9lr;

/* Function with fixed-point parameters to create complex expressions */
_Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed-type multiplication */
    _Accum temp2 = temp1 << shift;  /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Force wider intermediate calculation */
    long _Fract wide_temp = c * c;  /* Square could overflow */
    
    /* Convert with potential overflow */
    _Accum temp3 = wide_temp;       /* Conversion from long _Fract to _Accum */
    
    /* Combine results with another multiplication */
    _Accum result = temp2 * temp3;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Another function focusing on saturation scenarios */
short _Fract saturate_example(_Accum large_val, short _Fract narrow_val) {
    /* Operation that likely exceeds short _Fract range */
    _Accum intermediate = large_val * large_val;
    
    /* Left shift to potentially overflow */
    intermediate = intermediate << 1;
    
    /* Convert to narrower type - should trigger saturation check */
    short _Fract result = intermediate * narrow_val;
    
    asm volatile ("" : : : "memory");
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 5 + 1 : 3;
    
    /* Arrays of fixed-point values */
    short _Fract sf_array[5] = {0.1r, 0.5r, 0.7r, 0.9r, 0.99r};
    _Accum acc_array[5] = {0.1k, 0.5k, 1.0k, 1.5k, 2.0k};
    long _Fract lf_array[5] = {0.1lr, 0.5lr, 0.8lr, 0.9lr, 0.99lr};
    
    /* Results accumulator */
    _Accum total_result = 0.0k;
    
    printf("Starting fixed-point operations (iterations: %d)\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile variables to prevent compile-time evaluation */
        short _Fract sf1 = v_sf1;
        short _Fract sf2 = v_sf2;
        _Accum acc1 = v_acc1;
        _Accum acc2 = v_acc2;
        long _Fract lf1 = v_lf1;
        
        /* Vary shift amount based on loop index */
        int shift_amount = i % 4 + 1;
        
        /* Test case 1: Complex fixed-point expression */
        _Accum result1 = process_fixed_point(sf1, acc1, lf1, shift_amount);
        
        /* Test case 2: Multiplication with potential overflow */
        _Accum mult_result = acc1 * acc2;
        mult_result = mult_result << shift_amount;  /* FIXED_LSHIFT_EXPR */
        
        /* Test case 3: Narrowing conversion with saturation */
        short _Fract narrow_result = saturate_example(acc2, sf_array[i % 5]);
        
        /* Test case 4: Array operations with mixed types */
        for (int j = 0; j < 3; j++) {
            /* Operations that create wide intermediate results */
            long _Fract temp = lf_array[j] * lf_array[(j + 1) % 5];
            _Accum converted = temp;  /* Conversion */
            converted = converted << (j + 1);  /* Left shift */
            
            /* Multiply with array element */
            total_result += converted * acc_array[j];
        }
        
        /* Combine results with volatile memory barrier */
        asm volatile ("" : : : "memory");
        
        total_result += result1 + mult_result + narrow_result;
        
        /* Modify volatile variables slightly */
        v_sf1 = sf_array[i % 5];
        v_acc1 = acc_array[(i + 1) % 5];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %ld (scaled by 2^16)\n", (long)(total_result * 65536));
    
    /* Additional test: Direct overflow scenario */
    {
        /* Create values near maximum */
        _Accum near_max = 0.999999k;  /* Very close to 1.0 */
        _Accum near_max2 = 0.999999k;
        
        /* Multiplication that could overflow the integer part */
        _Accum product = near_max * near_max2;
        
        /* Left shift to force overflow checking */
        product = product << 2;
        
        /* Convert to narrower type */
        short _Fract narrowed = product;
        
        printf("Overflow test result: %ld\n", (long)(narrowed * 32768));
    }
    
    return 0;
}
