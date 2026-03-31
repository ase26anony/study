#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / (w + 1);
    return res;
}

int main(void) {
    /* Phase 1: Declare many variables to create register pressure */
    
    /* Integer variables - many will need to stay live */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    
    /* More non-volatile integers with complex live ranges */
    int i11 = rand() % 100;
    int i12 = rand() % 100;
    int i13 = rand() % 100;
    int i14 = rand() % 100;
    int i15 = rand() % 100;
    int i16 = rand() % 100;
    int i17 = rand() % 100;
    int i18 = rand() % 100;
    int i19 = rand() % 100;
    int i20 = rand() % 100;
    
    /* Floating point variables - different register class */
    volatile float f1 = (float)(rand() % 100) / 10.0f;
    volatile float f2 = (float)(rand() % 100) / 10.0f;
    volatile float f3 = (float)(rand() % 100) / 10.0f;
    volatile float f4 = (float)(rand() % 100) / 10.0f;
    volatile float f5 = (float)(rand() % 100) / 10.0f;
    
    /* Double precision variables */
    double d1 = (double)(rand() % 100) / 3.0;
    double d2 = (double)(rand() % 100) / 3.0;
    double d3 = (double)(rand() % 100) / 3.0;
    double d4 = (double)(rand() % 100) / 3.0;
    double d5 = (double)(rand() % 100) / 3.0;
    
    /* Explicit register variables - will conflict with natural allocation */
    register int reg_var1 asm("r12");  /* Try to bind to specific reg */
    register int reg_var2 asm("r13");
    reg_var1 = rand() % 100;
    reg_var2 = rand() % 100;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int j = 0; j < 256; j++) {
        array[j] = rand() % 1000;
    }
    
    /* Pointer that will be used with non-offsettable addresses */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* Phase 2: Complex loop with high register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* Complex expression 1: Mix integer and float operations */
        int temp1 = v1 + v2 * v3 - v4 / (v5 + 1);
        float temp2 = f1 * f2 + (float)temp1 / f3;
        double temp3 = d1 + (double)temp2 * d2 - d3;
        
        /* Type conversions that require register moves */
        int int_from_float = (int)f4;
        float float_from_int = (float)(v6 + v7);
        double double_from_mix = (double)temp1 + (double)float_from_int;
        
        /* Complex addressing: array[index + large_constant] */
        /* This often requires address reloads */
        int idx = v8 + v9;
        int val1 = array[idx + 64];    /* Non-simple offset */
        int val2 = array[idx + 128];   /* Another non-simple offset */
        
        /* More complex expressions extending live ranges */
        i11 = i11 * i12 + val1;
        i12 = i13 - i14 * val2;
        i13 = i15 / (i16 + 1) + temp1;
        i14 = i17 ^ i18 & i19 | i20;
        
        /* Bitwise and arithmetic mix */
        int bitwise = (v10 << 2) | (v1 & 0xFF);
        bitwise = bitwise ^ (v2 * v3);
        bitwise = (bitwise >> 1) + (v4 % 7);
        
        /* Function call clobbers registers */
        int func_result = dummy_function(v1, v2, f1, d1, i11, i12);
        
        /* Inline assembly with clobbers - increases pressure */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "xmm0", "xmm1", 
              "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More computations after clobber */
        d2 = d2 * 1.01 + (double)func_result;
        d3 = d3 / 1.02 - (double)bitwise;
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads due to register class conflicts */
        float f_from_reg = (float)reg_var1 + f2;
        double d_from_reg = (double)reg_var2 * d4;
        
        /* Complex pointer arithmetic */
        int* ptr2 = ptr + (v3 * 2 + 16);  /* Non-simple computation */
        int val3 = *ptr2;
        ptr2 = ptr + (v4 * 3 - 8);        /* Another non-simple computation */
        int val4 = *ptr2;
        
        /* Mixed size accesses */
        char* char_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        char char_val = char_ptr[idx * 4 + 32];      /* char access */
        short short_val = short_ptr[idx * 2 + 64];   /* short access */
        int int_val = array[idx + 96];               /* int access */
        
        /* Combine different sized results */
        int mixed_sizes = char_val + short_val + int_val;
        
        /* Another function call with mixed types */
        double d_result = complex_op(v5, f3, d5, i13);
        
        /* Update accumulator to prevent elimination */
        accumulator += temp1 + (int)temp2 + (int)temp3 + 
                      int_from_float + (int)float_from_int +
                      val1 + val2 + i14 + bitwise + func_result +
                      (int)d_from_reg + val3 + val4 + mixed_sizes + (int)d_result;
        
        /* Modify some variables for next iteration */
        v1 = (v1 + 1) % 100;
        v2 = (v2 * 3) % 100;
        f1 = f1 * 1.1f;
        d1 = d1 * 0.99;
        reg_var1 = (reg_var1 + 7) % 100;
        reg_var2 = (reg_var2 * 2) % 100;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final accumulator: %d\n", accumulator);
    printf("Values: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
    
    return 0;
}
