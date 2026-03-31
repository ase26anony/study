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
    volatile double tmp = x * y + z - w;
    return tmp;
}

int main(void) {
    /* Phase 1: Declare MANY variables to create register pressure */
    /* Integer variables - many will be live simultaneously */
    volatile int v0 = rand() % 100;
    register int v1 asm("r10") = rand() % 100;  /* Bind to specific reg */
    register int v2 asm("r11") = rand() % 100;  /* Another specific reg */
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
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer variables */
    int *ptr1 = &array[0];
    int *ptr2 = &array[128];
    volatile int *volatile_ptr = &array[64];
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* Phase 2: Complex loop with high register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        /* Step 1: Complex integer expressions with many live variables */
        /* This creates long dependency chains */
        v3 = v0 + v1 * v2 - v3 / (v4 + 1);
        v4 = v5 | (v6 & v7) ^ (v8 << 2);
        v5 = v9 + v10 - v11 * v12;
        v6 = v13 ^ v14 | v15;
        v7 = (v0 << 3) + (v1 >> 2) - (v2 & 0xFF);
        
        /* Step 2: Type conversions - force moves between register files */
        f1 = (float)v3 + f0 * 2.0f;
        f2 = (float)v4 / 3.0f - f1;
        d0 = (double)v5 + (double)f1 * d1;
        d2 = (double)v6 / d3 + (double)f2;
        
        /* Step 3: Complex addressing modes with non-offsettable addresses */
        /* array[index + constant] where index is in a register */
        int idx1 = v7 & 0x7F;  /* 0-127 */
        int idx2 = v8 & 0x7F;
        
        /* These addresses may need reloading due to complex offset */
        int val1 = array[idx1 + 64];    /* offset 64 may be too large */
        int val2 = array[idx2 + 128];   /* offset 128 likely too large */
        
        /* More complex: array[array[index]] - indirect addressing */
        int indirect_idx = array[idx1] & 0xFF;
        int val3 = array[indirect_idx];
        
        /* Step 4: Function call clobbers registers */
        int func_result = dummy_func(v0, v1, f0, d0, v2, v3);
        
        /* Step 5: Inline assembly with many clobbers */
        /* Force compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly with many clobbers\n"
            "nop\n"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Step 6: More mixed-type operations after clobber */
        f3 = f1 * f2 + (float)val1 - (float)val2;
        d3 = d0 * d1 + (double)val3 - d2;
        
        /* Step 7: Use register-bound variables in conflicting contexts */
        /* v1 is bound to r10 (integer reg), use in float context */
        float temp_float = (float)v1 + f3;
        /* Use in pointer arithmetic (requires address reg on some arch) */
        int *temp_ptr = ptr1 + v1;  /* v1 in register, but may need reload */
        
        /* Step 8: Access through volatile pointer - can't optimize */
        int volatile_val = *volatile_ptr;
        *volatile_ptr = volatile_val + func_result;
        
        /* Step 9: Another function call with different types */
        double d_result = complex_calc(d0, d1, f0, v0);
        
        /* Step 10: Update accumulator to prevent dead code elimination */
        accumulator += v0 + v1 + v2 + v3 + v4 + (int)f0 + (int)d_result;
        
        /* Step 11: Rotate values to extend live ranges */
        int temp = v0;
        v0 = v15; v15 = v14; v14 = v13; v13 = v12;
        v12 = v11; v11 = v10; v10 = v9; v9 = v8;
        v8 = v7; v7 = v6; v6 = v5; v5 = v4;
        v4 = v3; v3 = v2; v2 = v1; v1 = temp;
        
        float ftemp = f0;
        f0 = f4; f4 = f3; f3 = f2; f2 = f1; f1 = ftemp;
        
        double dtemp = d0;
        d0 = d4; d4 = d3; d3 = d2; d2 = d1; d1 = dtemp;
    }
    
    /* Phase 3: Final output to prevent optimization */
    printf("Final accumulator: %d\n", accumulator);
    printf("Values: v0=%d, v1=%d, f0=%.2f, d0=%.2f\n", v0, v1, f0, d0);
    
    return 0;
}
