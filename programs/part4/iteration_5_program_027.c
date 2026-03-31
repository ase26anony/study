#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double dummy_func2(double a, double b, float c, int d, int e) {
    volatile double result = a * b + c + d + e;
    return result;
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
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* More variables for additional pressure */
    int i11 = rand() % 100;
    int i12 = rand() % 100;
    int i13 = rand() % 100;
    int i14 = rand() % 100;
    int i15 = rand() % 100;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    double d6 = (double)rand() / RAND_MAX;
    double d7 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES - create register class conflicts */
    /* These will bind to specific registers, creating constraints */
    register int r_i1 asm("r12") = i1;  /* Try to bind to r12 */
    register int r_i2 asm("r13") = i2;  /* Try to bind to r13 */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int idx = 0; idx < 256; idx++) {
        array[idx] = rand() % 1000;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization across iterations */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer and floating point operations */
        i1 = i2 + i3 * i4 - i5 / (i6 + 1);
        i2 = i7 ^ i8 | i9 & i10;
        i3 = (i11 << 2) | (i12 >> 3);
        
        /* Integer to float conversions - require moves between register files */
        f1 = (float)i1 + f2 * 3.14159f;
        f2 = (float)(i2 & 0xFF) / 255.0f;
        
        /* Float to double conversions */
        d1 = (double)f1 + d2 * 2.71828;
        d2 = (double)f2 + d3 / 1.41421;
        
        /* Mixed-type expressions */
        f3 = f4 + (float)i3 * 0.5f;
        d3 = d4 + (double)(i4 * i5) * 0.25;
        
        /* More complex expressions with many operands */
        i4 = ((i6 * i7) + (i8 << 1)) / (i9 | 1);
        i5 = (i10 ^ i11) & (i12 | i13) + (i14 << i15);
        
        /* Non-offsettable memory addressing - may require address reload */
        /* array[index + constant] where index is complex expression */
        int complex_index = (i1 * i2 + i3) & 0xFF;
        int mem_val = array[complex_index + 16];  /* Non-simple offset */
        i6 = mem_val * i7;
        
        /* More pointer arithmetic with complex expressions */
        ptr = array + ((i8 * 3 + i9 * 7) & 0x7F);
        i7 = *ptr + i10;
        
        /* Different sized memory accesses */
        short short_val = (short)(array[complex_index] & 0xFFFF);
        char char_val = (char)(array[complex_index + 1] & 0xFF);
        i8 = short_val * char_val + i11;
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_func(i1, i2, f1, d1, i3);
        i9 = call_result + i4;
        
        /* INLINE ASSEMBLY WITH CLOBBERS - increases register pressure */
        /* Tell compiler many registers are unavailable */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11"
        );
        
        /* More computations after clobber */
        f4 = f5 * 2.0f + (float)i5;
        d4 = d5 * 3.0 + (double)i6;
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloading due to register class constraints */
        f5 = (float)r_i1 + f6;  /* Integer register used in float op */
        d5 = (double)r_i2 + d6; /* Integer register used in double op */
        
        /* Another function call with different types */
        double d_result = dummy_func2(d1, d2, f3, i7, i8);
        d6 = d_result * 0.5;
        
        /* Complex expression mixing all types */
        accumulator += (double)i1 + (double)f1 + d1 + 
                      (double)(i2 * i3) + (double)(f2 * f3) + 
                      d2 * d3 + (double)(i4 & i5);
        
        /* Update volatile variables to prevent elimination */
        v1 = i1 + i2;
        v2 = f1 + f2;
        v3 = d1 + d2;
        
        /* More register pressure operations */
        i10 = i11 * i12 - i13 / (i14 + 1);
        i11 = i12 ^ i13 | i14 & i15;
        f6 = f7 * (float)i10 + 1.234f;
        f7 = (float)i11 / 45.67f;
        d7 = d6 * (double)i12 + 89.1011;
        
        /* Another inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
              "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Final mixed-type computation */
        i12 = (int)((double)i13 * d7 + (float)i14 * f7);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f\n", accumulator);
    printf("Volatile checks: %d, %f, %f\n", v1, v2, v3);
    
    return 0;
}
