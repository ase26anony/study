#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
float float_ops(float a, float b, float c, double d, double e) {
    volatile float res = a * b + c / (float)d - e;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int trigger = 1;
    volatile int loop_counter = 0;
    volatile double accumulator = 0.0;
    
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
    
    /* More integers */
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    int v16 = rand() % 100;
    int v17 = rand() % 100;
    int v18 = rand() % 100;
    int v19 = rand() % 100;
    int v20 = rand() % 100;
    
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
    /* These may bind to specific registers, creating constraints */
    register int reg_var1 asm("ax") = v1 + v2;
    register int reg_var2 asm("bx") = v3 * v4;
    register float reg_float asm("xmm0") = f1 + f2;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* LOOP with invariant spilling */
    while (loop_counter < 100) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        
        /* Mixed integer operations */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v7 & v8 | v9 ^ v10;
        v3 = (v11 << 2) | (v12 >> 3);
        v4 = v13 * v14 + v15 - v16;
        v5 = v17 % (v18 + 1) + v19 * v20;
        
        /* Integer to float conversions - require moves between register files */
        f1 = (float)v1 + (float)v2 * 0.5f;
        f2 = (float)(v3 & 0xFF) / (float)(v4 | 1);
        f3 = (float)(v5 ^ v6) + (float)(v7 << 1);
        
        /* Float to double conversions */
        d1 = (double)f1 * (double)f2;
        d2 = (double)f3 + (double)f4 - (double)f5;
        
        /* Mixed precision operations */
        d3 = d1 * d2 + (double)v8;
        d4 = d2 / (d1 + 1.0) - (double)v9;
        
        /* Use explicit register variables in conflicting contexts */
        /* This may require reloads due to register class mismatches */
        f4 = reg_float + (float)reg_var1 * 0.3f;
        v6 = reg_var2 + (int)(reg_float * 10.0f);
        
        /* FUNCTION CALL - clobbers registers, forces save/restore */
        int call_result = dummy_function(v1, v2, f1, d1, v3, v4);
        v7 = call_result + v5;
        
        /* INLINE ASSEMBLY with multiple clobbers - increases register pressure */
        /* Tells compiler these registers are unusable */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More complex calculations after asm clobber */
        f5 = float_ops(f1, f2, f3, d1, d2);
        v8 = (int)f5 + v6 * v7;
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* Array access with complex index calculation */
        int idx = v9 + v10 * 2 - v11;
        /* This creates addressing like array[base + index*scale + large_const] */
        /* May require reloading the address calculation */
        v9 = array[idx + 64] + array[idx + 128] + array[idx + 192];
        
        /* Pointer arithmetic with non-simple offsets */
        v10 = *(ptr + v1 % 64) + *(ptr - (v2 % 32));
        
        /* DIFFERENT SIZED MEMORY ACCESSES - cause mode mismatches */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed size accesses in same expression */
        v11 = byte_ptr[v12 % 256] + short_ptr[v13 % 128] + array[v14 % 64];
        
        /* BITWISE and ARITHMETIC combinations */
        v12 = (v15 & 0xFF00) | ((v16 << 8) & 0xFF);
        v13 = (v17 * v18) + (v19 | v20) - (v1 ^ v2);
        
        /* Update volatile accumulator to prevent dead code elimination */
        accumulator += (double)v1 + (double)v2 + (double)v3 + 
                      (double)v4 + (double)v5 + d1 + d2 + d3 + d4;
        
        /* Another function call */
        float float_res = float_ops(f1, f2, f3, d3, d4);
        accumulator += (double)float_res;
        
        /* More inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11"
        );
        
        /* Final mixed-type expression */
        v14 = (int)((double)v15 * d1) + (int)((float)v16 * f1);
        v15 = v17 + (int)(d2 * 100.0) - (int)(f2 * 50.0f);
        
        /* Use all variables one more time to extend live ranges */
        v16 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        v17 = v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        f1 = f1 * 0.99f + f2 * 0.01f;
        f2 = f2 * 0.98f + f3 * 0.02f;
        f3 = f3 * 0.97f + f4 * 0.03f;
        
        d1 = d1 * 0.99 + d2 * 0.01;
        d2 = d2 * 0.98 + d3 * 0.02;
        
        loop_counter++;
    }
    
    /* Print result to prevent optimization */
    printf("Accumulator: %f\n", accumulator);
    printf("Final values: %d %d %f %f\n", v1, v2, f1, d1);
    
    return 0;
}
