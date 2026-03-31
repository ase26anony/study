#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* External function to clobber registers */
extern int rand(void);

int main(void) {
    /* Phase 1: Declare MANY variables to exhaust registers */
    /* Integer variables - many will need to stay live */
    volatile int v0 = rand() % 100;
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    
    /* Non-volatile but heavily used integers */
    int i0 = rand() % 100;
    int i1 = rand() % 100;
    int i2 = rand() % 100;
    int i3 = rand() % 100;
    int i4 = rand() % 100;
    int i5 = rand() % 100;
    int i6 = rand() % 100;
    int i7 = rand() % 100;
    int i8 = rand() % 100;
    int i9 = rand() % 100;
    
    /* Floating point variables - different register class */
    volatile float f0 = (float)rand() / RAND_MAX;
    volatile float f1 = (float)rand() / RAND_MAX;
    volatile float f2 = (float)rand() / RAND_MAX;
    volatile float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    
    /* Double precision - another register class */
    volatile double d0 = (double)rand() / RAND_MAX;
    volatile double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    
    /* Phase 2: Explicit register variables with conflicts */
    /* Force specific registers, then use in conflicting contexts */
    register int r0 asm("r0") = v0;
    register int r1 asm("r1") = v1;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int j = 0; j < 256; j++) {
        array[j] = rand() % 1000;
    }
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* Phase 3: Main high-pressure loop */
    for (int iter = 0; iter < loop_limit; iter++) {
        /* Complex expression 1: Mix integer and float operations */
        float temp_f = f0 + (float)i0 + (float)(v0 * v1) / 100.0f;
        temp_f = temp_f * f1 - (float)(i1 >> 2);
        
        /* Type conversions forcing register moves */
        double temp_d = (double)temp_f + d0;
        temp_d = temp_d * (double)((i2 & 0xFF) + 1);
        
        /* Complex integer expression with many intermediates */
        int temp_i = i3 * i4 - i5;
        temp_i = (temp_i << 3) | (i6 & 0xF);
        temp_i = temp_i + (i7 ^ i8) * i9;
        
        /* Non-offsettable memory addressing */
        /* Large offset that may not fit in addressing mode */
        int index = (v2 + iter) & 0xFF;
        int offset = 128; /* Large constant offset */
        int array_val = array[index + offset]; /* May need address reload */
        
        /* More complex addressing with multiple components */
        int addr = (v3 * 4) + (v4 << 1) + 64;
        if (addr >= 0 && addr < 256) {
            array_val += array[addr]; /* Another complex address */
        }
        
        /* Function call clobbering registers */
        int func_result = dummy_func(v5, v6, f2, d1, v7, v8);
        
        /* Inline assembly with many clobbers */
        /* Force compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "f0", "f1", "f2", "f3", "f4", "f5"
        );
        
        /* Use explicit register variables in conflicting ways */
        /* These are bound to specific registers but used in FP context */
        double conflict_d = (double)r0 + (double)r1;
        
        /* More type mixing */
        i0 = (int)(temp_f * 100.0f) ^ array_val;
        i1 = (int)temp_d + func_result;
        
        /* Bitwise and arithmetic combination */
        i2 = ((i2 * 3) & 0xFFFF) | ((i3 << 8) & 0xFF00);
        i3 = (i4 ^ i5) + (i6 * i7) - (i8 / (i9 + 1));
        
        /* Float/int conversions */
        f4 = (float)((i0 & 0xFF) + (i1 & 0xFF));
        f5 = f4 * 0.5f + (float)(v9 % 64);
        
        /* Double operations with int conversion */
        d2 = d3 * 2.0 + (double)((i2 + i3) % 100);
        d3 = conflict_d * 0.25;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += temp_i + (int)temp_f + (int)temp_d + array_val + func_result;
        
        /* Shuffle values to extend live ranges */
        int rot = i0;
        i0 = i1; i1 = i2; i2 = i3; i3 = i4; i4 = i5;
        i5 = i6; i6 = i7; i7 = i8; i8 = i9; i9 = rot;
        
        float frot = f4;
        f4 = f5; f5 = f0; f0 = f1; f1 = f2; f2 = f3; f3 = frot;
        
        double drot = d2;
        d2 = d3; d3 = d0; d0 = d1; d1 = drot;
    }
    
    /* Phase 4: Final complex expression using all variables */
    int final_result = 
        v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
        i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 +
        (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d0 + (int)d1 + (int)d2 + (int)d3 +
        accumulator;
    
    printf("Result: %d\n", final_result);
    
    /* Use all variables one more time to keep them live */
    asm volatile (
        "# Final use of variables\n"
        : 
        : "r"(r0), "r"(r1), "m"(v0), "m"(v1), "m"(v2), "m"(v3), "m"(v4),
          "m"(v5), "m"(v6), "m"(v7), "m"(v8), "m"(v9),
          "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "memory"
    );
    
    return final_result % 100;
}
