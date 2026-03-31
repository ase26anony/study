#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Phase 1: Declare many variables with extended live ranges */
    /* Integer variables - many will need to stay live */
    register int r0 asm("r0") = rand();
    register int r1 asm("r1") = rand();
    volatile int v0 = rand();
    volatile int v1 = rand();
    volatile int v2 = rand();
    volatile int v3 = rand();
    volatile int v4 = rand();
    volatile int v5 = rand();
    volatile int v6 = rand();
    volatile int v7 = rand();
    int i0 = rand(), i1 = rand(), i2 = rand(), i3 = rand();
    int i4 = rand(), i5 = rand(), i6 = rand(), i7 = rand();
    int i8 = rand(), i9 = rand(), i10 = rand(), i11 = rand();
    
    /* Floating point variables - different register class */
    volatile float f0 = rand() / 1000.0f;
    volatile float f1 = rand() / 1000.0f;
    volatile float f2 = rand() / 1000.0f;
    volatile float f3 = rand() / 1000.0f;
    float f4 = rand() / 1000.0f, f5 = rand() / 1000.0f;
    float f6 = rand() / 1000.0f, f7 = rand() / 1000.0f;
    
    /* Double precision variables */
    volatile double d0 = rand() / 10000.0;
    volatile double d1 = rand() / 10000.0;
    double d2 = rand() / 10000.0, d3 = rand() / 10000.0;
    double d4 = rand() / 10000.0, d5 = rand() / 10000.0;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int idx = 0; idx < 256; idx++) {
        array[idx] = rand();
    }
    volatile int* p0 = &array[0];
    volatile int* p1 = &array[128];
    
    /* Mixed size variables */
    volatile short s0 = rand() & 0xFFFF;
    volatile short s1 = rand() & 0xFFFF;
    volatile char c0 = rand() & 0xFF;
    volatile char c1 = rand() & 0xFF;
    
    /* Phase 2: Loop with invariant spilling */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    for (volatile int iteration = 0; iteration < loop_limit; iteration++) {
        /* Complex expression 1: Mix integer and float with conversions */
        f0 = (float)v0 + (float)v1 * f1 - (float)(i0 * i1) / f2;
        d0 = (double)f0 + (double)v2 * d1 - (double)(i2 | i3) / d2;
        
        /* Use explicit register variables in conflicting contexts */
        int temp1 = r0 + r1;  /* Uses hard registers */
        f3 = (float)temp1 * f4;  /* Conversion needed */
        
        /* Complex addressing mode: non-offsettable address */
        /* This often requires reloading the address calculation */
        int idx = v3 + v4 + (iteration & 0xF);
        int val1 = array[idx + 64];  /* Large offset may need reload */
        int val2 = array[idx + 128]; /* Another large offset */
        
        /* Mixed size accesses in same expression */
        v5 = (v5 & 0xFF00) | (c0 & 0xFF);
        v6 = (v6 << 8) | (s0 & 0xFF);
        
        /* Function call clobbers registers */
        int call_result = dummy_function(v0, v1, f0, d0, i4, f5);
        
        /* Inline assembly with many clobbers */
        /* Forces compiler to save/restore around asm */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11", "r12",
              "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7"
        );
        
        /* More complex expressions after clobber */
        i4 = ((i5 * i6) >> 4) & ((i7 | i8) << 2);
        f5 = (float)i4 / (f6 + 1.0f);
        
        /* Type conversions between different sizes */
        d3 = (double)((short)v5) + (double)((char)v6);
        f7 = (float)((int)d3) * f2;
        
        /* Pointer arithmetic that may need reload */
        int* complex_ptr = &array[(v7 + i9 + (iteration * 3)) & 0xFF];
        v7 = *complex_ptr + *(complex_ptr + 32) + *(complex_ptr + 64);
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v0 + v1 + (int)f0 + (int)d0 + i0 + call_result;
        
        /* More register pressure */
        i10 = i11 ^ (v2 * v3);
        d4 = d5 * (double)i10 + (double)f3;
        
        /* Another function call */
        dummy_function(v2, v3, f1, d1, i5, f6);
        
        /* Complex bitwise/arithmetic combination */
        v4 = ((v5 & v6) | (v7 << 4)) + ((i0 * i1) >> 2);
        
        /* Mixed mode operation */
        f2 = f3 + (float)((v4 * i2) / (i3 + 1));
        
        /* Update variables to keep them live */
        v0 = v0 ^ 0x1234;
        v1 = v1 + 1;
        i0 = i0 - i1;
        i1 = i1 ^ i2;
    }
    
    /* Phase 3: Final complex expression using all variables */
    double final_result = 
        (double)accumulator + 
        (double)v0 + (double)v1 + (double)v2 +
        (double)i0 + (double)i1 + (double)i2 +
        (double)f0 + (double)f1 + (double)f2 +
        d0 + d1 + d2;
    
    /* Use all pointer variables */
    final_result += (double)(*p0 + *p1);
    final_result += (double)(s0 + s1 + c0 + c1);
    
    printf("Result: %f\n", final_result);
    
    /* Use all remaining variables to prevent optimization */
    asm volatile ("" : : "r"(r0), "r"(r1), "r"(v0), "r"(v1), 
                   "r"(i0), "r"(i1), "r"(i2), "r"(i3),
                   "r"(f0), "r"(f1), "r"(f2), "r"(d0), "r"(d1));
    
    return (int)final_result & 0xFF;
}
