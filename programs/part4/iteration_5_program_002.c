#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    volatile int loop_limit = 100;
    
    /* MANY LIVE SCALAR VARIABLES (20+ variables) */
    /* Integer variables */
    int a1 = rand() % 100;
    int a2 = rand() % 100;
    int a3 = rand() % 100;
    int a4 = rand() % 100;
    int a5 = rand() % 100;
    int a6 = rand() % 100;
    int a7 = rand() % 100;
    int a8 = rand() % 100;
    int a9 = rand() % 100;
    int a10 = rand() % 100;
    
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
    
    /* More variables to increase pressure */
    int b1 = rand() % 100;
    int b2 = rand() % 100;
    int b3 = rand() % 100;
    int b4 = rand() % 100;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("eax");
    register int reg_var2 asm("ebx");
    register int reg_var3 asm("ecx");
    reg_var1 = rand() % 100;
    reg_var2 = rand() % 100;
    reg_var3 = rand() % 100;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer for non-offsettable addresses */
    int *ptr = array;
    
    /* Volatile accumulator to prevent dead code elimination */
    volatile int accumulator = 0;
    
    /* LOOP with invariant spilling */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS mixing all variables */
        
        /* Integer operations with bitwise and arithmetic */
        a1 = ((a1 * a2) & (a3 | a4)) + ((a5 << 2) ^ (a6 >> 1));
        a2 = (a7 * a8) + ((a9 & 0xFF) | (a10 << 3));
        
        /* Floating point operations */
        f1 = f1 * f2 + f3 - f4 / (f5 + 1.0f);
        f2 = (f6 * f7) + (float)a1 / 100.0f;
        
        /* Double precision operations */
        d1 = d1 * d2 + d3 - d4 / (d5 + 1.0);
        d2 = (double)f1 * 2.0 + d1 / 3.0;
        
        /* Type conversions (int <-> float <-> double) */
        f3 = (float)a2 + f1;
        a3 = (int)f2 + (int)d1;
        d3 = (double)a4 + (double)f3;
        
        /* More complex expressions */
        b1 = ((b1 + b2) * (b3 - b4)) & 0xFFFF;
        b2 = (b1 << 3) | (b3 >> 2);
        
        /* Use register variables in conflicting contexts */
        f4 = (float)reg_var1 + f2;  /* Integer register var in float op */
        a4 = reg_var2 * a5;         /* Register var in computation */
        
        /* FUNCTION CALL to clobber registers */
        int call_result = dummy_func(a1, a2, f1, d1, b1);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* This forces the compiler to save/restore registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* Large offset that might not be directly addressable */
        int idx = a1 + a2 + 128;  /* Could create large displacement */
        int val1 = array[idx];    /* Might need address reload */
        
        /* Pointer arithmetic with complex index */
        int val2 = *(ptr + a3 * 2 + 64);  /* Non-simple address */
        
        /* More type mixing and conversions */
        f5 = f4 + (float)val1 + (float)val2;
        d4 = d3 * (double)f5 + (double)call_result;
        
        /* Use all variables in final expression to extend live ranges */
        accumulator += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
                     + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5
                     + (int)d1 + (int)d2 + (int)d3 + (int)d4
                     + b1 + b2 + b3 + b4
                     + reg_var1 + reg_var2 + reg_var3
                     + val1 + val2 + call_result;
        
        /* Update variables for next iteration to prevent optimization */
        a5 = accumulator % 100;
        a6 = (accumulator >> 1) % 100;
        f6 = (float)(accumulator % 1000) / 10.0f;
        d5 = (double)(accumulator % 1000) / 20.0;
        
        /* Another function call */
        dummy_func(reg_var1, reg_var2, f6, d5, accumulator % 100);
        
        /* More inline assembly */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12"
        );
        
        /* Additional complex expression to use all variables */
        reg_var1 = (reg_var1 * reg_var2 + reg_var3) & 0xFF;
        reg_var2 = (reg_var2 ^ reg_var1) + a7;
        reg_var3 = (reg_var3 | reg_var1) * a8;
        
        /* Mixed size memory accesses */
        char *char_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        /* Different sized accesses in same expression */
        int mixed_access = (int)char_ptr[idx] + 
                          (int)short_ptr[idx % 128] + 
                          array[idx % 256];
        
        accumulator += mixed_access;
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    /* Use all variables one more time to ensure they're live */
    volatile int final_check = 
        v1 + (int)v2 + (int)v3 +
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        b1 + b2 + b3 + b4 +
        (int)f6 + (int)f7 + (int)d5 +
        reg_var1 + reg_var2 + reg_var3;
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
