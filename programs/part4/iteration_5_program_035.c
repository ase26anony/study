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
    volatile double final_acc = 0.0;
    volatile int limit = 100;  /* Loop iterations */
    
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
    
    /* Double variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    double d6 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with conflicting usage */
    /* Force specific registers, then use in conflicting contexts */
    register int reg_ax asm("ax") = v1;  /* Try to bind to ax/rax/eax */
    register int reg_bx asm("bx") = v2;  /* Try to bind to bx/rbx/ebx */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* MAIN LOOP - Creates sustained register pressure */
    for (loop_counter = 0; loop_counter < limit; loop_counter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - Extend live ranges */
        /* Mix integer operations */
        v1 = v2 + v3 * v4 - (v5 & v6) | (v7 << 2);
        v2 = v3 ^ v4 + (v8 >> 1) * v9;
        v3 = (v10 + v11) * (v12 - v13) / (v14 | 1);
        v4 = v15 & 0xFF + v1 * v2;
        
        /* Integer to float conversions (require moves between register files) */
        f1 = (float)v1 + f2 * (float)v3;
        f2 = (float)(v2 & 0xFFF) / f1 + f3;
        
        /* Float to double conversions */
        d1 = (double)f1 * d2 + (double)(v4 % 100);
        d2 = d3 / (double)f2 - d4;
        
        /* Mixed-type expressions */
        d3 = d1 * (double)v5 + (double)f3 * d5;
        d4 = (double)(v6 + v7) - d2 * (double)f4;
        
        /* Use explicit register variables in conflicting contexts */
        /* These may require reloads due to register class conflicts */
        f3 = (float)reg_ax + f4;  /* Integer register used in FP op */
        f4 = f5 * (float)reg_bx;
        
        /* Non-offsettable memory addressing */
        /* array[index + large_constant] creates complex addresses */
        int idx1 = v8 % 64;
        int idx2 = v9 % 64;
        
        /* Complex addressing: array[idx1 + 100] - non-simple offset */
        v5 = array[idx1 + 100] + array[idx2 + 150];  /* May need address reload */
        v6 = *(ptr + idx1 * 2 - 50);  /* Complex pointer arithmetic */
        
        /* Different sized memory accesses */
        char* char_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed-size accesses in same expression */
        v7 = char_ptr[idx1 * 4] + short_ptr[idx2 * 2] + array[idx1];
        
        /* FUNCTION CALL - Clobbers caller-saved registers */
        v8 = dummy_func(v1, v2, f1, d1, v3, v4);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS */
        /* Force compiler to work around unavailable registers */
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
        
        /* MORE COMPLEX CALCULATIONS after clobber */
        v9 = v10 * v11 + (v12 >> v13) | (v14 & v15);
        v10 = (v1 + v2) * (v3 - v4) / (v5 | 1);
        
        /* Type conversions and mixed operations */
        d5 = complex_calc(d1, d2, f1, v6);
        d6 = (double)v7 * d3 + (double)(v8 & 0xFF) / d4;
        
        /* Bitwise and arithmetic combinations */
        v11 = (v9 << 3) + (v10 * 7) & 0xFFFF;
        v12 = (v11 | 0xFF00) * (v13 + 1) - (v14 ^ 0x00FF);
        
        /* More function calls */
        v13 = dummy_func(v15, v14, f3, d5, v12, v11);
        
        /* Update volatile accumulator (prevents dead code elimination) */
        final_acc += (double)v1 + d1 + (double)v2 + d2 + 
                    (double)v3 + d3 + (double)v4 + d4 +
                    (double)v5 + d5 + (double)v6 + d6 +
                    (double)f1 + (double)f2 + (double)f3 + (double)f4;
        
        /* Shuffle values to create data dependencies across iterations */
        int temp = v1;
        v1 = v15;
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
        v2 = temp;
        
        float ftemp = f1;
        f1 = f6;
        f6 = f5;
        f5 = f4;
        f4 = f3;
        f3 = f2;
        f2 = ftemp;
        
        double dtemp = d1;
        d1 = d6;
        d6 = d5;
        d5 = d4;
        d4 = d3;
        d3 = d2;
        d2 = dtemp;
    }
    
    printf("Final accumulator: %f\n", (double)final_acc);
    printf("Variables: %d %d %d %f %f %lf %lf\n", 
           v1, v2, v3, f1, f2, d1, d2);
    
    return 0;
}
