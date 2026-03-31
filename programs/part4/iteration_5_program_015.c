#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int barrier = a + b + (int)c + (int)d + e + (int)f;
    return barrier & 0xFF;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_op(int x, double y, float z, int w) {
    volatile double result = (double)x * y + (double)z * w;
    return result;
}

int main(void) {
    /* Phase 1: Declare MANY variables to create register pressure */
    /* Integer variables - many will need to stay live */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    
    /* More non-volatile integers with complex interdependencies */
    int i1 = v1 * 2;
    int i2 = v2 + v3;
    int i3 = v4 ^ v5;
    int i4 = v6 | v7;
    int i5 = v8 & v9;
    int i6 = v10 << 2;
    int i7 = i1 + i2;
    int i8 = i3 * i4;
    int i9 = i5 - i6;
    int i10 = i7 ^ i8;
    int i11 = i9 + i10;
    int i12 = i11 * 3;
    int i13 = i12 / 2;
    int i14 = i13 | 0xFF;
    int i15 = i14 & 0x7F;
    
    /* Floating point variables - mixed precision */
    volatile float f1 = (float)rand() / RAND_MAX;
    volatile float f2 = (float)rand() / RAND_MAX;
    volatile float f3 = (float)rand() / RAND_MAX;
    volatile float f4 = (float)rand() / RAND_MAX;
    volatile float f5 = (float)rand() / RAND_MAX;
    
    float f6 = f1 * 2.0f;
    float f7 = f2 + f3;
    float f8 = f4 / f5;
    float f9 = f6 - f7;
    float f10 = f8 * 3.14f;
    
    /* Double precision variables */
    volatile double d1 = (double)rand() / RAND_MAX;
    volatile double d2 = (double)rand() / RAND_MAX;
    volatile double d3 = (double)rand() / RAND_MAX;
    
    double d4 = d1 * 2.0;
    double d5 = d2 + d3;
    double d6 = d4 / 3.14159;
    double d7 = d5 * d6;
    
    /* Phase 2: Explicit register variables with potential conflicts */
    /* These create hard constraints on register allocation */
    register int r1 asm("r1") = v1 + 1;
    register int r2 asm("r2") = v2 + 2;
    /* Note: Actual register names would be arch-specific, but compiler
       will try to honor these requests, creating pressure */
    
    /* Phase 3: Array with complex addressing */
    int array[256];
    for (int j = 0; j < 256; j++) {
        array[j] = rand() % 1000;
    }
    
    /* Phase 4: Loop with high register pressure and complex operations */
    volatile int loop_limit = 100; /* volatile prevents loop unrolling */
    volatile double accumulator = 0.0;
    
    for (int iter = 0; iter < loop_limit; iter++) {
        /* Complex expression mixing all variable types - creates many
           intermediate values that need registers */
        double temp1 = (double)i1 * d1 + (double)i2 * d2;
        float temp2 = (float)i3 * f1 + (float)i4 * f2;
        int temp3 = (int)(temp1 * 100.0) ^ (int)(temp2 * 100.0f);
        
        /* Type conversions that require moving between register files */
        double temp4 = (double)((float)i5 + f3) * d3;
        float temp5 = (float)((double)i6 / d4) + f4;
        
        /* Complex pointer arithmetic with non-simple addressing */
        /* array[index + constant] where index is complex expression */
        int idx = (i7 + i8 + iter) & 0xFF;
        int val1 = array[idx + 5];  /* Non-offsettable address */
        int val2 = array[(idx * 2) & 0xFF];  /* Even more complex */
        
        /* Bitwise and arithmetic combinations */
        int temp6 = (val1 * val2) + (i9 << 2) | (i10 & 0xF0);
        float temp7 = (float)temp6 * f5 - f6 / 2.0f;
        
        /* Function call clobbers registers */
        int ret1 = dummy_func(i11, i12, f7, d5, i13, f8);
        
        /* Inline assembly with clobbers - increases register pressure */
        /* Generic clobbers that work on multiple architectures */
        asm volatile (
            "/* dummy assembly */"
            : /* no outputs */
            : /* no inputs */
            : "memory", "cc"  /* Clobbers memory and condition codes */
            /* Add more specific register clobbers for your architecture:
               For x86: "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
               For ARM: "r0", "r1", "r2", "r3", "r4", "r5" */
        );
        
        /* More complex operations after clobber */
        double temp8 = complex_op(ret1, d6, f9, i14);
        float temp9 = (float)temp8 + f10 * 2.0f;
        
        /* Use explicit register variables in complex context */
        int temp10 = r1 * r2 + i15;  /* r1, r2 are register-bound */
        
        /* Mixed-size memory accesses */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        int byte_sum = 0;
        for (int k = 0; k < 16; k++) {
            byte_sum += byte_ptr[(idx + k) & 0xFF];  /* byte access */
        }
        
        int short_sum = 0;
        for (int k = 0; k < 8; k++) {
            short_sum += short_ptr[(idx + k) & 0x7F];  /* short access */
        }
        
        /* Final complex expression using everything */
        accumulator += (double)temp3 * 0.01 
                     + (double)temp6 * 0.001 
                     + temp4 
                     + (double)temp7 
                     + temp8 
                     + (double)temp9 
                     + (double)temp10 * 0.0001
                     + (double)byte_sum * 0.00001
                     + (double)short_sum * 0.000001;
        
        /* Update some variables to create loop-carried dependencies */
        i1 = (i1 + iter) & 0xFF;
        i2 = i2 ^ (iter * 3);
        f1 = f1 * 1.01f;
        d1 = d1 * 1.001;
        
        /* Another function call to force more register saves */
        ret1 = dummy_func(i1, i2, f1, d1, temp10, (float)accumulator);
    }
    
    /* Phase 5: Final complex computation to use all variables */
    double final_result = accumulator;
    final_result += (double)v1 * 0.1;
    final_result += (double)v2 * 0.01;
    final_result += (double)i15 * 0.001;
    final_result += d7 * 10.0;
    final_result += (double)f10 * 0.1;
    
    /* Use all remaining variables in output calculation */
    int output = (int)final_result % 1000;
    
    printf("Result: %d\n", output);
    return output;
}
