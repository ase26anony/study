/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Non-inlineable helper to force calls */
static __attribute__((noinline)) int helper(int x) {
    return x * 3 + 7;
}

/* Vector types to use SSE/MMX registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Main test function with caller-save pressure */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vol = seed;  /* Prevent optimizations */
    int i, result = 0;
    
    /* Force many call-clobbered registers to be live */
    register long rax_val asm("rax") = vol + 1;
    register long rbx_val asm("rbx") = vol + 2;
    register long rcx_val asm("rcx") = vol + 3;
    register long rdx_val asm("rdx") = vol + 4;
    register double xmm0_val asm("xmm0") = vol * 1.5;
    register double xmm1_val asm("xmm1") = vol * 2.5;
    v4si vec_int = {vol, vol+1, vol+2, vol+3};
    v4sf vec_float = {vol*1.1f, vol*1.2f, vol*1.3f, vol*1.4f};
    
    /* Loop to prevent hoisting of save/restore */
    for (i = 0; i < iterations; i++) {
        /* Basic block that will end with a call/asm */
        
        /* 1. Use integer registers before clobber */
        rax_val = rax_val * 3 + i;
        rbx_val = rbx_val * 5 - i;
        
        /* 2. Clobber call-clobbered registers with asm (simulating call) */
        /* This creates a basic block ending with asm */
        asm volatile (
            "movq %0, %%rax\n\t"
            "movq %1, %%rbx\n\t"
            : 
            : "r"(rax_val), "r"(rbx_val)
            : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7", "mm0", "mm1", "mm2", "mm3",
              "mm4", "mm5", "mm6", "mm7", "memory"
        );
        
        /* 3. Use the values after clobber - forces save/restore */
        result += (int)rax_val + (int)rbx_val;
        
        /* 4. Use SSE registers */
        xmm0_val = xmm0_val * 1.7 + i;
        xmm1_val = xmm1_val * 2.3 - i;
        
        /* 5. Another clobbering asm - basic block ends here */
        asm volatile (
            "addpd %0, %%xmm0\n\t"
            "subpd %1, %%xmm1\n\t"
            : 
            : "x"(xmm0_val), "x"(xmm1_val)
            : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7", "mm0", "mm1", "mm2", "mm3",
              "mm4", "mm5", "mm6", "mm7", "memory"
        );
        
        /* 6. Use vector types */
        vec_int += (v4si){i, i*2, i*3, i*4};
        vec_float = vec_float * 1.5f + (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
        
        /* 7. Real function call - basic block definitely ends here */
        int call_result = helper(vol + i);
        
        /* Label to create control flow edge after call */
        if (call_result > 100) {
            /* Use vector results after call */
            result += vec_int[0] + vec_int[1];
            result += (int)vec_float[0];
            
            /* Another asm clobber after conditional */
            asm volatile (
                "movq %0, %%mm0\n\t"
                "paddd %1, %%mm0\n\t"
                : 
                : "r"(result), "x"(vec_int)
                : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "xmm6", "xmm7", "mm0", "mm1", "mm2", "mm3",
                  "mm4", "mm5", "mm6", "mm7", "memory"
            );
        } else {
            /* Different path with different register usage */
            result -= vec_int[2] + vec_int[3];
            result -= (int)vec_float[1];
        }
        
        /* Update volatile to prevent loop unrolling */
        vol = result % 256;
    }
    
    /* Mix all values for final result */
    result += (int)rax_val + (int)rbx_val;
    result += (int)xmm0_val + (int)xmm1_val;
    result += vec_int[0] + vec_int[1] + vec_int[2] + vec_int[3];
    
    return result;
}

/* Second test function with different pattern */
static __attribute__((noinline))
int test_caller_save2(int iterations, int seed) {
    volatile int vol = seed;
    int result = 0;
    
    /* Use MMX registers */
    register long long mm0_val asm("mm0") = vol * 2LL;
    register long long mm1_val asm("mm1") = vol * 3LL;
    
    for (int i = 0; i < iterations; i++) {
        /* Alternate between different clobber patterns */
        if (i % 3 == 0) {
            /* Pattern 1: asm clobber then call */
            mm0_val = mm0_val + i * 7LL;
            
            asm volatile (
                "movq %0, %%mm0\n\t"
                "psllq $2, %%mm0\n\t"
                : 
                : "r"(mm0_val)
                : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "mm0", "mm1",
                  "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", "memory"
            );
            
            /* Function call right after asm */
            result += helper(vol + i);
            
            /* Use mmx value after call */
            result += (int)(mm0_val >> 32);
        } 
        else if (i % 3 == 1) {
            /* Pattern 2: call then asm clobber */
            result += helper(vol - i);
            
            mm1_val = mm1_val - i * 5LL;
            
            asm volatile (
                "movq %0, %%mm1\n\t"
                "psrlq $1, %%mm1\n\t"
                : 
                : "r"(mm1_val)
                : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "mm0", "mm1",
                  "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", "memory"
            );
            
            result += (int)(mm1_val & 0xFFFFFFFF);
        }
        else {
            /* Pattern 3: back-to-back asm clobbers */
            asm volatile (
                "mov $0x12345678, %%eax\n\t"
                "mov $0x87654321, %%ebx\n\t"
                : 
                : 
                : "rax", "rbx", "rcx", "memory"
            );
            
            asm volatile (
                "xorps %%xmm0, %%xmm0\n\t"
                "addsd %0, %%xmm0\n\t"
                : 
                : "x"((double)(vol + i))
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            result += i * 11;
        }
        
        vol = (vol * 1103515245LL + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int seed = 42;
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Call test functions multiple times with different arguments */
    int total = 0;
    
    total += test_caller_save(iterations, seed);
    total += test_caller_save(iterations, seed + 1);
    total += test_caller_save2(iterations / 2, seed + 2);
    total += test_caller_save2(iterations / 2, seed + 3);
    
    /* Additional calls with small iteration counts */
    for (int i = 0; i < 5; i++) {
        total += test_caller_save(3, seed + 10 + i);
        total += test_caller_save2(2, seed + 20 + i);
    }
    
    printf("Result: %d\n", total);
    return total & 255;
}
