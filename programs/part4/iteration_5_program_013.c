#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_counter;
    volatile int accumulator = 0;
    volatile int control = 100;
    
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
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    
    /* Floating point variables */
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    float f5 = (float)(rand() % 100) / 10.0f;
    
    /* Double variables */
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    double d3 = (double)(rand() % 100) / 10.0;
    double d4 = (double)(rand() % 100) / 10.0;
    
    /* EXPLICIT REGISTER VARIABLES with conflicting classes */
    /* These create constraints that require reloads */
    register int reg_var1 asm("eax") = v1 + v2;
    register int reg_var2 asm("ebx") = v3 + v4;
    register int reg_var3 asm("ecx") = v5 + v6;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer with complex arithmetic */
    int* ptr = array + 128;
    
    /* Loop with invariant spilling */
    for (loop_counter = 0; loop_counter < control; loop_counter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mixed integer operations */
        int t1 = v1 * v2 + v3 / (v4 + 1);
        int t2 = (v5 & 0xFF) | (v6 << 3);
        int t3 = v7 ^ v8 ^ v9;
        
        /* Integer to float conversions - require register file moves */
        f1 = (float)t1 + f2 * 2.5f;
        f3 = (float)(t2 & 0xFFFF) / 16.0f;
        
        /* Float to double conversions */
        d1 = (double)f1 + d2 * 1.5;
        d2 = (double)f3 + d3 / 3.0;
        
        /* Mixed type operations */
        v10 = (int)(f1 * 10.0f) + t3;
        v11 = (int)d1 * v10;
        
        /* Complex pointer arithmetic with non-offsettable addresses */
        /* This often requires address reloads */
        int idx = v12 + loop_counter * 4 + 128;  /* Large offset */
        v13 = array[idx] + ptr[loop_counter * 2 - 64];
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloading due to register class constraints */
        v14 = reg_var1 * reg_var2 + reg_var3;
        reg_var1 = v14 + loop_counter;  /* Update register variable */
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_function(v1, v2, f1, d1, v10, v11);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* Forces compiler to save/restore registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More complex calculations after assembly clobber */
        /* Compiler must reload values after clobber */
        v15 = call_result + v13 + (int)(f4 * 100.0f);
        f5 = (float)v15 / (float)(loop_counter + 1);
        d4 = d3 * 2.0 + (double)v15;
        
        /* Different sized memory accesses */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        v1 = byte_ptr[idx] + short_ptr[idx/2] + array[idx];
        
        /* Bitwise and arithmetic combinations */
        v2 = ((v3 << 2) & 0xFF00) | ((v4 >> 1) & 0x00FF);
        v3 = v5 * v6 + (v7 & v8) ^ v9;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v15 + (int)f5 + (int)d4;
        
        /* Shuffle values to create data dependencies */
        int temp = v1;
        v1 = v2;
        v2 = v3;
        v3 = v4;
        v4 = v5;
        v5 = v6;
        v6 = v7;
        v7 = v8;
        v8 = v9;
        v9 = temp;
        
        /* Rotate floating point values */
        float ftemp = f1;
        f1 = f2;
        f2 = f3;
        f3 = f4;
        f4 = f5;
        f5 = ftemp;
        
        /* Rotate double values */
        double dtemp = d1;
        d1 = d2;
        d2 = d3;
        d3 = d4;
        d4 = dtemp;
    }
    
    /* Use all variables one more time to extend live ranges */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        accumulator;
    
    printf("Result: %d\n", final_result);
    return final_result % 100;
}
