/* reload_stress.c - Program to stress GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_calc(double x, double y, float z, int w) {
    volatile double r = x * y + z - w;
    return r;
}

int main(void) {
    /* VOLATILE VARIABLES - Prevent optimization */
    volatile int loop_counter;
    volatile int accumulator = 0;
    volatile int limit = 100;
    
    /* MANY LIVE SCALAR VARIABLES - Create register pressure */
    /* Integer variables */
    int v1 = rand() % 100;
    int v2 = rand() % 100;
    int v3 = rand() % 100;
    int v4 = rand() % 100;
    int v5 = rand() % 100;
    int v6 = rand() % 100;
    int v7 = rand() % 100;
    int v8 = rand() % 100;
    int v9 = rand() % 100;
    int v10 = rand() % 100;
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    
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
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("eax") = v1 + v2;
    register int reg_var2 asm("ebx") = v3 * v4;
    register float reg_var3 asm("xmm0") = f1 + f2;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer with complex arithmetic */
    int* ptr = array + 128;
    
    /* LOOP with invariant spilling */
    for (loop_counter = 0; loop_counter < limit; loop_counter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - Extend live ranges */
        /* Mix integer and floating point operations */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v7 | v8 & v9 ^ v10;
        v3 = (v11 << 3) | (v12 >> 2);
        
        /* Integer to float conversions - requires register file transfers */
        f1 = (float)v1 + (float)v2 * 0.5f;
        f2 = (float)(v3 & 0xFF) + f1;
        
        /* Float to double conversions */
        d1 = (double)f1 * (double)f2;
        d2 = d1 + (double)(v4 % 100);
        
        /* Mixed type operations in single expression */
        d3 = d1 * d2 + (double)f3 - (double)v5 + (double)(v6 & 0xF);
        
        /* Complex addressing mode: array[index + large_constant] */
        /* This often requires reloading the address calculation */
        int idx1 = v7 % 64;
        int idx2 = v8 % 64;
        v9 = array[idx1 + 100] + array[idx2 + 150];  /* Non-offsettable addresses */
        v10 = ptr[idx1 - 64] + ptr[idx2 - 32];       /* More complex addressing */
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_func(v1, v2, f1, d1, v3, v4);
        
        /* INLINE ASSEMBLY with multiple clobbers - increases register pressure */
        /* This tells GCC these registers are unusable after this point */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More calculations after clobber - forces reloads */
        f3 = f1 * f2 + (float)call_result;
        f4 = (float)v9 / (float)(v10 + 1);
        
        /* Using explicit register variables in conflicting contexts */
        /* reg_var1 is in eax (integer reg), using in float context */
        float temp_float = (float)reg_var1 + f3;
        
        /* reg_var3 is in xmm0 (float reg), using in integer context */
        int temp_int = (int)reg_var3 + v11;
        
        /* Different sized memory accesses in same expression */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        v11 = byte_ptr[v12 % 128] + 
              short_ptr[(v13 % 64) * 2] + 
              array[v14 % 64];
        
        /* Another function call with mixed arguments */
        double dresult = complex_calc(d1, d2, f1, v15);
        
        /* Update volatile accumulator to prevent dead code elimination */
        accumulator += v1 + v2 + (int)f1 + (int)dresult + temp_int;
        
        /* Rotate values to extend live ranges across iterations */
        int temp = v15;
        v15 = v14; v14 = v13; v13 = v12; v12 = v11;
        v11 = v10; v10 = v9; v9 = v8; v8 = v7;
        v7 = v6; v6 = v5; v5 = v4; v4 = v3;
        v3 = v2; v2 = v1; v1 = temp + loop_counter;
        
        float ftemp = f5;
        f5 = f4; f4 = f3; f3 = f2; f2 = f1;
        f1 = ftemp + (float)loop_counter * 0.1f;
        
        double dtemp = d5;
        d5 = d4; d4 = d3; d3 = d2; d2 = d1;
        d1 = dtemp + (double)loop_counter * 0.01;
    }
    
    printf("Final accumulator: %d\n", accumulator);
    return 0;
}
