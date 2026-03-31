/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point operations that should trigger range checking */
static volatile short _Fract global_volatile_fract = 0.5r;

void fixed_point_operations(short _Fract a, short _Fract b, _Accum c, int shift) {
    /* Multiple fixed-point operations that may overflow */
    short _Fract r1 = a * b;                    /* FIXED_MULT_P */
    _Accum r2 = c << shift;                     /* FIXED_LSHIFT_EXPR - should trigger shift logic */
    
    /* Mix with integer promotion */
    long _Fract r3 = a * 256;                   /* Integer promotion */
    
    /* Narrower assignment - potential overflow */
    short _Fract r4 = (_Accum)a * (_Accum)b;    /* Wider intermediate */
    
    /* Use volatile to prevent optimization */
    volatile short _Fract vr1 = r1;
    volatile _Accum vr2 = r2;
    
    /* Memory barrier to keep operations separate */
    asm volatile ("" : : : "memory");
    
    /* Print to prevent dead code elimination */
    printf("Results: %hd %hd %hd %hd\n", 
           (short)(vr1 * 256), 
           (short)(vr2 * 256),
           (short)(r3 * 256),
           (short)(r4 * 256));
}

/* Another function with saturation-prone operations */
void saturation_test(long _Fract a, long _Fract b) {
    /* Operations that could saturate */
    short _Fract narrow1 = a * b;               /* Potential overflow to narrower type */
    _Accum accum1 = a * 4k;                     /* Multiplication with _Accum */
    
    /* Left shift that could overflow */
    _Accum shifted = accum1 << 3;               /* FIXED_LSHIFT_EXPR */
    
    /* Chain of operations */
    short _Fract final = (a * b) * (a * b);     /* Multiple multiplications */
    
    volatile short _Fract vfinal = final;
    asm volatile ("" : : : "memory");
    
    printf("Saturation test: %hd %hd\n", 
           (short)(narrow1 * 256),
           (short)(vfinal * 256));
}

/* Main function with loops and arrays */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Array of fixed-point values */
    short _Fract fract_array[10];
    _Accum accum_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        fract_array[i] = (short _Fract)(i * 0.1r);
        accum_array[i] = (_Accum)(i * 0.2k);
    }
    
    /* Perform fixed-point operations in loop */
    for (int i = 0; i < iterations; i++) {
        /* Use array elements to prevent constant folding */
        short _Fract a = fract_array[i];
        short _Fract b = fract_array[9 - i];
        _Accum c = accum_array[i];
        
        /* Call operations function with varying shift */
        fixed_point_operations(a, b, c, i + 1);
        
        /* Test saturation with values near limits */
        long _Fract la = 0.9lr;
        long _Fract lb = 0.95lr;
        saturation_test(la, lb);
        
        /* Additional shift operations */
        _Accum val = 0.5k;
        for (int j = 0; j < 3; j++) {
            val = val << (j + 1);               /* Multiple FIXED_LSHIFT_EXPR */
            volatile _Accum vval = val;
            asm volatile ("" : : : "memory");
        }
        
        /* Mixed-width operations */
        short _Fract mixed = a * global_volatile_fract;
        _Accum widened = (_Accum)mixed * c;
        
        /* Force potential overflow by assigning to narrower type */
        short _Fract narrowed = widened;        /* May trigger range check */
        
        volatile short _Fract vnarrowed = narrowed;
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation with all types */
    unsigned short _Fract uf1 = 0.8ur;
    unsigned short _Fract uf2 = 0.9ur;
    unsigned short _Fract uresult = uf1 * uf2;
    
    _Accum accum_result = 0k;
    for (int i = 0; i < iterations; i++) {
        accum_result += (_Accum)fract_array[i] * accum_array[i];
    }
    
    /* Print final results to ensure side effects */
    printf("Final: %hd %hd\n", 
           (short)(uresult * 256),
           (short)(accum_result * 256));
    
    return 0;
}
