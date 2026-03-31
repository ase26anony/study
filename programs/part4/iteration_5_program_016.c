#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_limit = 100;
    volatile int global_acc = 0;
    
    /* MANY LIVE SCALAR VARIABLES (30+ variables) */
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
    volatile double d1 = (double)(rand() % 100) / 10.0;
    volatile double d2 = (double)(rand() % 100) / 10.0;
    volatile double d3 = (double)(rand() % 100) / 10.0;
    volatile double d4 = (double)(rand() % 100) / 10.0;
    volatile double d5 = (double)(rand() % 100) / 10.0;
    
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
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("eax") = v1 + v2;
    register int reg_var2 asm("ebx") = v3 + v4;
    /* Note: Compiler may ignore or reassign these on different architectures,
       but they still create constraints */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array;
    
    /* LOOP with invariant spilling */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS with mixed types */
        /* 1. Integer expressions with bitwise and arithmetic ops */
        int expr1 = (v1 & v2) | (v3 << 2) + (v4 * v5) - (v6 ^ v7);
        int expr2 = ((v8 | v9) & (v10 << 3)) * (v11 + v12) / (v13 | 1);
        
        /* 2. Float/double conversions and operations */
        float f_expr = f1 + (float)expr1 - f2 * (float)v14;
        double d_expr = d1 * (double)f_expr + d2 - (double)(v15 & 0xFF);
        
        /* 3. Type conversions between int/float/double */
        int int_from_float = (int)f3 + (int)d3;
        float float_from_int = (float)(v16 * v17) / 100.0f;
        double double_from_mixed = (double)(v18 & v19) + (double)f4 - d4;
        
        /* 4. Complex pointer arithmetic with non-simple offsets */
        /* Non-offsettable address: array[index + large_constant] */
        int index = (v20 + iter) & 0xFF;
        int large_offset = 128; /* May be too large for direct addressing on some arches */
        int array_val = array[(index + large_offset) & 0xFF];
        
        /* 5. More mixed operations using explicit register variables */
        int reg_expr = reg_var1 * array_val - reg_var2 / (expr2 | 1);
        
        /* 6. FUNCTION CALL to clobber registers */
        int call_result = dummy_func(expr1, expr2, f_expr, d_expr, reg_expr);
        
        /* 7. INLINE ASSEMBLY with multiple clobbers */
        /* This tells GCC many registers are unavailable */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* 8. More calculations after clobbering */
        /* Different sized memory accesses */
        char char_val = (char)(array_val & 0xFF);
        short short_val = (short)(array_val & 0xFFFF);
        int int_val = array_val;
        
        /* Mixed size expression */
        int mixed_size_expr = (int)char_val * short_val + int_val / 256;
        
        /* 9. Another function call */
        call_result += dummy_func(mixed_size_expr, int_from_float, 
                                 float_from_int, double_from_mixed, v10);
        
        /* 10. Complex expression using all variable types */
        double final_expr = (double)call_result * d5 
                          + (double)f5 * (double)v1 
                          - (double)(v2 & v3) 
                          + (double)array[(index + 64) & 0xFF];
        
        /* Update volatile accumulator to prevent elimination */
        global_acc += (int)final_expr;
        
        /* Modify some variables to create data dependencies across iterations */
        v1 = (v1 + 1) & 0xFF;
        v2 = (v2 + global_acc) & 0xFF;
        f1 = f1 * 1.01f;
        d1 = d1 * 0.99;
        
        /* Use explicit register variables in conflicting context */
        reg_var1 = reg_var1 + array[(reg_var2 + iter) & 0xFF];
        
        /* External call that might clobber more registers */
        external_call();
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final accumulator: %d\n", global_acc);
    printf("Values: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
    
    return 0;
}

/* Empty function definition for external_call */
void external_call(void) {
    /* Do nothing, but compiler doesn't know that */
    volatile int dummy = 0;
    (void)dummy;
}
