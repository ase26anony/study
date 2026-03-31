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
extern void external_call(void);

int main(void) {
    /* Phase 1: Declare many live variables with volatile to prevent optimization */
    volatile int v0 = rand();
    volatile int v1 = rand();
    volatile int v2 = rand();
    volatile int v3 = rand();
    volatile int v4 = rand();
    volatile int v5 = rand();
    volatile int v6 = rand();
    volatile int v7 = rand();
    volatile int v8 = rand();
    volatile int v9 = rand();
    
    volatile float f0 = (float)rand() / RAND_MAX;
    volatile float f1 = (float)rand() / RAND_MAX;
    volatile float f2 = (float)rand() / RAND_MAX;
    volatile float f3 = (float)rand() / RAND_MAX;
    volatile float f4 = (float)rand() / RAND_MAX;
    
    volatile double d0 = (double)rand() / RAND_MAX;
    volatile double d1 = (double)rand() / RAND_MAX;
    volatile double d2 = (double)rand() / RAND_MAX;
    volatile double d3 = (double)rand() / RAND_MAX;
    
    /* Phase 2: Explicit register variables with potential conflicts */
    register int r0 asm("eax") = v0;
    register int r1 asm("ebx") = v1;
    register int r2 asm("ecx") = v2;
    register int r3 asm("edx") = v3;
    
    /* Phase 3: Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand();
    }
    
    /* Phase 4: Volatile loop counter to prevent optimization */
    volatile int loop_counter = 100;
    volatile int accumulator = 0;
    
    /* Main high-pressure loop */
    while (loop_counter-- > 0) {
        /* Complex interdependent expressions with mixed types */
        int t0 = v0 + v1 * v2 - v3 / (v4 + 1);
        float t1 = f0 * f1 + (float)v5 - f2 / (f3 + 1.0f);
        double t2 = d0 * d1 + (double)v6 - d2 / (d3 + 1.0);
        
        /* Type conversions forcing register moves */
        f0 = (float)t0 + t1;
        d0 = (double)t0 + t2;
        v0 = (int)f0 + (int)d0;
        
        /* Non-offsettable memory addresses */
        int idx = v1 + v2 + 1000; /* Large offset */
        int val1 = array[idx % 256]; /* Complex addressing */
        int val2 = array[(idx + 64) % 256]; /* Another complex address */
        
        /* Bitwise and arithmetic combinations */
        v1 = (v1 << 3) | (v2 & 0xFF);
        v2 = (v2 * 13) ^ (v3 | 0x5555);
        v3 = (v3 + v4) & ~(v5 - 1);
        
        /* Function call clobbering registers */
        int call_result = dummy_func(v0, v1, f0, d0, v2, v3);
        
        /* Inline assembly with many clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            :
            :
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory", "cc"
        );
        
        /* More complex expressions after clobber */
        f1 = f0 * 2.0f + (float)call_result;
        d1 = d0 * 2.0 + (double)call_result;
        
        /* Using explicit register variables in conflicting contexts */
        float f_temp = (float)r0 + (float)r1; /* Integer regs used for float */
        double d_temp = (double)r2 + (double)r3;
        
        /* More memory operations with different sizes */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        v4 = byte_ptr[v0 % 256] + short_ptr[v1 % 128];
        v5 = byte_ptr[v2 % 256] * 2 - short_ptr[v3 % 128];
        
        /* Mixed-size operations */
        v6 = (v4 << 8) | (byte_ptr[v5 % 256] & 0xFF);
        v7 = v5 * v6 + (short_ptr[v4 % 128] << 4);
        
        /* Update accumulator to prevent elimination */
        accumulator += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 
                     + (int)f0 + (int)f1 + (int)d0 + (int)d1;
        
        /* Another function call */
        dummy_func(v4, v5, f1, d1, v6, v7);
        
        /* More inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "memory", "cc"
        );
        
        /* Final complex expression mixing everything */
        v8 = ((v0 & v1) | (v2 ^ v3)) + ((v4 << 1) * (v5 >> 2)) 
             - (v6 / (v7 + 1)) + (int)(f0 * 100.0f) 
             + (int)(d0 * 50.0) + call_result;
        
        v9 = v8 * 3 - accumulator % 100;
        
        /* Force use of all variables */
        f2 = (float)v8 / (float)(v9 + 1);
        d2 = (double)v9 / (double)(v8 + 1);
        f3 = f2 * 2.0f - f1;
        d3 = d2 * 2.0 - d1;
        
        /* Prevent loop unrolling */
        if (rand() % 1000 == 0) {
            loop_counter += 1;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
