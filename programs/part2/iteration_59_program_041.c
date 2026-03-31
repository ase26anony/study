/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract vf1 = 0.5r;
volatile short _Fract vf2 = 0.8r;
volatile _Accum va1 = 0.5k;
volatile _Accum va2 = 1.2k;

/* Function with fixed-point parameters to force analysis */
long _Fract process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed-type multiplication */
    _Accum temp2 = temp1 << shift;  /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Force integer promotion */
    long _Fract result = temp2 * 256;  /* Integer promotion */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Another function with saturation-prone operations */
short _Fract saturating_multiply(long _Fract a, long _Fract b) {
    /* This multiplication could overflow short _Fract range */
    long _Fract wide_result = a * b;
    
    /* Assignment to narrower type may trigger saturation check */
    short _Fract result = wide_result;
    
    asm volatile ("" : : : "memory");
    return result;
}

/* Loop-based fixed-point processing */
void process_fixed_array(int iterations) {
    /* Array of fixed-point values */
    _Accum arr[10];
    short _Fract results[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        arr[i] = (i * 0.1k) + 0.5k;
    }
    
    /* Perform operations in loop to prevent compile-time evaluation */
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < 9; i++) {
            /* Multiplication that could overflow */
            _Accum prod = arr[i] * arr[i + 1];
            
            /* Left shift operation - triggers FIXED_LSHIFT_EXPR */
            _Accum shifted = prod << (iter % 3);
            
            /* Convert to narrower type, potentially requiring saturation */
            results[i] = shifted;
            
            /* Update array with volatile-like behavior */
            arr[i] = shifted * 0.9k;
        }
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract sum = 0r;
    for (int i = 0; i < 9; i++) {
        sum += results[i];
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Test 1: Mixed-type operations with volatile operands */
    short _Fract sf1 = vf1;
    short _Fract sf2 = vf2;
    _Accum acc1 = va1;
    _Accum acc2 = va2;
    
    /* These multiplications may overflow */
    short _Fract result1 = sf1 * sf2;
    _Accum result2 = acc1 * acc2;
    
    /* Left shift operations - directly trigger FIXED_LSHIFT_EXPR */
    _Accum shifted1 = result2 << 2;
    _Accum shifted2 = acc1 << 3;
    
    /* Test 2: Function calls with fixed-point arithmetic */
    long _Fract lf1 = 0.9lr;
    long _Fract lf2 = 0.95lr;
    
    /* This may overflow when converting to short _Fract */
    short _Fract narrow_result = saturating_multiply(lf1, lf2);
    
    /* Test 3: Process with shifting */
    long _Fract processed = process_fixed_point(sf1, acc1, iterations % 4);
    
    /* Test 4: Array-based processing with loops */
    process_fixed_array(iterations);
    
    /* Test 5: Additional overflow-prone scenarios */
    /* Unsigned fixed-point types */
    unsigned short _Fract usf1 = 0.8ur;
    unsigned short _Fract usf2 = 0.9ur;
    unsigned short _Fract usf_result = usf1 * usf2;
    
    /* Left shift on unsigned */
    unsigned _Accum uacc = 0.5uk;
    uacc = uacc << 1;
    
    /* Mixed signed/unsigned */
    _Accum mixed = usf_result * acc1;
    mixed = mixed << 2;
    
    /* Chain of operations */
    _Accum chain = 0.25k;
    for (int i = 0; i < 3; i++) {
        chain = chain * chain;      /* Square - could overflow */
        chain = chain << 1;         /* Shift - FIXED_LSHIFT_EXPR */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent optimization */
    volatile _Accum final_result = shifted1 + shifted2 + mixed + chain;
    
    printf("Result: %ld\n", (long)(final_result * 1000k));
    
    return 0;
}
