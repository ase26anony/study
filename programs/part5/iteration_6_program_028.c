/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Variables that must survive across asm clobbers */
    int i1 = vi + 1;
    int i2 = vi * 2;
    float f1 = vf + 1.0f;
    float f2 = vf * 2.0f;
    v4si vec_i = {i1, i2, i1 + 1, i2 + 1};
    v4sf vec_f = {f1, f2, f1 + 1.0f, f2 + 1.0f};
    
    /* MMX variable */
    long long mmx_val = (long long)i1 << 32 | i2;
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int iter = 0; iter < iterations; iter++) {
        /* ====== BLOCK 1: Integer register pressure ====== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int a = i1 * iter + 123;
        int b = i2 * iter + 456;
        int c = a ^ b;
        int d = (a + b) * iter;
        
        /* Clobber integer registers - simulating a function call */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0x55555555, %%rcx\n\t"
            "mov $0xAAAAAAAA, %%rdx\n\t"
            "add $1, %%rax\n\t"
            "sub $1, %%rbx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += a + b + c + d;
        
        /* ====== BLOCK 2: SSE register pressure ====== */
        /* Use xmm0-xmm5 (call-clobbered) */
        v4sf v1 = vec_f + (float)iter;
        v4sf v2 = vec_f * (float)(iter + 1);
        v4sf v3 = v1 + v2;
        v4sf v4 = v1 * v2;
        float sum1 = v3[0] + v3[1] + v3[2] + v3[3];
        float sum2 = v4[0] + v4[1] + v4[2] + v4[3];
        
        /* Clobber SSE registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use SSE values after clobber */
        result += (int)(sum1 + sum2);
        
        /* ====== BLOCK 3: Mixed register pressure with control flow ====== */
        /* Create a basic block ending with clobber */
        if (iter % 3 == 0) {
            /* More integer work */
            int x = i1 * i2 + iter;
            int y = x ^ (iter * 7);
            
            /* Clobber and immediately branch - creates block end at asm */
            asm volatile (
                "# Clobber before branch\n\t"
                "mov $0x11111111, %%r8\n\t"
                "mov $0x22222222, %%r9\n\t"
                "mov $0x33333333, %%r10\n\t"
                :
                :
                : "r8", "r9", "r10", "memory"
            );
            
            /* This use after asm forces save BEFORE the asm */
            result += x * y;
            
            /* Label to create control flow edge after the asm */
            if (y > 100) {
                result += 1000;
            }
        } else {
            /* MMX/SSE mix */
            mmx_val += iter;
            v4si vi1 = vec_i + iter;
            v4si vi2 = vec_i * iter;
            
            /* Clobber MMX and more SSE */
            asm volatile (
                "# Clobber MMX/SSE\n\t"
                "pxor %%mm0, %%mm0\n\t"
                "pxor %%mm1, %%mm1\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                :
                :
                : "mm0", "mm1", "xmm6", "xmm7", "memory"
            );
            
            /* Use values after clobber */
            result += vi1[0] + vi2[1] + (int)(mmx_val & 0xFF);
        }
        
        /* ====== BLOCK 4: Function call that ends basic block ====== */
        /* Force a real function call that clobbers registers */
        if (iter % 4 == 0) {
            /* Compute value that must survive call */
            int pre_call = result * 3 + iter;
            
            /* Call to external function - creates block end at call */
            int r = rand() % 100;
            
            /* This use after call forces save BEFORE the call */
            result += pre_call ^ r;
            
            /* Immediate label/jump after call to create edge */
            if (r > 50) {
                result += 777;
            } else {
                result -= 333;
            }
        }
        
        /* Update volatile to prevent loop optimizations */
        vi += iter % 7;
    }
    
    return result;
}

/* Another function to create interprocedural pressure */
static __attribute__((noinline))
int intermediate(int x, int y) {
    volatile int v = x;
    asm volatile (
        "# Intermediate clobber\n\t"
        "mov $0x44444444, %%r11\n\t"
        "mov $0x55555555, %%r12\n\t"
        :
        :
        : "r11", "r12", "memory"
    );
    return v * y + (v ^ y);
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(42);
    int total = 0;
    
    /* Multiple calls with different arguments */
    total += test_caller_save(iterations, 1);
    total += test_caller_save(iterations / 2, 100);
    total += intermediate(iterations, total);
    total += test_caller_save(iterations / 3, 1000);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Also use in asm to force register usage */
    asm volatile (
        "# Final use of result\n\t"
        "add $1, %0\n\t"
        : "+r" (total)
        :
        : "cc"
    );
    
    printf("Final: %d\n", total);
    return total != 0 ? 0 : 1;
}
