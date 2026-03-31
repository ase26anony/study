/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types to stress caller-save */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Volatile variables to extend liveness */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 3;
    volatile float vf1 = seed * 1.5f;
    volatile float vf2 = seed * 2.5f;
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v4sf vec_float = {seed*1.1f, seed*1.2f, seed*1.3f, seed*1.4f};
    
    /* MMX type (8-byte vector) */
    volatile long long mmx_val = (long long)seed * 0x100010001LL;
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTERS ========== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int a = vi1 + i;
        int b = vi2 * i;
        int c = a ^ b;
        int d = (a << 3) | (b >> 2);
        
        /* Clobber integer registers - simulating a function call */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov %0, %%rax\n\t"
            "mov %1, %%rbx\n\t"
            "mov %2, %%rcx\n\t"
            "mov %3, %%rdx\n\t"
            :
            : "r"(a), "r"(b), "r"(c), "r"(d)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use values after clobber - forces save/restore */
        result += a - b + c * d;
        
        /* ========== SSE/AVX REGISTERS ========== */
        /* Use xmm0-xmm3 (call-clobbered) */
        float f1 = vf1 + i * 0.1f;
        float f2 = vf2 - i * 0.2f;
        v4sf v1 = vec_float;
        v4sf v2 = {f1, f2, f1*2, f2*2};
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            "movaps %0, %%xmm0\n\t"
            "movaps %1, %%xmm1\n\t"
            "addps %%xmm1, %%xmm0\n\t"
            :
            : "x"(v1), "x"(v2)
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use vector values after clobber */
        v4sf v3 = v1 + v2;
        result += (int)(v3[0] + v3[1] + v3[2] + v3[3]);
        
        /* ========== MMX REGISTERS ========== */
        /* Use mm0-mm1 (call-clobbered) */
        long long mm1 = mmx_val + i * 0x1000LL;
        long long mm2 = mm1 ^ 0xAAAAAAAAAAAAAAAALL;
        
        /* Clobber MMX registers */
        asm volatile (
            "# CLOBBER MMX REGS\n\t"
            "movq %0, %%mm0\n\t"
            "movq %1, %%mm1\n\t"
            "paddq %%mm1, %%mm0\n\t"
            : 
            : "y"(mm1), "y"(mm2)
            : "mm0", "mm1", "mm2", "mm3", "memory"
        );
        
        result += (int)(mm1 >> 32) + (int)mm2;
        
        /* ========== CREATE BASIC BLOCK ENDING WITH CLOBBER ========== */
        /* This creates a control flow edge right after a clobbering asm */
        if (result & 1) {
            /* Another clobber that could be at block end */
            asm volatile (
                "# POTENTIAL BLOCK-END CLOBBER\n\t"
                "mov %0, %%r8\n\t"
                "mov %1, %%r9\n\t"
                : 
                : "r"(result), "r"(i)
                : "r8", "r9", "r10", "r11", "memory"
            );
            /* Label/jump creates block boundary */
            goto update;
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# ALTERNATIVE CLOBBER\n\t"
                "mov %0, %%r12\n\t"
                "mov %1, %%r13\n\t"
                :
                : "r"(result), "r"(i)
                : "r12", "r13", "r14", "r15", "memory"
            );
        }
        
    update:
        /* Force register usage across the label */
        vi1 += result;
        vi2 -= i;
        
        /* ========== MIXED REGISTER PRESSURE ========== */
        /* Simultaneous pressure on multiple register classes */
        {
            int t1 = vi1 * vi2;
            float t2 = vf1 * vf2;
            v4si t3 = vec_int + (v4si){i, i, i, i};
            
            /* Complex asm with many clobbers */
            asm volatile (
                "# MEGA CLOBBER\n\t"
                "mov %0, %%rax\n\t"
                "mov %1, %%xmm0\n\t"
                "mov %2, %%mm0\n\t"
                "add $1, %%rax\n\t"
                "addps %%xmm0, %%xmm0\n\t"
                "paddd %%mm0, %%mm0\n\t"
                :
                : "r"(t1), "x"(t2), "y"(t3)
                : "rax", "rbx", "rcx", "rdx",
                  "xmm0", "xmm1", "xmm2", "xmm3",
                  "mm0", "mm1", "mm2", "mm3",
                  "memory"
            );
            
            result = result ^ t1 ^ (int)t2 ^ t3[0];
        }
    }
    
    return result;
}

/* External function to prevent optimization */
extern int external_func(int);

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Call multiple times with different seeds */
    for (int s = 0; s < 5; s++) {
        int res = test_caller_save(iterations, s * 100);
        total += res;
        
        /* Call external function to flush registers */
        total = external_func(total);
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
