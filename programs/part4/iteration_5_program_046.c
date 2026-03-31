#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_calc(double a, double b, double c, int d, float e, short f) {
    volatile double temp = a * b + c / d + e * f;
    return temp;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_counter = 0;
    volatile int volatile_acc = 0;
    volatile float volatile_float = 1.5f;
    volatile double volatile_double = 2.71828;
    
    /* MANY LIVE SCALAR VARIABLES (20+ to create register pressure) */
    /* Integer variables */
    int var1 = rand() % 100;
    int var2 = rand() % 100 + 1;
    int var3 = rand() % 100;
    int var4 = rand() % 100;
    int var5 = rand() % 100;
    int var6 = rand() % 100;
    int var7 = rand() % 100;
    int var8 = rand() % 100;
    int var9 = rand() % 100;
    int var10 = rand() % 100;
    
    /* Floating point variables */
    float fvar1 = (float)rand() / RAND_MAX;
    float fvar2 = (float)rand() / RAND_MAX;
    float fvar3 = (float)rand() / RAND_MAX;
    float fvar4 = (float)rand() / RAND_MAX;
    float fvar5 = (float)rand() / RAND_MAX;
    
    /* Double variables */
    double dvar1 = (double)rand() / RAND_MAX;
    double dvar2 = (double)rand() / RAND_MAX;
    double dvar3 = (double)rand() / RAND_MAX;
    double dvar4 = (double)rand() / RAND_MAX;
    double dvar5 = (double)rand() / RAND_MAX;
    
    /* Short and char variables for different sized accesses */
    short svar1 = rand() % 256;
    short svar2 = rand() % 256;
    char cvar1 = rand() % 128;
    char cvar2 = rand() % 128;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    int *ptr1 = &array[0];
    int *ptr2 = &array[128];
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("r12") = var1;  /* Try to bind to specific reg */
    register int reg_var2 asm("r13") = var2;  /* May conflict with compiler choices */
    
    /* Loop with invariant spilling - volatile condition prevents optimization */
    while (loop_counter < 100) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS mixing all types */
        
        /* Integer operations with bitwise and arithmetic */
        var1 = ((var2 & var3) | (var4 ^ var5)) + (var6 << 2) * (var7 >> 1);
        var2 = var1 * var3 - var4 / (var5 + 1) + (var6 & 0xFF) | (var7 ^ var8);
        
        /* Floating point operations with conversions */
        fvar1 = (float)var1 + fvar2 * 3.14f - (float)(var2 & 0xFF);
        fvar2 = fvar3 / (fvar4 + 1.0f) + (float)(var3 % 100);
        
        /* Double precision operations */
        dvar1 = dvar2 * dvar3 + (double)var4 - dvar4 / (dvar5 + 1.0);
        dvar2 = (double)fvar1 + dvar1 * 2.0 - (double)(var5 & 0xFFF);
        
        /* Type conversions between int and float/double */
        var3 = (int)(fvar1 * 100.0f) + (int)dvar1;
        fvar3 = (float)var6 + (float)var7 / 10.0f;
        dvar3 = (double)var8 + (double)var9 / 100.0;
        
        /* Mixed type expression */
        var4 = (int)((fvar4 * dvar4) + (var10 & 0xFF)) | (svar1 << 8);
        
        /* Different sized memory accesses */
        cvar1 = (char)(array[var1 % 256] & 0xFF);
        svar1 = (short)(array[var2 % 256] & 0xFFFF);
        var5 = array[var3 % 256];
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = var6 % 128;
        int idx2 = var7 % 128;
        
        /* Non-simple addressing: array[index + constant] where constant is large */
        var6 = array[idx1 + 64];  /* May be non-offsettable depending on arch */
        var7 = array[idx2 + 96];  /* Large offset may need separate register */
        
        /* Pointer arithmetic that might need reloading */
        int offset = var8 % 64;
        var8 = *(ptr1 + offset + 16);  /* Complex address calculation */
        var9 = *(ptr2 - offset + 32);  /* Another complex address */
        
        /* FUNCTION CALL to clobber registers */
        int call_result = dummy_function(var1, var2, fvar1, dvar1, var3, fvar2);
        var10 = call_result & 0xFF;
        
        /* INLINE ASSEMBLY with MANY CLOBBERS to increase register pressure */
        /* This tells GCC these registers are unusable, forcing more reloads */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More complex calculations after asm clobber */
        dvar4 = complex_calc(dvar1, dvar2, dvar3, var4, fvar3, svar2);
        fvar4 = (float)dvar4 + fvar1 - fvar2 * 2.0f;
        
        /* Use explicit register variables in conflicting contexts */
        /* Try to use integer register variable in float context */
        fvar5 = fvar5 + (float)reg_var1 / 100.0f;
        
        /* Bitfield operations */
        var1 = (var1 & 0xF0F0F0F0) | (var2 & 0x0F0F0F0F);
        var2 = (var2 << 4) | (var3 >> 4);
        
        /* Update volatile accumulator to prevent dead code elimination */
        volatile_acc += var1 + var2 + (int)fvar1 + (int)dvar1;
        volatile_acc -= var3 + var4 + (int)fvar2;
        
        /* Mix in char/short operations */
        cvar2 = (cvar1 + svar1) & 0xFF;
        svar2 = (svar1 * 2 - svar2) & 0xFFFF;
        
        /* Another function call */
        double dresult = complex_calc(dvar2, dvar3, dvar4, var5, fvar4, svar1);
        dvar5 = dresult * 0.99;
        
        /* Loop control with volatile to prevent optimization */
        loop_counter++;
        
        /* Additional asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11"
        );
    }
    
    /* Final output to prevent complete optimization */
    printf("Result: %d\n", volatile_acc);
    printf("Final values: %d %f %lf\n", var1, fvar1, dvar1);
    
    return 0;
}
