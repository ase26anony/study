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
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / w;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int limit = 100;
    volatile int accumulator = 0;
    
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
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    
    /* Double variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("r12") = v1 + v2;  /* Try to bind to specific reg */
    register int reg_var2 asm("r13") = v3 * v4;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array;
    
    /* LOOP with invariant spilling */
    for (volatile int iter = 0; iter < limit; iter++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v7 ^ v8 | v9 & v10;
        v3 = (v11 << 2) | (v12 >> 3);
        v4 = v13 * v14 - v15;
        
        /* MIXED TYPE OPERATIONS - force register class/mode changes */
        f1 = (float)v1 + f2 * 3.14f;
        f2 = f3 / (float)v2 + f4;
        
        /* Integer to float conversions */
        d1 = (double)v3 + d2 * 2.71828;
        d2 = d3 / (double)v4 + d4;
        
        /* Float to integer conversions */
        v5 = (int)f1 + (int)f2 * v6;
        v6 = (int)d1 ^ (int)d2 | v7;
        
        /* Different sized memory accesses */
        char char_val = (char)(v8 & 0xFF);
        short short_val = (short)(v9 & 0xFFFF);
        v7 = char_val * 3 + short_val / 2;
        
        /* NON-OFFSETTABLE ADDRESSING - complex pointer arithmetic */
        /* This often requires address reloads */
        int index = v10 + v11;
        int value1 = array[index + 128];  /* Large offset may need reload */
        int value2 = array[index + 64];   /* Another large offset */
        v8 = value1 * value2 + v12;
        
        /* More complex addressing with multiple components */
        ptr = array + v13;
        int value3 = ptr[v14 * 2 + 16];  /* Non-simple address */
        v9 = value3 + v15;
        
        /* FUNCTION CALL - clobbers registers, forces save/restore */
        int call_result = dummy_function(v1, v2, f1, d1, v3, f2);
        v10 = call_result + v4;
        
        /* INLINE ASSEMBLY with MANY CLOBBERS - increase register pressure */
        /* This tells GCC these registers are unusable */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", 
              "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* More calculations after clobber */
        f3 = f4 * f5 + (float)v5;
        f4 = f6 / f7 - (float)v6;
        
        d3 = d4 * d5 + (double)v7;
        d4 = d1 / d2 - (double)v8;
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads due to register class conflicts */
        float temp_float = (float)reg_var1 + f3;  /* Integer reg var in float op */
        double temp_double = (double)reg_var2 * d3;
        
        /* Bitwise and arithmetic combinations */
        v11 = (v12 & v13) | (v14 ^ v15);
        v12 = (v11 << 3) + (v12 >> 2) * v13;
        
        /* Another function call with mixed types */
        double d_result = complex_op(v13, f4, d4, v14);
        v13 = (int)d_result + v15;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v1 + v2 + v3 + (int)f1 + (int)d1;
        
        /* More mixed operations */
        v14 = v15 * 2 - v1;
        v15 = (v2 + v3) & (v4 | v5);
        
        f5 = f6 + f7 * 2.0f;
        f6 = f1 - f2 / 3.0f;
        
        d5 = d1 + d2 * 1.5;
    }
    
    /* Use all variables at the end to ensure they're live */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
        reg_var1 + reg_var2;
    
    printf("Result: %d (accumulator: %d)\n", final_result, accumulator);
    return 0;
}
