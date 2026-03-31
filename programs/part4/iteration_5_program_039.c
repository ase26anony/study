#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
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
    volatile int loop_limit = 100;
    volatile double final_acc = 0.0;
    
    /* MANY LIVE SCALAR VARIABLES (30+ variables) */
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
    
    /* More integers with volatile to force reloads */
    volatile int v11 = rand() % 100;
    volatile int v12 = rand() % 100;
    volatile int v13 = rand() % 100;
    
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
    double d6 = (double)rand() / RAND_MAX;
    
    /* More mixed types */
    volatile float f6 = (float)rand() / RAND_MAX;
    volatile double d7 = (double)rand() / RAND_MAX;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    int v16 = rand() % 100;
    int v17 = rand() % 100;
    int v18 = rand() % 100;
    int v19 = rand() % 100;
    int v20 = rand() % 100;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    /* These will bind to specific registers, creating pressure */
    register int reg_var1 asm("eax") = v1;
    register int reg_var2 asm("ebx") = v2;
    register int reg_var3 asm("ecx") = v3;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer that will be used with non-offsettable addresses */
    int* ptr = array;
    
    /* LOOP with high register pressure */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS mixing all variables */
        
        /* Integer operations with bitwise and arithmetic */
        v1 = ((v1 & v2) | (v3 << 2)) + v4 * v5 - v6 / (v7 + 1);
        v2 = (v8 ^ v9) * (v10 + v11) - (v12 & v13);
        v3 = v14 + ((v15 | v16) << 1) - (v17 ^ v18) * v19;
        
        /* Mixed integer/float conversions */
        f1 = (float)v1 + f2 * (float)v2 - (float)(v3 & 0xFF);
        f2 = f3 + (float)(v4 * v5) / f4;
        
        /* Double precision with conversions */
        d1 = (double)v6 + d2 * (double)v7 - (double)(v8 | v9);
        d2 = d3 + (double)(v10 * v11) / d4;
        
        /* Type mixing and size changes */
        v4 = (int)f1 + (int)d1 + (v5 & 0xFFFF);
        v5 = (short)v6 + (char)v7 + (int)(f2 * 100.0f);
        
        /* FUNCTION CALL to clobber registers */
        int call_result = dummy_function(v1, v2, f1, d1, v3, v4);
        
        /* INLINE ASSEMBLY with many clobbered registers */
        /* This forces the compiler to work around these registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory"
        );
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = v1 + v2;
        int idx2 = v3 + v4;
        
        /* Non-simple addressing: array[index + constant] where 
           the offset might be too large for direct addressing */
        int val1 = array[idx1 + 64];  /* Large constant offset */
        int val2 = array[idx2 + 128]; /* Another large offset */
        
        /* More pointer arithmetic */
        int* ptr2 = ptr + idx1;
        int* ptr3 = ptr2 + idx2;
        
        /* Access with scaling */
        v6 = ptr[idx1 * 2] + ptr[idx2 * 4];
        v7 = *(ptr3 - 32) + *(ptr3 + 16);
        
        /* More type conversions and mixed operations */
        f3 = (float)val1 + (float)val2 * 0.5f;
        d3 = (double)call_result + d1 * 0.25 - d2 / 3.0;
        
        /* Use explicit register variables in conflicting contexts */
        /* These are bound to integer registers but used in FP contexts */
        float temp_f = (float)reg_var1 + f3;
        double temp_d = (double)reg_var2 + d3;
        
        /* Force the register variables to be used and updated */
        reg_var1 = (int)temp_f;
        reg_var2 = (int)temp_d;
        reg_var3 = val1 + val2;
        
        /* Another function call with mixed arguments */
        double dres = complex_op(v8, f4, d4, v9);
        
        /* More complex expressions extending live ranges */
        v8 = v9 + v10 * v11 - v12 / (v13 + 1);
        v9 = (v14 & v15) | (v16 << v17);
        v10 = v18 + v19 * v20 - call_result;
        
        f4 = f5 + (float)v8 * 0.3f - (float)v9 / 2.0f;
        f5 = (float)v10 + f6 * 1.5f;
        
        d4 = d5 + (double)v11 * 0.7 - (double)v12 / 4.0;
        d5 = d6 + d7 * 2.0;
        
        /* Update volatile accumulator to prevent elimination */
        final_acc += (double)v1 + (double)v2 + (double)v3 +
                    (double)f1 + (double)f2 + (double)f3 +
                    d1 + d2 + d3 + d4 + d5 +
                    (double)val1 + (double)val2 +
                    (double)call_result + dres;
        
        /* Another inline asm to clobber different registers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
        
        /* Shuffle values to extend live ranges across iterations */
        int tmp = v1;
        v1 = v20;
        v20 = v19;
        v19 = v18;
        v18 = v17;
        v17 = v16;
        v16 = v15;
        v15 = v14;
        v14 = v13;
        v13 = v12;
        v12 = v11;
        v11 = v10;
        v10 = v9;
        v9 = v8;
        v8 = v7;
        v7 = v6;
        v6 = v5;
        v5 = v4;
        v4 = v3;
        v3 = v2;
        v2 = tmp;
        
        float ftmp = f1;
        f1 = f5;
        f5 = f4;
        f4 = f3;
        f3 = f2;
        f2 = ftmp;
        
        double dtmp = d1;
        d1 = d6;
        d6 = d5;
        d5 = d4;
        d4 = d3;
        d3 = d2;
        d2 = dtmp;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final accumulator: %f\n", (double)final_acc);
    
    /* Use all variables one more time to extend live ranges */
    volatile int check = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 + (int)d7;
    
    printf("Check sum: %d\n", check);
    
    return 0;
}
