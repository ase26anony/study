/* reload_stress.c - Program to stress GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to force register saves/restores across calls */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int barrier = a + b + (int)c + (int)d + e + (int)f;
    return barrier & 0xFF;
}

/* Another noinline function to clobber registers */
__attribute__((noinline))
void clobber_regs(void) {
    /* Empty function that compiler can't optimize away */
    asm volatile("" : : : "memory");
}

int main(void) {
    /* SEED RANDOM FOR PORTABILITY */
    srand(42);
    
    /* VOLATILE VARIABLES TO PREVENT OPTIMIZATION */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    volatile int v4 = rand();
    volatile float v5 = (float)rand() / RAND_MAX;
    volatile double v6 = (double)rand() / RAND_MAX;
    
    /* MANY LIVE SCALAR VARIABLES - EXTENDED LIVE RANGES */
    int a1 = rand() % 100;
    int a2 = rand() % 100;
    int a3 = rand() % 100;
    int a4 = rand() % 100;
    int a5 = rand() % 100;
    int a6 = rand() % 100;
    int a7 = rand() % 100;
    int a8 = rand() % 100;
    int a9 = rand() % 100;
    int a10 = rand() % 100;
    
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    float f8 = (float)rand() / RAND_MAX;
    
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES WITH POTENTIAL CONFLICTS */
    register int r1 asm("r15") = a1 + 1;  /* Try to bind to specific reg */
    register int r2 asm("r14") = a2 + 2;
    
    /* ARRAY FOR COMPLEX ADDRESSING MODES */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* VOLATILE LOOP COUNTER TO PREVENT LOOP OPTIMIZATIONS */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MAIN STRESS LOOP */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - MIXED TYPES */
        f1 = (float)a1 + f2 * 3.14f - (float)(a3 & 0xFF);
        d1 = (double)f3 + d2 * 2.71828 - (double)(a4 | 0x0F);
        
        /* TYPE CONVERSIONS FORCING REGISTER MOVES */
        a5 = (int)(f4 * 100.0f) + (int)d3;
        f5 = (float)(a6 * 2) / 7.0f;
        d4 = (double)((a7 << 3) & 0xFF) + d5;
        
        /* BITWISE AND ARITHMETIC COMBINATIONS */
        a8 = ((a1 + a2) * (a3 - a4)) & ((a5 | a6) << 2);
        a9 = (a7 * a8) + ((a10 & 0xF0) >> 4);
        
        /* COMPLEX ADDRESSING WITH NON-OFFSETTABLE ADDRESSES */
        /* array[index + constant] where constant may be too large */
        int idx1 = a1 + (loop & 0x0F);
        int idx2 = a2 + 128;  /* Large offset */
        int idx3 = a3 + 64;
        
        /* These may require address reloads */
        int val1 = array[idx1] + array[idx2];
        int val2 = array[idx3 * 2] - array[idx1 + 32];
        
        /* USE EXPLICIT REGISTER VARIABLES IN DIFFERENT CONTEXTS */
        /* Force conflicts by using integer register in float context */
        f6 = f6 + (float)r1 * 0.5f;
        d5 = d5 + (double)r2 * 0.25;
        
        /* FUNCTION CALL TO CLOBBER REGISTERS */
        int ret = dummy_function(a1, a2, f1, d1, a3, f2);
        
        /* INLINE ASSEMBLY WITH MANY CLOBBERS */
        /* Clobber multiple registers to increase pressure */
        asm volatile(
            "/* Clobber many registers */"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* MORE COMPLEX EXPRESSIONS AFTER REGISTER CLOBBERING */
        /* Compiler must reload values after asm clobber */
        f7 = f1 * 2.0f + f3 / 3.0f - (float)(val1 & 0xFF);
        d2 = d1 * 1.5 + d3 / 4.0 - (double)(val2 | 0x0F);
        
        a10 = (a8 * a9) + ((a5 + a6) & 0xFF) - ret;
        
        /* MIXED SIZE MEMORY ACCESSES */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        /* Different sized accesses in same expression */
        int byte_sum = byte_ptr[idx1] + byte_ptr[idx2];
        int short_sum = short_ptr[idx3] + short_ptr[idx1];
        a4 = (byte_sum << 8) | (short_sum & 0xFF);
        
        /* ANOTHER FUNCTION CALL */
        clobber_regs();
        
        /* UPDATE VOLATILE ACCUMULATOR - PREVENT DEAD CODE ELIMINATION */
        accumulator += a1 + a2 + a3 + a4 + a5 + (int)f1 + (int)d1 + ret;
        accumulator &= 0xFFFF;  /* Prevent overflow */
        
        /* SHUFFLE VALUES TO EXTEND LIVE RANGES */
        int tmp = a1;
        a1 = a2;
        a2 = a3;
        a3 = a4;
        a4 = a5;
        a5 = a6;
        a6 = a7;
        a7 = a8;
        a8 = a9;
        a9 = a10;
        a10 = tmp + loop;
        
        float ftmp = f1;
        f1 = f2;
        f2 = f3;
        f3 = f4;
        f4 = f5;
        f5 = f6;
        f6 = f7;
        f7 = f8;
        f8 = ftmp * 0.9f;
        
        double dtmp = d1;
        d1 = d2;
        d2 = d3;
        d3 = d4;
        d4 = d5;
        d5 = dtmp * 0.8;
    }
    
    /* USE ALL VARIABLES ONE MORE TIME TO EXTEND LIVE RANGES */
    int final_result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 +
        (int)f5 + (int)f6 + (int)f7 + (int)f8 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
        v1 + (int)v2 + (int)v3 + v4 + (int)v5 + (int)v6;
    
    printf("Result: %d (Accumulator: %d)\n", final_result, accumulator);
    return final_result & 0xFF;
}
