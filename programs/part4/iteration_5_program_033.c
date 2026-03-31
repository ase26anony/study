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
    volatile double r = x * y + (double)z / (w + 1);
    return r;
}

int main(void) {
    /* Phase 1: Declare MANY variables to create register pressure */
    /* Integer variables - many will need to stay live */
    volatile int v0 = rand() % 100;
    register int v1 asm("eax") = v0 + 1;  /* Bind to specific reg */
    register int v2 asm("ebx") = v0 + 2;  /* Another specific reg */
    int v3 = v1 * v2;
    int v4 = v3 ^ 0x1234;
    int v5 = v4 << 3;
    int v6 = v5 | 0xFF;
    int v7 = v6 - v3;
    int v8 = v7 / (v1 + 1);
    int v9 = v8 % 77;
    int v10 = v9 & 0xAA;
    int v11 = v10 + v5;
    int v12 = v11 * 3;
    int v13 = v12 ^ v6;
    int v14 = v13 | v7;
    int v15 = v14 - v8;
    int v16 = v15 + v9;
    int v17 = v16 * 2;
    int v18 = v17 ^ 0x55;
    int v19 = v18 << 1;
    
    /* Floating point variables - different register class */
    float f0 = (float)v0 * 0.1f;
    float f1 = f0 + 1.1f;
    float f2 = f1 * 2.2f;
    float f3 = f2 - 3.3f;
    float f4 = f3 / 4.4f;
    
    /* Double variables - more precision, different regs */
    double d0 = (double)v1 * 0.01;
    double d1 = d0 + 1.11;
    double d2 = d1 * 2.22;
    double d3 = d2 - 3.33;
    double d4 = d3 / 4.44;
    
    /* More mixed types */
    volatile int v20 = v19 + (int)f0;
    volatile float f5 = (float)v20 + f4;
    volatile double d5 = (double)f5 + d4;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* Phase 2: Loop with high register pressure and complex operations */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* Complex expression mixing all variables - creates many intermediates */
        int t1 = v1 + v2 * v3 - v4 / (v5 + 1) | v6 & v7;
        float t2 = f0 * f1 + f2 - f3 / (f4 + 1.0f);
        double t3 = d0 * d1 + d2 - d3 / (d4 + 1.0);
        
        /* Type conversions forcing moves between register files */
        float t4 = (float)v8 + (float)v9 * 0.5f;
        double t5 = (double)f0 + (double)f1 * 0.25;
        int t6 = (int)d0 + (int)d1 * 2;
        
        /* Complex addressing mode - non-offsettable */
        /* array[index + constant] where index is complex expression */
        int idx = (v10 + v11 * 2 - v12 / 4) & 0xFF;
        int mem_val1 = array[idx + 16];  /* May need address reload */
        int mem_val2 = array[idx + 32];  /* Another non-simple address */
        int mem_val3 = array[idx + 64];  /* Large offset */
        
        /* Mixed operations with memory accesses */
        v1 = v1 + mem_val1 * 2;
        v2 = v2 - mem_val2 / 3;
        f0 = f0 + (float)mem_val3 * 0.1f;
        
        /* Function call clobbers registers */
        int call_result = dummy_func(v1, v2, f0, d0, v3, v4);
        
        /* Inline assembly with many clobbers */
        /* This tells GCC these regs are unusable, increasing pressure */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More complex expressions after clobber */
        double t7 = complex_calc(d1, d2, f1, v5);
        float t8 = (float)t7 + f2 * 2.0f - f3;
        
        /* Pointer arithmetic with type mixing */
        char *byte_ptr = (char *)array;
        int offset = v6 + v7 * 4 - v8 / 2;
        char byte_val = byte_ptr[offset + 128];  /* Complex address */
        
        /* Different sized accesses in same expression */
        v3 = v3 + (int)byte_val + mem_val1;
        f1 = f1 + (float)((short)mem_val2) * 0.5f;
        
        /* Update accumulator to prevent dead code elimination */
        accumulator += (double)t1 + (double)t2 + t3 + (double)t4 + t5 
                     + (double)t6 + t7 + (double)t8 + (double)byte_val;
        
        /* Shuffle variables to extend live ranges */
        int tmp = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5;
        v5 = v6; v6 = v7; v7 = v8; v8 = v9; v9 = tmp;
        
        float ftmp = f0; f0 = f1; f1 = f2; f2 = f3; f3 = f4; f4 = ftmp;
        
        double dtmp = d0; d0 = d1; d1 = d2; d2 = d3; d3 = d4; d4 = dtmp;
        
        /* Another function call */
        dummy_func(v10, v11, f3, d3, v12, v13);
        
        /* More inline asm with different clobbers */
        asm volatile (
            "# More register clobbering\n"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12"
        );
        
        /* Final complex expression before loop end */
        v10 = (v10 * v11 + v12 - v13 / (v14 + 1)) | 
              (v15 & v16) ^ (v17 | v18) + mem_val1 - mem_val2;
        
        f4 = ((float)v10 * 0.3f + f0 * 2.0f - f1 / (f2 + 0.1f)) * f3;
        
        d4 = ((double)v11 * 0.7 + d0 * 3.0 - d1 / (d2 + 0.01)) * d3;
    }
    
    /* Use results to prevent optimization */
    printf("Result: %f\n", accumulator);
    printf("Final values: v1=%d, f0=%f, d0=%f\n", v1, f0, d0);
    
    return 0;
}
