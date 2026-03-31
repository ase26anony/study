#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_calc(double a, double b, double c, int d, float e) {
    volatile double temp = a * b + c / d + e;
    return temp;
}

int main(void) {
    /* Phase 1: Declare MANY variables to create register pressure */
    /* Integer variables */
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
    
    /* Floating point variables */
    volatile float f1 = (float)(rand() % 100) / 10.0f;
    volatile float f2 = (float)(rand() % 100) / 10.0f;
    volatile float f3 = (float)(rand() % 100) / 10.0f;
    volatile float f4 = (float)(rand() % 100) / 10.0f;
    volatile float f5 = (float)(rand() % 100) / 10.0f;
    
    /* Double precision variables */
    volatile double d1 = (double)(rand() % 100) / 5.0;
    volatile double d2 = (double)(rand() % 100) / 5.0;
    volatile double d3 = (double)(rand() % 100) / 5.0;
    volatile double d4 = (double)(rand() % 100) / 5.0;
    volatile double d5 = (double)(rand() % 100) / 5.0;
    
    /* More integer variables */
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
    
    /* More floating point */
    float f6 = (float)(rand() % 100) / 10.0f;
    float f7 = (float)(rand() % 100) / 10.0f;
    float f8 = (float)(rand() % 100) / 10.0f;
    float f9 = (float)(rand() % 100) / 10.0f;
    float f10 = (float)(rand() % 100) / 10.0f;
    
    /* Array for complex addressing modes */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Explicit register variables - these will conflict with natural allocation */
    register int reg_var1 asm("ax");
    register int reg_var2 asm("bx");
    reg_var1 = v1 + v2;
    reg_var2 = v3 * v4;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_counter = 10;
    volatile double accumulator = 0.0;
    
    /* Phase 2: Complex loop with high register pressure */
    for (int iter = 0; iter < loop_counter; iter++) {
        /* Complex expression mixing all types and variables */
        double temp1 = d1 * d2 + (double)f1 * (double)f2;
        float temp2 = f3 * f4 + (float)v5 * (float)v6;
        int temp3 = v7 * v8 + (int)f5 * (int)d3;
        
        /* Type conversions that require register moves */
        double conv1 = (double)v9 + (double)v10;
        float conv2 = (float)d4 + (float)d5;
        int conv3 = (int)f6 + (int)f7;
        
        /* Complex addressing with non-offsettable addresses */
        /* array[index + constant] where constant might be too large */
        int idx1 = v11 + v12;
        int val1 = array[idx1 + 15];  /* May require address reload */
        int idx2 = v13 + v14;
        int val2 = array[idx2 + 25];  /* Another complex address */
        
        /* Mixed operations in single expression */
        double complex_expr = (d1 * 3.14159) + (double)(v15 & 0xFF) * 
                             (double)((v16 << 2) | (v17 >> 3)) +
                             (double)((v18 ^ v19) + v20);
        
        /* Function call that clobbers registers */
        int func_result = dummy_function(v1, v2, f1, d1, v3, f2);
        
        /* Inline assembly with many clobbered registers */
        /* This forces the compiler to save/restore around it */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More complex calculations after clobber */
        float float_ops = f8 * f9 - f10 / f6 + (float)func_result;
        double double_ops = d2 * d3 / d4 + (double)float_ops;
        
        /* Use explicit register variables in conflicting contexts */
        /* These are bound to integer regs but used in FP contexts */
        double mixed_use = (double)reg_var1 * 1.5 + (double)reg_var2 * 2.5;
        
        /* Another function call with different signature */
        double calc_result = complex_calc(d1, d2, d3, v4, f3);
        
        /* Bitwise and arithmetic combinations */
        int bitwise_result = (v5 & v6) | (v7 ^ v8) + (v9 << 2) * (v10 >> 1);
        
        /* Different sized memory accesses */
        char char_val = (char)(array[v11] & 0xFF);
        short short_val = (short)(array[v12] & 0xFFFF);
        int int_val = array[v13];
        double double_val = (double)array[v14];
        
        /* Complex expression with all the above */
        accumulator += temp1 + temp2 + temp3 + conv1 + conv2 + conv3 +
                      val1 + val2 + complex_expr + double_ops + 
                      mixed_use + calc_result + bitwise_result +
                      char_val + short_val + int_val + double_val;
        
        /* Update some variables to create data dependencies */
        v1 = v2 + 1;
        v2 = v3 - 1;
        v3 = v4 * 2;
        v4 = v5 / 2;
        v5 = v6 ^ 0x55;
        
        f1 = f2 * 1.1f;
        f2 = f3 + 0.5f;
        f3 = f4 - 0.3f;
        
        d1 = d2 * 1.01;
        d2 = d3 + 0.25;
        d3 = d4 / 1.5;
        
        /* Use explicit register variables again */
        reg_var1 = reg_var1 + v1;
        reg_var2 = reg_var2 * v2;
        
        /* Another inline asm with clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9"
        );
    }
    
    /* Phase 3: Final output to prevent dead code elimination */
    printf("Final accumulator: %f\n", accumulator);
    printf("Values: v1=%d, f1=%f, d1=%f\n", v1, f1, d1);
    
    return 0;
}
