#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
float float_ops(float a, float b, double c, int d) {
    volatile float res = a * b + (float)c + d;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_counter;
    volatile int accumulator = 0;
    volatile int loop_limit = 100;
    
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
    float f6 = (float)(rand() % 100) / 10.0f;
    
    /* Double variables */
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    double d4 = (double)(rand() % 100) / 5.0;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("eax") = v1 + v2;
    register int reg_var2 asm("ebx") = v3 * v4;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addresses */
    int* ptr = array + 128;
    
    /* LOOP with invariant spilling */
    for (loop_counter = 0; loop_counter < loop_limit; loop_counter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v7 ^ v8 | v9 & v10;
        v3 = (v11 << 2) | (v12 >> 3);
        v4 = v13 * v14 - v15;
        
        /* MIXED TYPE OPERATIONS - force register class changes */
        f1 = (float)v1 + f2 * 3.14f;
        f2 = f3 / f4 + (float)(v2 & 0xFF);
        
        /* Integer to float conversions */
        d1 = (double)v3 + d2 * 2.71828;
        d2 = d3 / d4 + (double)(v4 % 100);
        
        /* Float to integer conversions */
        v5 = (int)f1 + (int)d1 * v6;
        v6 = v7 + (int)(f2 * 10.0f);
        
        /* FUNCTION CALL - clobbers registers */
        int call_result = dummy_func(v1, v2, f1, d1, v3);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* This tells GCC many registers are unavailable */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory", "cc"
        );
        
        /* MORE COMPLEX CALCULATIONS with explicit register variables */
        /* Using register variables in contexts that might require reloads */
        int temp1 = reg_var1 * reg_var2;
        float temp2 = (float)reg_var1 + f3;
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* Create addresses that might not be directly usable */
        int index = v8 + v9;
        int offset = index * sizeof(int) + 64;  /* Large offset */
        
        /* Access with complex addressing - may need address reload */
        int array_val = array[index + 16];  /* Non-simple offset */
        int ptr_val = ptr[index / 2];       /* Pointer with index */
        
        /* DIFFERENT SIZED MEMORY ACCESSES */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed size accesses in same expression */
        v7 = byte_ptr[offset] + short_ptr[index] + array_val;
        
        /* BITWISE and ARITHMETIC COMBINATIONS */
        v8 = (v9 & 0xF0) | ((v10 << 3) + (v11 >> 2));
        v9 = (v12 * v13) ^ (v14 | v15);
        
        /* More function calls with mixed arguments */
        float float_res = float_ops(f3, f4, d3, v15);
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v1 + v2 + (int)f1 + (int)d1 + call_result + 
                      array_val + v7 + v8 + v9 + (int)float_res;
        
        /* Shuffle values to create data dependencies across iterations */
        int rotate = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = rotate;
        
        float f_rotate = f1;
        f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = f6; f6 = f_rotate;
        
        double d_rotate = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = d_rotate;
        
        /* Update explicit register variables */
        reg_var1 = v1 + v3;
        reg_var2 = v2 * v4;
    }
    
    /* Use all variables one more time to extend live ranges */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        reg_var1 + reg_var2;
    
    printf("Result: %d (accumulator: %d)\n", final_result, accumulator);
    
    return 0;
}
