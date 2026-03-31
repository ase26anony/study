/* reload_stress.c - Program to stress GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_calc(double x, double y, float z, int w) {
    volatile double r = x * y + z - w;
    return r;
}

int main(void) {
    /* Phase 1: Declare MANY variables to create register pressure */
    /* Integer variables - many will need to stay live */
    volatile int v0 = rand() % 100;
    register int v1 asm("eax") = rand() % 100;  /* Bind to specific reg */
    register int v2 asm("ebx") = rand() % 100;  /* Another specific reg */
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
    
    /* Floating point variables - mixed precision */
    volatile float f0 = (float)rand() / RAND_MAX;
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    double d0 = (double)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* Pointer variables for complex addressing */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = rand() % 1000;
    }
    int* ptr1 = &array[0];
    int* ptr2 = &array[50];
    
    /* More variables to increase pressure */
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    int v16 = rand() % 100;
    int v17 = rand() % 100;
    int v18 = rand() % 100;
    int v19 = rand() % 100;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* Phase 2: Complex loop with high register pressure */
    for (int iteration = 0; iteration < loop_limit; iteration++) {
        /* Step 1: Complex integer calculations with mixed operations */
        int temp1 = v0 + v1 * v2 - v3;
        int temp2 = (v4 & v5) | (v6 << 2);
        int temp3 = v7 * v8 + (v9 >> 1);
        int temp4 = (v10 ^ v11) + (v12 & 0xFF);
        
        /* Force use of register-bound variables in FP context */
        float f_temp1 = (float)v1 + f0;  /* v1 is register-bound to eax */
        float f_temp2 = (float)v2 * f1;  /* v2 is register-bound to ebx */
        
        /* Step 2: Type conversions that require moves between reg files */
        double d_temp1 = (double)temp1 + d0;
        double d_temp2 = (double)temp2 * d1;
        float f_temp3 = (float)d_temp1 + f2;
        
        /* Step 3: Complex addressing modes - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = v3 + v4 + 12345;  /* Large constant may be non-offsettable */
        int idx2 = v5 * 2 + 6789;    /* Another large offset */
        
        /* Access with complex addressing - may need reload */
        int mem1 = array[idx1 % 100];  /* idx1 may need reloading */
        int mem2 = array[idx2 % 100];  /* idx2 may need reloading */
        
        /* Step 4: Function call clobbers registers */
        int func_result = dummy_func(v6, v7, f_temp1, d_temp1, mem1);
        
        /* Step 5: Inline assembly with many clobbers */
        /* This tells GCC these registers are unavailable */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Step 6: More calculations after registers are clobbered */
        /* Compiler must reload values */
        v0 = v0 + func_result;
        v1 = v1 * 2 + mem2;  /* v1 is register-bound, needs reload after asm */
        v2 = v2 / 3 + temp3; /* v2 is register-bound, needs reload */
        
        /* Mixed type operations */
        d_temp1 = d_temp1 * (double)v1 + (double)v2; /* Using register-bound vars */
        f_temp2 = f_temp2 + (float)d_temp1;
        
        /* Another function call with mixed arguments */
        double d_result = complex_calc(d2, d3, f_temp2, v8);
        
        /* Step 7: More complex integer expressions */
        v3 = (v3 ^ v4) + (v5 & v6);
        v4 = v4 * v5 - v6;
        v5 = (v7 << 3) | (v8 >> 2);
        v6 = v9 + v10 * v11;
        
        /* Step 8: Floating point with integer conversions */
        f3 = f3 + (float)(v12 + v13);
        d4 = d4 * (double)(v14 - v15);
        
        /* Step 9: Pointer arithmetic that may need reloads */
        ptr1 = &array[(v16 + iteration) % 100];
        ptr2 = &array[(v17 * 2 + 5432) % 100];  /* Large constant offset */
        
        /* Step 10: Access through pointers */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Step 11: Update accumulator to prevent elimination */
        accumulator += (double)val1 * d_result + (double)val2;
        
        /* Step 12: More register pressure with all variables */
        v7 = v7 + v8 * v9 - v10;
        v8 = (v11 & v12) | (v13 << 1);
        v9 = v14 * v15 + (v16 >> 2);
        v10 = (v17 ^ v18) + (v19 & 0x7F);
        
        f4 = f4 * 1.01f + (float)v7;
        d5 = d5 / 1.01 + (double)v8;
        
        /* Another inline asm to clobber different registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12"
        );
        
        /* Final calculations requiring reloads after asm clobber */
        v11 = v11 + v12 - v13;
        v12 = v12 * v14 / (v15 + 1);
        f0 = f0 + (float)v16;
        d0 = d0 * (double)v17;
    }
    
    /* Phase 3: Final output to prevent dead code elimination */
    printf("Final accumulator: %f\n", accumulator);
    printf("v0=%d, v1=%d, v2=%d\n", v0, v1, v2);
    printf("f0=%f, d0=%f\n", f0, d0);
    
    return 0;
}
