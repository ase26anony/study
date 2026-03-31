#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_calc(double a, double b, float c, int d) {
    volatile double res = (a * b) / (c + d);
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    
    /* MANY LIVE SCALAR VARIABLES - create register pressure */
    /* Integer variables */
    int i1 = rand() % 100;
    int i2 = rand() % 100;
    int i3 = rand() % 100;
    int i4 = rand() % 100;
    int i5 = rand() % 100;
    int i6 = rand() % 100;
    int i7 = rand() % 100;
    int i8 = rand() % 100;
    int i9 = rand() % 100;
    int i10 = rand() % 100;
    
    /* More integers */
    int i11 = rand() % 100;
    int i12 = rand() % 100;
    int i13 = rand() % 100;
    int i14 = rand() % 100;
    int i15 = rand() % 100;
    int i16 = rand() % 100;
    int i17 = rand() % 100;
    int i18 = rand() % 100;
    int i19 = rand() % 100;
    int i20 = rand() % 100;
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    
    /* Double variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    /* These may bind to specific registers, creating constraints */
    register int reg_var1 asm("eax");
    register int reg_var2 asm("ebx");
    register double reg_dbl asm("xmm0");
    
    reg_var1 = rand() % 100;
    reg_var2 = rand() % 100;
    reg_dbl = (double)rand() / RAND_MAX;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int j = 0; j < 256; j++) {
        array[j] = rand() % 1000;
    }
    
    /* Volatile loop counter to prevent optimization across iterations */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer operations */
        i1 = i2 + i3 * i4 - i5 / (i6 + 1);
        i2 = i3 ^ i4 | i5 & i6;
        i3 = (i7 << 2) | (i8 >> 3);
        i4 = i9 * i10 - i11 + i12;
        
        /* Mixed integer/float operations - may require reloads */
        f1 = (float)i1 / (float)(i2 + 1) + f2 * f3;
        f2 = f3 - f4 + (float)(i3 * i4) / 100.0f;
        
        /* Integer to float conversions */
        f3 = (float)(i5 + i6) * 0.5f;
        f4 = (float)(i7 | i8) + f5;
        
        /* Float to double conversions */
        d1 = (double)f1 + d2 * 0.5;
        d2 = (double)f2 / (d3 + 0.001);
        
        /* Double precision operations */
        d3 = d4 * d5 + (double)i9 / 1000.0;
        d4 = d1 - d2 + d3 * d5;
        
        /* More complex integer expressions */
        i5 = (i13 + i14) * (i15 - i16);
        i6 = (i17 & 0xFF) | (i18 << 8);
        i7 = i19 ^ i20 + i1 * i2;
        
        /* FUNCTION CALL - clobbers registers, forces save/restore */
        int func_result = dummy_func(i1, i2, f1, d1, i3);
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads if used in floating point ops */
        float temp_float = (float)reg_var1 + f3;
        double temp_double = reg_dbl * d2;
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* Forces compiler to work around unavailable registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* Create addresses that may need reloading into registers */
        int idx = i8 + i9;
        /* array[idx + 1000] - large offset may need separate register */
        int array_val = array[(idx + 1000) & 0xFF];
        
        /* More pointer arithmetic with different type sizes */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        /* Mixed size accesses in same expression */
        int mixed_access = byte_ptr[i10] + short_ptr[i11] + array[i12];
        
        /* TYPE CONVERSIONS and mixed operations */
        double conv1 = (double)(i13 + i14) * f4;
        float conv2 = (float)d4 + (float)i15;
        
        /* Bitwise and arithmetic combinations */
        int complex_expr = (i16 & 0xF0) + (i17 << 4) * (i18 >> 2) - i19 / (i20 + 1);
        
        /* Another function call with mixed arguments */
        double dbl_result = complex_calc(d1, d2, f3, i1);
        
        /* Update accumulator with volatile write */
        accumulator += (double)func_result + dbl_result + (double)complex_expr 
                     + (double)array_val + temp_double + (double)mixed_access;
        
        /* Shuffle values to create new dependencies for next iteration */
        i8 = i9; i9 = i10; i10 = i11;
        i11 = i12; i12 = i13; i13 = i14;
        i14 = i15; i15 = i16; i16 = i17;
        i17 = i18; i18 = i19; i19 = i20;
        i20 = complex_expr & 0xFF;
        
        f5 = f4; f4 = f3; f3 = f2; f2 = f1;
        f1 = conv2 * 0.9f;
        
        d5 = d4; d4 = d3; d3 = d2; d2 = d1;
        d1 = conv1 * 0.99;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final accumulator: %f\n", accumulator);
    printf("Volatile values: %d, %f, %f\n", v1, v2, v3);
    
    return 0;
}
