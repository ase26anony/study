#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed random for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_limit = 100;
    volatile int global_acc = 0;
    
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
    
    /* More integers with volatile to force reloads */
    volatile int v11 = rand() % 100;
    volatile int v12 = rand() % 100;
    
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
    
    /* Additional mixed variables */
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    double d6 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("eax") = v1 + v2;  /* May conflict with x86 eax */
    register int reg_var2 asm("ebx") = v3 + v4;  /* May conflict with x86 ebx */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* LOOP with invariant spilling */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS mixing all variables */
        
        /* Integer expressions with bitwise and arithmetic ops */
        int expr1 = (v1 & v2) | (v3 << 2) + (v4 * v5) - (v6 ^ v7);
        int expr2 = ((v8 + v9) * (v10 - v11)) / (v12 + 1);
        
        /* Float/double conversions and operations */
        float expr3 = f1 * (float)expr1 + f2 / (float)expr2;
        double expr4 = d1 * (double)expr3 + d2 - (double)v13;
        
        /* Mixed type conversions */
        int expr5 = (int)(expr3 * 100.0f) + (int)(expr4 * 50.0);
        float expr6 = (float)expr5 / 10.0f + f3 * f4;
        
        /* Complex pointer arithmetic with non-offsettable address */
        /* array[index + constant] where constant is large */
        int index = v14 % 64;
        int offset = 100;  /* Large offset that may not be directly addressable */
        int mem_val = array[index + offset];  /* May require address reload */
        
        /* More mixed operations */
        double expr7 = d3 * (double)mem_val + d4 * (double)expr5;
        float expr8 = f5 * (float)expr7 + f6;
        
        /* Use explicit register variables in conflicting contexts */
        int conflict1 = reg_var1 * (int)f7;  /* Integer reg var used with float */
        float conflict2 = (float)reg_var2 * f6;  /* Integer reg var in float op */
        
        /* FUNCTION CALL to clobber registers */
        int call_result = dummy_func(v1, v2, f1, d1, v3, f2);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* Clobber many registers to increase pressure */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* More calculations after assembly clobber */
        v1 = expr1 + call_result;
        v2 = expr2 - mem_val;
        f1 = expr3 * 0.5f;
        d1 = expr4 / 2.0;
        
        /* Different sized memory accesses */
        char char_val = (char)(array[index] & 0xFF);
        short short_val = (short)(array[index + 1] & 0xFFFF);
        int int_val = array[index + 2];
        
        /* Combine different sized accesses */
        int mixed_access = (int)char_val * (int)short_val + int_val;
        
        /* More complex expressions with all remaining variables */
        v3 = v4 + v5 * v6 - v7 / (v8 + 1);
        v4 = (v9 ^ v10) | (v11 << 3);
        v5 = v12 + v13 - v14 * v15;
        
        f2 = f3 + f4 * f5 - f6 / (f7 + 0.1f);
        f3 = f4 * 2.0f + f5 / 3.0f;
        
        d2 = d3 + d4 * d5 - d6 / (d1 + 0.1);
        d3 = d4 * 2.0 + d5 / 3.0;
        
        /* Update volatile accumulator to prevent elimination */
        global_acc += expr1 + expr5 + (int)expr6 + mem_val + mixed_access;
        
        /* Additional pointer arithmetic */
        ptr = array + (index * 3 + 50);  /* Complex address calculation */
        int ptr_val = *ptr;
        
        /* Use pointer value in next iteration */
        v6 = ptr_val + v7;
        
        /* Another function call */
        dummy_func(v8, v9, f3, d2, v10, f4);
        
        /* More inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12"
        );
        
        /* Final mixed calculations */
        double final_expr = (double)v1 * d1 + (double)v2 * d2 + 
                           (double)v3 * d3 + (double)v4 * d4;
        float final_float = (float)final_expr + f1 + f2 + f3;
        
        global_acc += (int)final_float;
    }
    
    printf("Final accumulator: %d\n", global_acc);
    return 0;
}
