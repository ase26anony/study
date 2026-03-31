/* reload_stressor.c - Designed to trigger GCC's reload allocation logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
    return result;
}

/* Another noinline function to clobber registers */
__attribute__((noinline, optimize("O0")))
void clobber_regs(void) {
    /* Empty but prevents optimization */
    asm volatile("" : : : "memory");
}

int main(void) {
    /* VOLATILE VARIABLES - Prevent optimization and force register usage */
    volatile int seed = 42;
    volatile int loop_limit = 100;
    volatile double final_acc = 0.0;
    
    /* MANY LIVE SCALAR VARIABLES (30+ variables) */
    /* Integer variables */
    register int v1 asm("eax") = rand() ^ seed;
    register int v2 asm("ebx") = rand() ^ (seed + 1);
    register int v3 asm("ecx") = rand() ^ (seed + 2);
    int v4 = rand() ^ (seed + 3);
    int v5 = rand() ^ (seed + 4);
    int v6 = rand() ^ (seed + 5);
    int v7 = rand() ^ (seed + 6);
    int v8 = rand() ^ (seed + 7);
    int v9 = rand() ^ (seed + 8);
    int v10 = rand() ^ (seed + 9);
    int v11 = rand() ^ (seed + 10);
    int v12 = rand() ^ (seed + 11);
    int v13 = rand() ^ (seed + 12);
    int v14 = rand() ^ (seed + 13);
    int v15 = rand() ^ (seed + 14);
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    float f6 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    double d6 = (double)rand() / RAND_MAX;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand();
    }
    
    volatile int* volatile_ptr = &array[0];
    int* ptr1 = &array[64];
    int* ptr2 = &array[128];
    
    /* LOOP WITH INVARIANT SPILLING */
    for (volatile int iteration = 0; iteration < loop_limit; iteration++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - Extend live ranges */
        /* Integer expressions with bitwise and arithmetic ops */
        v1 = (v1 * v2) ^ (v3 << 2) | (v4 & 0xFF);
        v2 = (v2 + v5) * (v6 - v7) ^ (v8 >> 3);
        v3 = (v9 | v10) + (v11 & v12) * (v13 ^ v14);
        v4 = v15 + ((v1 & v2) | (v3 ^ v4)) * v5;
        v5 = (v6 * v7) + (v8 / (v9 + 1)) ^ v10;
        
        /* Mixed integer/float conversions - Force register file moves */
        f1 = (float)v1 + f2 * (float)v2;
        f2 = (float)(v3 ^ v4) / f3 + (float)(v5 & v6);
        f3 = f1 * f2 - (float)v7 + (float)(v8 | v9);
        
        /* Double precision with integer conversions */
        d1 = (double)v10 + d2 * (double)(v11 ^ v12);
        d2 = (double)(v13 & v14) / d3 + (double)v15;
        d3 = d1 * d2 - (double)(v1 | v2) + (double)v3;
        
        /* FUNCTION CALL - Clobbers caller-saved registers */
        int call_result = dummy_function(v1, v2, f1, d1, v3);
        v6 = call_result ^ v4;
        
        /* INLINE ASSEMBLY WITH MANY CLOBBERS - Increase register pressure */
        asm volatile(
            "/* Clobber many registers */"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* COMPLEX ADDRESSING MODES - Non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = v1 & 0x3F;  /* 0-63 */
        int idx2 = v2 & 0x3F;  /* 0-63 */
        
        /* Large offset that may not be directly addressable */
        int val1 = array[idx1 + 128];  /* offset 128 may be too large */
        int val2 = array[idx2 + 192];  /* offset 192 may be too large */
        
        /* Pointer arithmetic with complex expressions */
        int* complex_ptr = &array[(v3 & 0x7F) + ((v4 & 0x3F) << 1)];
        int val3 = *complex_ptr;
        
        /* More mixed-type operations */
        f4 = (float)val1 * f5 + (float)val2 / f6;
        d4 = (double)val3 * d5 - d6 / (double)(val1 + 1);
        
        /* Type conversions with different sizes */
        short s1 = (short)(v1 & 0xFFFF);
        char c1 = (char)(v2 & 0xFF);
        v7 = (int)s1 * (int)c1 + v8;
        
        /* Update volatile accumulator - Prevent dead code elimination */
        final_acc += (double)v1 + (double)v2 + (double)v3 +
                    (double)f1 + (double)f2 + (double)f3 +
                    d1 + d2 + d3 + (double)val1 + (double)val2;
        
        /* Another function call */
        clobber_regs();
        
        /* More complex expressions to extend live ranges further */
        v8 = (v9 * v10) ^ (v11 << (v12 & 0x3)) | (v13 & v14);
        v9 = (v15 + v1) * (v2 - v3) ^ (v4 >> (v5 & 0x7));
        v10 = (v6 | v7) + (v8 & v9) * (v10 ^ v11);
        
        /* Float/double mixing */
        f5 = f1 * (float)d1 - f2 / (float)d2 + f3;
        d5 = d3 * (double)f3 - d4 / (double)f4 + d6;
        
        /* Another inline asm with clobbers */
        asm volatile(
            "/* More register clobbering */"
            :
            :
            : "rax", "rbx", "rcx", "rdx",
              "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7", "memory"
        );
    }
    
    /* Use all variables one more time to extend live ranges to end */
    volatile int final_check = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
    
    printf("Final accumulator: %f\n", final_acc);
    printf("Final check: %d\n", final_check);
    
    return 0;
}
