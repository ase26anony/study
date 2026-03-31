#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_calc(double x, double y, int z, float w) {
    volatile double res = (x * y) / (z + 1) + w;
    return res;
}

int main(void) {
    /* SEED RANDOM FOR VARIABLE INITIALIZATION */
    srand(42);
    
    /* VOLATILE VARIABLES TO PREVENT OPTIMIZATION */
    volatile int loop_counter;
    volatile int accumulator = 0;
    volatile int limit = 100;
    
    /* DECLARE MANY INTEGER VARIABLES TO CREATE REGISTER PRESSURE */
    /* Initialize with rand() to create data dependencies */
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
    int v16 = rand() % 100;
    int v17 = rand() % 100;
    int v18 = rand() % 100;
    int v19 = rand() % 100;
    int v20 = rand() % 100;
    
    /* FLOATING POINT VARIABLES FOR MIXED OPERATIONS */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    float f6 = (float)rand() / RAND_MAX;
    
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES WITH POTENTIAL CONFLICTS */
    /* These will bind to specific registers, creating constraints */
    register int reg_var1 asm("eax") = v1 + v2;
    register int reg_var2 asm("ebx") = v3 * v4;
    register float reg_float asm("xmm0") = f1 + f2;
    
    /* ARRAY FOR COMPLEX ADDRESSING MODES */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* POINTERS FOR NON-OFFSETTABLE ADDRESS CALCULATIONS */
    int* ptr1 = &array[0];
    int* ptr2 = &array[128];
    
    /* MAIN LOOP WITH HIGH REGISTER PRESSURE */
    for (loop_counter = 0; loop_counter < limit; loop_counter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS WITH MIXED TYPES */
        /* Force many intermediate values to be kept in registers */
        
        /* Integer expressions with bitwise and arithmetic ops */
        int t1 = v1 + v2 * v3;
        int t2 = (v4 & v5) | (v6 << 2);
        int t3 = v7 ^ v8 + v9 * v10;
        int t4 = (v11 | v12) & (v13 ^ v14);
        int t5 = v15 + (v16 << 1) - (v17 >> 2);
        int t6 = v18 * v19 / (v20 + 1);
        
        /* Mixed integer/float conversions */
        float ft1 = (float)t1 + f1;
        float ft2 = (float)t2 * f2;
        double dt1 = (double)t3 + d1;
        double dt2 = (double)t4 * d2;
        
        /* Complex floating point expressions */
        f3 = ft1 * ft2 + f3;
        f4 = (f4 + ft1) / (ft2 + 1.0f);
        d3 = dt1 * dt2 - d3;
        d4 = (d4 + dt1) / (dt2 + 1.0);
        
        /* TYPE CONVERSIONS THAT REQUIRE REGISTER MOVES */
        int from_float1 = (int)ft1;
        int from_float2 = (int)ft2;
        int from_double1 = (int)dt1;
        int from_double2 = (int)dt2;
        
        float from_int1 = (float)from_float1;
        float from_int2 = (float)from_float2;
        double from_int3 = (double)from_double1;
        double from_int4 = (double)from_double2;
        
        /* COMPLEX ADDRESSING WITH NON-SIMPLE OFFSETS */
        /* These often require address reloads */
        int idx1 = v1 + v2 + loop_counter;
        int idx2 = v3 * v4 - loop_counter;
        
        /* Non-offsettable addresses: array[base + index*scale + constant] */
        int val1 = array[idx1 * 2 + 16];  /* Large offset */
        int val2 = array[idx2 * 4 + 32];  /* Complex addressing */
        
        /* More complex: array[array[index] + constant] */
        int val3 = array[array[idx1 % 256] + 64];
        int val4 = array[array[idx2 % 256] + 128];
        
        /* FUNCTION CALL TO CLOBBER REGISTERS */
        int func_result = dummy_func(v1, v2, f1, d1, v3, f2);
        
        /* INLINE ASSEMBLY WITH MANY CLOBBERED REGISTERS */
        /* Forces compiler to save/restore values */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            "mov $0, %%eax\n"
            "mov $0, %%ebx\n"
            "mov $0, %%ecx\n"
            "mov $0, %%edx\n"
            "pxor %%xmm0, %%xmm0\n"
            "pxor %%xmm1, %%xmm1\n"
            "pxor %%xmm2, %%xmm2\n"
            :
            :
            : "eax", "ebx", "ecx", "edx", "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* MORE COMPLEX EXPRESSIONS AFTER ASSEMBLY CLOBBER */
        /* Compiler must reload values after registers were clobbered */
        v1 = t1 + val1 + func_result;
        v2 = t2 - val2 * from_float1;
        v3 = t3 ^ val3 + from_float2;
        v4 = t4 | val4 - from_double1;
        
        /* Floating point ops after clobber */
        f1 = ft1 * from_int1 + f1;
        f2 = ft2 / from_int2 - f2;
        d1 = dt1 + from_int3 * d1;
        d2 = dt2 - from_int4 / d2;
        
        /* USE EXPLICIT REGISTER VARIABLES IN CONFLICTING CONTEXTS */
        /* May require reloads due to register class conflicts */
        int use_reg1 = reg_var1 + v5;  /* Integer register variable */
        float use_reg2 = reg_float * f3;  /* Float register variable */
        
        /* Convert between register-bound variables */
        float conflict1 = (float)reg_var1 + reg_float;  /* Mixing register classes */
        int conflict2 = (int)reg_float * reg_var2;      /* Another mix */
        
        /* POINTER ARITHMETIC WITH COMPLEX EXPRESSIONS */
        int offset = (v1 * v2 + v3 - v4) / 2;
        int* complex_ptr = ptr1 + offset + 8;  /* Non-simple offset */
        int ptr_val = *complex_ptr;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v1 + v2 + v3 + v4 + (int)f1 + (int)f2 + 
                      (int)d1 + (int)d2 + val1 + val2 + ptr_val +
                      use_reg1 + (int)use_reg2 + (int)conflict1 + conflict2;
        
        /* SECOND FUNCTION CALL WITH DIFFERENT ARGUMENTS */
        double dresult = complex_calc(d1, d2, v5, f3);
        accumulator += (int)dresult;
        
        /* ROTATE VARIABLES TO EXTEND LIVE RANGES */
        int temp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = v16;
        v16 = v17; v17 = v18; v18 = v19; v19 = v20; v20 = temp;
        
        float ftemp = f1;
        f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = f6; f6 = ftemp;
        
        double dtemp = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = dtemp;
    }
    
    printf("Final accumulator: %d\n", accumulator);
    return 0;
}
