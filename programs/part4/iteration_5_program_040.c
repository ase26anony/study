#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double another_dummy(int x, double y, float z, int w) {
    volatile double res = y * z + x - w;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_counter = 0;
    volatile int accumulator = 0;
    volatile int limit = 100;  /* Force loop execution */
    
    /* MANY LIVE SCALAR VARIABLES - create register pressure */
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
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    float f5 = (float)(rand() % 100) / 10.0f;
    float f6 = (float)(rand() % 100) / 10.0f;
    
    /* Double variables */
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    double d4 = (double)(rand() % 100) / 5.0;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    /* These may conflict with register allocator's choices */
    register int reg_var1 asm("eax") = v1 + v2;
    register int reg_var2 asm("ebx") = v3 * v4;
    register float reg_float asm("xmm0") = f1 + f2;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* LOOP with high register pressure */
    while (loop_counter < limit) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer and floating point operations */
        v1 = v2 + v3 - v4 * v5 / (v6 + 1);
        v2 = v7 | v8 & v9 ^ v10;
        v3 = (v11 << 2) | (v12 >> 3);
        v4 = v13 * v14 - v15;
        
        /* Type conversions - force moves between register files */
        f1 = (float)v1 + f2 * 2.5f;
        f2 = f3 / f4 + (float)v2;
        d1 = (double)f1 + d2 * 3.14159;
        d2 = (double)v3 / d3 + d4;
        
        /* Mixed size accesses in same expression */
        char char_val = (char)(v4 & 0xFF);
        short short_val = (short)(v5 & 0xFFFF);
        v5 = char_val * 2 + short_val / 3 + v6;
        
        /* Use explicit register variables in conflicting contexts */
        /* This may require reloads if register classes conflict */
        f3 = reg_float + (float)reg_var1 * 0.5f;
        v6 = reg_var2 + (int)(reg_float * 2.0f);
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_function(v1, v2, f1, d1, v3, v4);
        
        /* INLINE ASSEMBLY with many clobbers - increase register pressure */
        /* This tells GCC these registers are unusable */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = v7 % 128;
        int idx2 = v8 % 64;
        
        /* Non-simple addressing: array[index + constant] where offset may be too large */
        int val1 = array[idx1 + 64];  /* May require address calculation reload */
        int val2 = array[idx2 + 96];  /* Another complex address */
        
        /* Pointer arithmetic that might not be directly addressable */
        int* complex_ptr = ptr + idx1 - idx2 + 16;
        int val3 = *complex_ptr;
        
        /* More mixed operations after assembly clobber */
        v7 = val1 + val2 * val3 - call_result;
        v8 = (v9 << 3) | (v10 >> 1) & v11;
        
        /* Another function call with different types */
        double d_result = another_dummy(v12, d2, f3, v13);
        
        /* Bitwise and arithmetic combinations */
        v9 = (v14 & 0xF0F0) | (v15 << 4);
        v10 = (v1 * v2) + (v3 & v4) - (v5 | v6);
        
        /* Floating point to integer conversion */
        v11 = (int)(f4 * 100.0f) + (int)d3;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v1 + v2 + v3 + (int)f1 + (int)d1 + val1 + val2 + (int)d_result;
        
        /* More complex expressions */
        f4 = f5 * 2.0f - f6 / 3.0f + (float)v7;
        f5 = (float)v8 / 5.0f + f1 * f2;
        
        d3 = d4 * 1.5 + (double)f3 / 2.0;
        d4 = (double)v9 + d1 - d2;
        
        v12 = v13 ^ v14 & v15 | v1;
        v13 = v2 * 3 - v4 / 2 + v6;
        
        /* Another inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11"
        );
        
        /* Final mixed computation */
        v14 = (v15 << 1) + (v1 >> 2) * (v3 & 0xFF);
        v15 = (int)(f4 * d4) + v2 - v5;
        
        /* Update loop counter */
        loop_counter++;
    }
    
    /* Use all variables one more time to extend live ranges */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        accumulator;
    
    printf("Result: %d\n", final_result % 1000);
    
    return 0;
}
