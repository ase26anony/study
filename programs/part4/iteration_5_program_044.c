#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_calc(double x, double y, float z, int w) {
    volatile double res = (x * y) / (z + w);
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_counter;
    volatile double final_acc = 0.0;
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
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    float f5 = (float)(rand() % 100) / 10.0f;
    
    /* Double precision variables */
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    double d4 = (double)(rand() % 100) / 5.0;
    double d5 = (double)(rand() % 100) / 5.0;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int reg_var1 asm("r12") = v1 + v2;  /* May conflict with compiler's register allocator */
    register int reg_var2 asm("r13") = v3 + v4;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer that will be used with non-offsettable addresses */
    int* ptr = array;
    
    /* LOOP with invariant spilling */
    for (loop_counter = 0; loop_counter < loop_limit; loop_counter++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v3 ^ v4 | v5 & v6;
        v3 = (v7 << 2) | (v8 >> 3);
        v4 = v9 * v10 - v11 + v12;
        v5 = v13 & v14 | v15 ^ v16;
        
        /* MIXED DATA TYPE OPERATIONS - force register class changes */
        f1 = (float)v1 + f2 * 3.14f;
        f2 = f3 / (float)v2 - f4;
        
        /* Integer to float conversions */
        d1 = (double)v3 + d2 * 2.71828;
        d2 = d3 / (double)v4 - d4;
        
        /* Float to integer conversions */
        v6 = (int)f1 + (int)d1 * v7;
        v7 = v8 + (int)(f2 * d2);
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* array[index + constant] where index is complex expression */
        int index = (v1 + v2 * 3 - v4) & 0xFF;
        int offset = 37;  /* Non-simple offset */
        int val1 = array[index + offset];  /* May require address reload */
        int val2 = array[(index * 2) + 17]; /* More complex addressing */
        
        /* Use explicit register variables in conflicting contexts */
        int temp = reg_var1 * val1;
        float ftemp = (float)reg_var2 + f1;  /* Integer reg used in float op */
        
        /* FUNCTION CALL - clobbers registers */
        int call_result = dummy_func(v1, v2, f1, d1, v3, v4);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS - increase register pressure */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11",
              "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7"
        );
        
        /* MORE COMPLEX CALCULATIONS after asm clobber */
        v8 = v9 * v10 + call_result;
        v9 = (v11 << 1) | (v12 >> 2);
        
        /* Different sized memory accesses */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed size accesses in same expression */
        v10 = byte_ptr[index] + short_ptr[index / 2] + array[index];
        
        /* Bitwise and arithmetic combinations */
        v11 = (v12 & 0xFF) + (v13 | 0x7F) * (v14 ^ 0x55);
        v12 = ((v15 << 3) + (v16 >> 2)) * ((v17 & 0xF) | 0x10);
        
        /* Another function call with mixed types */
        double dresult = complex_calc(d1, d2, f3, v18);
        
        /* More type conversions */
        f3 = (float)dresult + f4;
        v13 = (int)f3 * v19;
        
        /* Update volatile accumulator - prevent dead code elimination */
        final_acc += (double)v1 + (double)v2 + (double)v3 + 
                    (double)v4 + (double)v5 + d1 + d2 + 
                    (double)f1 + (double)f2 + (double)call_result + dresult;
        
        /* Use pointer arithmetic that may need reloading */
        ptr = array + ((index * 3 + 19) & 0xFF);
        v14 = *ptr + v20;
        
        /* Another asm with different clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Final mixed computation */
        v15 = v16 + v17 - v18 * v19 / (v20 + 1);
        f4 = f5 * 2.0f + (float)v15;
        d3 = d4 / 3.0 + (double)v16;
        
        /* Use all variables one more time to keep them live */
        v16 = v17 + v18;
        v17 = v19 ^ v20;
        v18 = v1 & v2;
        v19 = v3 | v4;
        v20 = v5 + v6;
        
        f5 = f1 + f2 - f3 + f4;
        d4 = d1 * d2 - d3 + d5;
        d5 = d4 / 2.0;
    }
    
    /* Print result to prevent optimization */
    printf("Final accumulator: %f\n", (double)final_acc);
    
    /* Use all variables one final time */
    int total = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    printf("Total: %d\n", total);
    
    return 0;
}
