#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_calc(double a, double b, double c, int d, float e) {
    volatile double res = a * b + c / d + e;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int vol_counter = 0;
    volatile int vol_limit = 100;
    volatile double vol_accumulator = 0.0;
    
    /* MANY INTEGER VARIABLES - create register pressure */
    int var1 = rand() % 100;
    int var2 = rand() % 100;
    int var3 = rand() % 100;
    int var4 = rand() % 100;
    int var5 = rand() % 100;
    int var6 = rand() % 100;
    int var7 = rand() % 100;
    int var8 = rand() % 100;
    int var9 = rand() % 100;
    int var10 = rand() % 100;
    int var11 = rand() % 100;
    int var12 = rand() % 100;
    int var13 = rand() % 100;
    int var14 = rand() % 100;
    int var15 = rand() % 100;
    
    /* MANY FLOATING POINT VARIABLES - pressure on FP registers */
    float fvar1 = (float)rand() / RAND_MAX;
    float fvar2 = (float)rand() / RAND_MAX;
    float fvar3 = (float)rand() / RAND_MAX;
    float fvar4 = (float)rand() / RAND_MAX;
    float fvar5 = (float)rand() / RAND_MAX;
    float fvar6 = (float)rand() / RAND_MAX;
    float fvar7 = (float)rand() / RAND_MAX;
    float fvar8 = (float)rand() / RAND_MAX;
    
    /* MANY DOUBLE VARIABLES */
    double dvar1 = (double)rand() / RAND_MAX;
    double dvar2 = (double)rand() / RAND_MAX;
    double dvar3 = (double)rand() / RAND_MAX;
    double dvar4 = (double)rand() / RAND_MAX;
    double dvar5 = (double)rand() / RAND_MAX;
    double dvar6 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("ax") = 100;
    register int reg_var2 asm("bx") = 200;
    register float reg_fvar asm("xmm0") = 3.14f;
    
    /* Array for complex addressing modes */
    int large_array[256];
    for (int i = 0; i < 256; i++) {
        large_array[i] = i * 2;
    }
    
    /* Pointer with complex arithmetic */
    int* ptr1 = large_array + 128;
    
    /* LOOP with high register pressure */
    while (vol_counter < vol_limit) {
        /* COMPLEX EXPRESSIONS mixing all variable types */
        int temp1 = var1 * var2 + var3 - var4 / (var5 + 1);
        float temp2 = fvar1 * fvar2 + (float)var6 / (float)var7;
        double temp3 = dvar1 * dvar2 + (double)var8;
        
        /* TYPE CONVERSIONS - force moves between register files */
        fvar3 = (float)reg_var1 + fvar4;  /* integer reg to float reg */
        var9 = (int)reg_fvar + var10;     /* float reg to integer reg */
        
        /* COMPLEX ADDRESSING with non-offsettable addresses */
        /* array[index + constant] where offset may be too large */
        int index = var11 + var12;
        int array_val1 = large_array[index + 64];  /* May need address reload */
        int array_val2 = large_array[index * 2 + 32]; /* More complex */
        
        /* Use explicit register variables in conflicting contexts */
        int conflict1 = reg_var1 * array_val1;  /* Using ax register variable */
        float conflict2 = reg_fvar * fvar5;     /* Using xmm0 register variable */
        
        /* BITWISE and ARITHMETIC combinations */
        var13 = (var14 << 3) | (var15 & 0xFF);
        var14 = (var13 * 37) ^ (var12 | 0x7F);
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int func_result = dummy_function(var1, var2, fvar1, dvar1, var3, fvar2);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS */
        /* Force compiler to save/restore registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* MORE COMPLEX CALCULATIONS after asm clobber */
        dvar4 = complex_calc(dvar2, dvar3, dvar4, var4, fvar6);
        
        /* MIXED TYPE OPERATIONS in single expression */
        vol_accumulator += (double)temp1 + temp2 + temp3 + 
                          (double)func_result + dvar4 + 
                          (double)conflict1 + conflict2;
        
        /* UPDATE all variables to extend live ranges */
        var1 = var2 + 1;
        var2 = var3 * 2;
        var3 = var4 | 0x55;
        var4 = var5 ^ var6;
        var5 = var7 << 1;
        var6 = var8 >> 2;
        var7 = var9 + var10;
        var8 = var11 - var12;
        var9 = var13 * 3;
        var10 = var14 / 4;
        var11 = var15 % 5;
        
        fvar1 = fvar2 * 1.1f;
        fvar2 = fvar3 + 2.2f;
        fvar3 = fvar4 - 3.3f;
        fvar4 = fvar5 / 4.4f;
        
        dvar1 = dvar2 * 1.01;
        dvar2 = dvar3 + 2.02;
        dvar3 = dvar4 - 3.03;
        
        /* Update volatile counter */
        vol_counter++;
        
        /* Prevent loop unrolling with volatile condition */
        if (vol_counter % 10 == 0) {
            asm volatile("" : : "r"(vol_counter) : "memory");
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final accumulator: %f\n", (double)vol_accumulator);
    printf("Variables: %d %d %f %f\n", var1, var2, fvar1, dvar1);
    
    return 0;
}
