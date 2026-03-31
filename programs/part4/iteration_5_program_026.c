#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed random for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MANY LIVE SCALAR VARIABLES (25+ variables) */
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
    
    /* Floating point variables */
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    float f5 = (float)(rand() % 100) / 10.0f;
    
    /* Double precision variables */
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    double d3 = (double)(rand() % 100) / 10.0;
    double d4 = (double)(rand() % 100) / 10.0;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("ax") = v1 + v2;  /* Try to bind to ax */
    register int reg_var2 asm("bx") = v3 + v4;  /* Try to bind to bx */
    
    /* Array for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* LOOP with high register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        
        /* Mixed integer operations */
        int temp1 = v1 * v2 + v3 / (v4 + 1) | v5 & v6 ^ v7;
        int temp2 = (v8 << 2) | (v9 >> 3) + v10 * temp1;
        
        /* Integer to float conversions (require register file moves) */
        float ftemp1 = (float)temp1 + f1 * 2.0f;
        float ftemp2 = (float)temp2 / f2 + f3 - f4;
        
        /* Float to double conversions */
        double dtemp1 = (double)ftemp1 + d1 * 3.14159;
        double dtemp2 = (double)ftemp2 - d2 / 1.618;
        
        /* Mixed size memory accesses */
        char char_val = (char)(temp1 & 0xFF);
        short short_val = (short)(temp2 & 0xFFFF);
        int int_val = temp1 + temp2;
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* This often requires reloading the address calculation */
        int idx1 = v1 + v2 + loop;
        int idx2 = v3 + v4 + loop * 2;
        
        /* Large offset that may not be directly addressable */
        int val1 = array[idx1 + 64];  /* idx1 + 64 might be too large offset */
        int val2 = array[idx2 + 128]; /* idx2 + 128 might be too large offset */
        
        /* Pointer arithmetic with explicit register variables */
        int* addr1 = ptr + reg_var1;  /* Requires address reload */
        int* addr2 = ptr + reg_var2;  /* Requires address reload */
        
        int val3 = *addr1;
        int val4 = *addr2;
        
        /* FUNCTION CALL - clobbers registers, forces save/restore */
        int call_result = dummy_function(v1, v2, f1, d1, v3, f2);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* Tells compiler many registers are unavailable */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* MORE COMPLEX CALCULATIONS after asm clobber */
        double dtemp3 = dtemp1 * dtemp2 + d3 - d4;
        float ftemp3 = ftemp1 + ftemp2 * f5;
        
        /* Type conversions and mixed operations */
        int temp3 = (int)dtemp3 + (int)ftemp3 + char_val + short_val;
        
        /* Bitwise and arithmetic combinations */
        int temp4 = (temp3 & 0x0F0F0F0F) * 3 + 
                   ((temp3 >> 4) | 0x01010101) - 
                   (val1 ^ val2) + (val3 & val4);
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += temp4 + call_result;
        
        /* Modify variables to create data dependencies across iterations */
        v1 = v2 + 1;
        v2 = v3 - 1;
        v3 = v4 * 2;
        v4 = v5 / 2;
        v5 = v6 | v7;
        v6 = v7 & v8;
        v7 = v8 ^ v9;
        v8 = v9 << 1;
        v9 = v10 >> 1;
        v10 = temp4;
        
        f1 = f2 + 1.0f;
        f2 = f3 - 1.0f;
        f3 = f4 * 2.0f;
        f4 = f5 / 2.0f;
        f5 = ftemp3;
        
        d1 = d2 + 1.0;
        d2 = d3 - 1.0;
        d3 = d4 * 2.0;
        d4 = dtemp3;
        
        /* Update explicit register variables */
        reg_var1 = v1 + v3;
        reg_var2 = v2 + v4;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final accumulator: %d\n", accumulator);
    printf("Values: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
    
    return 0;
}
