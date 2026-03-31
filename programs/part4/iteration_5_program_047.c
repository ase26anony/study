#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / (w + 1);
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int trigger = 100;
    volatile double final_acc = 0.0;
    
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
    /* These may conflict with register allocator's choices */
    register int reg_var1 asm("r12") = v1 + v2;  /* Try to bind to specific reg */
    register int reg_var2 asm("r13") = v3 * v4;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Pointer that will be used with non-offsettable addresses */
    int* ptr = array + 128;
    
    /* LOOP with invariant spilling */
    volatile int loop_limit = 50;  /* volatile prevents loop unrolling */
    for (int iter = 0; iter < loop_limit; iter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer operations */
        int t1 = v1 + v2 * v3 - v4 / (v5 + 1);
        int t2 = v6 & v7 | v8 ^ v9 << 2;
        int t3 = (v10 + v11) * (v12 - v13) / (v14 | 1);
        int t4 = v15 % (v16 + 1) + v17 * v18 - v19;
        
        /* Mixed integer/float operations - may require moves between reg files */
        float ft1 = f1 * (float)t1 + f2 / (float)(t2 + 1);
        float ft2 = (float)(t3 & 0xFF) * f3 - f4 * (float)t4;
        
        /* Double precision operations */
        double dt1 = d1 * (double)t1 + d2 / (double)(t2 + 1);
        double dt2 = (double)ft1 * d3 - d4 * (double)ft2;
        
        /* TYPE CONVERSIONS - force register moves */
        double mixed1 = (double)v1 * f1 + (double)(v2 & v3);
        float mixed2 = (float)d1 * (float)v4 - (float)(v5 | v6);
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = (v7 + v8 * 2) & 0xFF;
        int idx2 = (v9 + v10 * 4) & 0xFF;
        int idx3 = (v11 + v12 * 8) & 0xFF;
        
        /* Non-simple addressing: array[base + index*scale + constant] */
        /* Compiler may need to reload the address calculation */
        int mem1 = array[idx1 + 64];      /* offset may be too large */
        int mem2 = array[idx2 * 2 + 32];  /* scaled index with offset */
        int mem3 = ptr[idx3 - 64];        /* pointer with index */
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_func(t1, t2, ft1, dt1, mem1, mixed2);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS - increase register pressure */
        /* Tell compiler many registers are unavailable */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11"
        );
        
        /* MORE COMPLEX EXPRESSIONS after asm clobber */
        /* Variables need to be reloaded after asm clobbers registers */
        int t5 = v20 + call_result * v19 - v18 / (v17 + 1);
        float ft3 = f5 * (float)t5 + (float)mem2 * f1;
        double dt3 = d5 * (double)t5 + (double)mem3 * d1;
        
        /* Another function call with different types */
        double call_result2 = complex_op(t5, ft3, dt3, mem1);
        
        /* Use explicit register variables in conflicting contexts */
        /* These are bound to integer regs but used in FP context */
        double conflict1 = (double)reg_var1 * d2;  /* integer reg var in FP op */
        float conflict2 = (float)reg_var2 * f3;    /* another conflict */
        
        /* Update all variables to keep them live across iterations */
        v1 = v2 + t1;
        v2 = v3 + t2;
        v3 = v4 + t3;
        v4 = v5 + t4;
        v5 = v6 + t5;
        v6 = v7 + mem1;
        v7 = v8 + mem2;
        v8 = v9 + mem3;
        v9 = v10 + call_result;
        v10 = v11 + (int)call_result2;
        
        v11 = v12 + (int)ft1;
        v12 = v13 + (int)ft2;
        v13 = v14 + (int)ft3;
        v14 = v15 + (int)conflict2;
        v15 = v16 + mem1;
        v16 = v17 + mem2;
        v17 = v18 + mem3;
        v18 = v19 + t1;
        v19 = v20 + t2;
        v20 = t3 + t4;
        
        f1 = f2 + ft1;
        f2 = f3 + ft2;
        f3 = f4 + ft3;
        f4 = f5 + conflict2;
        f5 = (float)(t1 % 100) / 100.0f;
        
        d1 = d2 + dt1;
        d2 = d3 + dt2;
        d3 = d4 + dt3;
        d4 = d5 + conflict1;
        d5 = (double)(t2 % 100) / 100.0;
        
        /* Update volatile accumulator to prevent elimination */
        final_acc += (double)t1 + (double)t2 + ft1 + ft2 + dt1 + dt2 + 
                    (double)mem1 + (double)mem2 + (double)mem3 +
                    (double)call_result + call_result2 + conflict1 + (double)conflict2;
        
        /* Another asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final accumulator: %f\n", (double)final_acc);
    
    /* Return based on final state to ensure all variables matter */
    return (v1 + v2 + v3 + (int)f1 + (int)d1) % 256;
}
