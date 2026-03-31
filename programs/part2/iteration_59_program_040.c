/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point operations that should trigger range checking */
static volatile _Accum g_volatile_accum = 0.5k;

short _Fract process_fixed_point(short _Fract a, short _Fract b, int shift) {
    /* Multiple operations that could overflow */
    short _Fract temp1 = a * b;                    /* Multiplication */
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum widened = (_Accum)temp1;
    widened = widened << shift;                    /* This triggers shift logic */
    
    asm volatile ("" : : : "memory");
    
    /* Convert back with potential overflow */
    short _Fract result = (short _Fract)widened;   /* Narrowing conversion */
    
    /* Mix with volatile to prevent constant folding */
    result = result * (short _Fract)g_volatile_accum;
    
    return result;
}

/* Another function using _Fract with saturation context */
long _Fract saturating_multiply(long _Fract x, long _Fract y) {
    /* This multiplication in -fsat-conversion mode should trigger bounds checking */
    long _Fract product = x * y;
    
    /* Force multiple operations */
    product = product << 1;                        /* Left shift */
    product = product << 2;                        /* Another shift */
    
    asm volatile ("" : : : "memory");
    
    /* Narrowing assignment that requires saturation check */
    short _Fract narrowed = (short _Fract)product;
    
    return (_Accum)narrowed * 0.25k;
}

/* Function with array operations in a loop */
void process_fixed_array(short _Fract *arr, int size, int shift) {
    _Accum accumulator = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression that could overflow */
        _Accum temp = (_Accum)arr[i];
        temp = temp << (shift + i);                /* Varying shift amount */
        
        asm volatile ("" : : : "memory");
        
        /* Multiplication that might exceed range */
        temp = temp * (_Accum)(arr[(i + 1) % size]);
        
        /* Assignment to narrower type */
        short _Fract narrowed = (short _Fract)temp;
        
        /* Accumulate with volatile mixing */
        accumulator += (_Accum)narrowed * g_volatile_accum;
        
        /* Store back to prevent dead code elimination */
        arr[i] = narrowed;
    }
    
    /* Use accumulator to prevent optimization */
    if (accumulator > 1.0k) {
        g_volatile_accum = accumulator * 0.5k;
    }
}

/* Main function with various fixed-point operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 + 1 : 5;
    
    /* Initialize arrays with different fixed-point types */
    short _Fract sf_arr[10];
    long _Fract lf_arr[10];
    _Accum accum_arr[10];
    
    /* Initialize with values that could cause overflow when multiplied */
    for (int i = 0; i < 10; i++) {
        sf_arr[i] = (short _Fract)(0.8r + 0.02r * i);
        lf_arr[i] = (long _Fract)(0.9lr - 0.01lr * i);
        accum_arr[i] = (_Accum)(0.7k + 0.03k * i);
    }
    
    /* Update volatile global */
    g_volatile_accum = (_Accum)(argc % 100) / 100.0k;
    
    /* Perform various fixed-point operations in loops */
    for (int iter = 0; iter < iterations; iter++) {
        /* Operation 1: Multiplication with potential overflow */
        short _Fract a = sf_arr[iter % 10];
        short _Fract b = sf_arr[(iter + 1) % 10];
        short _Fract c = process_fixed_point(a, b, iter % 4);
        
        asm volatile ("" : : : "memory");
        
        /* Operation 2: Saturating multiplication */
        long _Fract x = lf_arr[iter % 10];
        long _Fract y = lf_arr[(iter + 5) % 10];
        long _Fract z = saturating_multiply(x, y);
        
        /* Operation 3: Array processing with shifts */
        process_fixed_array(sf_arr, 10, iter % 3);
        
        /* Operation 4: Mixed-type operations */
        _Accum acc_val = accum_arr[iter % 10];
        acc_val = acc_val << (iter % 5);           /* Left shift */
        acc_val = acc_val * (_Accum)c;             /* Multiplication */
        
        /* Narrowing conversion that requires bounds checking */
        short _Fract narrowed = (short _Fract)acc_val;
        
        /* Store results back to arrays */
        sf_arr[iter % 10] = narrowed;
        accum_arr[iter % 10] = acc_val * 0.5k;
        
        /* Update volatile to prevent optimization */
        g_volatile_accum = (_Accum)narrowed;
    }
    
    /* Compute a checksum to ensure all operations have effect */
    _Accum checksum = 0.0k;
    for (int i = 0; i < 10; i++) {
        checksum += (_Accum)sf_arr[i] + accum_arr[i];
    }
    
    /* Print something to prevent dead code elimination */
    printf("Result: %ld\n", (long)(checksum * 1000k));
    
    return 0;
}
