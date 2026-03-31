/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types with volatile to extend liveness */
    volatile int vi1 = seed * 3;
    volatile int vi2 = seed + 7;
    volatile double vd1 = seed * 0.5;
    volatile double vd2 = seed * 1.5;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    volatile v4si vec_int = {seed, seed + 1, seed + 2, seed + 3};
    volatile v4sf vec_float = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTERS ========== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int a = vi1 * i + 123;
        int b = vi2 * i + 456;
        int c = a * b - 789;
        
        /* Clobber integer registers - simulating function call */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use values after clobber - forces save/restore */
        result += c + a - b;
        
        /* ========== FLOATING POINT REGISTERS ========== */
        double d1 = vd1 * i + 3.14159;
        double d2 = vd2 * i + 2.71828;
        double d3 = d1 * d2 - 1.41421;
        
        /* Clobber xmm registers */
        asm volatile (
            "# Clobber xmm regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use FP values after clobber */
        result += (int)(d3 * 1000);
        
        /* ========== VECTOR REGISTERS ========== */
        v4si v1 = vec_int * i;
        v4si v2 = {i, i*2, i*3, i*4};
        v4si v3 = v1 + v2;
        
        /* Clobber more xmm/ymm registers */
        asm volatile (
            "# Clobber more vector regs\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            :
            :
            : "xmm4", "xmm5", "xmm6", "xmm7", "memory"
        );
        
        /* Use vector results */
        for (int j = 0; j < 4; j++) {
            result += v3[j];
        }
        
        /* ========== MIXED REGISTERS WITH CONTROL FLOW ========== */
        /* Create basic block ending with asm clobber */
        if (i % 3 == 0) {
            int temp = result * 2;
            
            /* This asm creates a call-like clobber at potential block end */
            asm volatile (
                "# Mixed clobber at potential block end\n\t"
                "mov $0, %%r8\n\t"
                "pxor %%xmm8, %%xmm8\n\t"
                :
                :
                : "r8", "r9", "r10", "r11", 
                  "xmm8", "xmm9", "xmm10", "xmm11", "memory"
            );
            
            /* Label/jump after clobber to create block boundary */
            result = temp + 1;
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# Alternative clobber\n\t"
                "mov $0, %%r12\n\t"
                "pxor %%xmm12, %%xmm12\n\t"
                :
                :
                : "r12", "r13", "r14", "r15",
                  "xmm12", "xmm13", "xmm14", "xmm15", "memory"
            );
            result += i;
        }
        
        /* Update volatiles to keep them live */
        vi1 += i;
        vd1 += 0.1;
        vec_int += (v4si){1, 1, 1, 1};
    }
    
    return result;
}

/* External function call to force caller-save around actual call */
static __attribute__((noinline)) 
int external_helper(int x, int y) {
    return x * y + 12345;
}

/* Another function to create call site with live values */
static __attribute__((noinline))
int complex_caller(int base) {
    volatile int v1 = base * 3;
    volatile int v2 = base * 7;
    volatile double f1 = base * 1.234;
    volatile double f2 = base * 5.678;
    
    /* Many live values across call */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        int a = v1 + i * 11;
        int b = v2 + i * 13;
        double c = f1 * i + 2.345;
        double d = f2 * i + 6.789;
        
        /* Function call with many live values - forces caller-save */
        int call_result = external_helper(a, b);
        
        /* Use all values after call */
        sum += call_result + (int)(c * 100) + (int)(d * 100);
        
        /* Additional asm clobber after call */
        asm volatile (
            "# Post-call clobber\n\t"
            "mov $0, %%rax\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx",
              "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use values again */
        sum += a + b + (int)c + (int)d;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int s = 0; s < 5; s++) {
        int r1 = test_caller_save(iterations, s * 100);
        int r2 = complex_caller(s * 50 + r1 % 100);
        total += r1 + r2;
        
        printf("Iteration %d: r1=%d, r2=%d, total=%d\n", 
               s, r1, r2, total);
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
