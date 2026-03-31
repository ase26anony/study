/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
#define NOINLINE __attribute__((noinline))

/* Vector types to use SSE/MMX registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Non-inlineable helper that clobbers registers */
NOINLINE void clobber_all(void) {
    /* Empty function that will be called */
}

/* Main test function with caller-save pressure */
NOINLINE static uint64_t test_caller_save(int iterations, int seed) {
    volatile int vol_iter = iterations;  /* Prevent optimization */
    volatile int vol_seed = seed;
    
    /* Live values in call-clobbered registers */
    register uint64_t r1 asm("rax") = vol_seed + 1;
    register uint64_t r2 asm("rbx") = vol_seed + 2;
    register uint64_t r3 asm("rcx") = vol_seed + 3;
    register double f1 asm("xmm0") = vol_seed * 1.5;
    register double f2 asm("xmm1") = vol_seed * 2.5;
    v4si vec1 = {vol_seed, vol_seed + 1, vol_seed + 2, vol_seed + 3};
    v4sf vec2 = {vol_seed * 1.1f, vol_seed * 1.2f, 
                  vol_seed * 1.3f, vol_seed * 1.4f};
    
    uint64_t sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < vol_iter; i++) {
        /* Basic block starting with computations */
        
        /* 1. Integer computations in call-clobbered registers */
        r1 = r1 * 6364136223846793005ULL + 1;
        r2 = r2 * 6364136223846793005ULL + 2;
        r3 = r3 * 6364136223846793005ULL + 3;
        
        /* Use the values before clobbering */
        sum += r1 ^ r2 ^ r3;
        
        /* 2. Clobber integer registers - simulates a call */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* 3. Floating point computations */
        f1 = f1 * 1.61803398875 + 1.0;
        f2 = f2 * 1.61803398875 + 2.0;
        
        /* Use floating point values */
        sum += (uint64_t)(f1 + f2);
        
        /* 4. Clobber floating point registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            : 
            : 
            : "xmm0", "xmm1", "memory"
        );
        
        /* 5. Vector computations */
        vec1 = vec1 + (v4si){1, 2, 3, 4};
        vec2 = vec2 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Extract and use vector elements */
        int* v1 = (int*)&vec1;
        float* v2 = (float*)&vec2;
        sum += v1[0] + v1[1] + v1[2] + v1[3];
        sum += (uint64_t)(v2[0] + v2[1] + v2[2] + v2[3]);
        
        /* 6. Actual function call at end of basic block */
        clobber_all();
        
        /* Label to create control flow edge */
        if (i & 1) {
            /* This creates a basic block ending with the call above */
            goto skip;
        }
        
        /* More computations after label */
        r1 = sum ^ r1;
        f1 = f1 + (double)sum;
        
        skip:
        /* Continue loop */
        ;
        
        /* 7. Another clobbering asm to force more saves */
        asm volatile (
            "# Clobber more regs\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            : 
            : 
            : "r10", "r11", "xmm2", "xmm3", "memory"
        );
        
        /* Use MMX registers too */
        register long long mmx_val asm("mm0") = sum;
        asm volatile (
            "# Use MMX\n\t"
            "movq %0, %%mm0\n\t"
            "paddq %%mm0, %%mm0\n\t"
            : 
            : "r"(mmx_val)
            : "mm0"
        );
        
        /* Final clobber */
        asm volatile (
            "# Final clobber\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "mm0", "memory"
        );
    }
    
    return sum;
}

/* Another function to create more call sites */
NOINLINE static uint64_t test_wrapper(int iter1, int iter2, int seed) {
    uint64_t sum = 0;
    
    /* Multiple calls with different arguments */
    sum += test_caller_save(iter1, seed);
    sum += test_caller_save(iter2, seed + 1);
    sum += test_caller_save(iter1 + 1, seed + 2);
    sum += test_caller_save(iter2 + 1, seed + 3);
    
    return sum;
}

int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    uint64_t total = 0;
    
    /* Multiple test runs with different parameters */
    total += test_wrapper(iterations, iterations / 2, 12345);
    total += test_wrapper(iterations / 3, iterations / 4, 67890);
    total += test_caller_save(iterations, 13579);
    total += test_caller_save(iterations / 2, 24680);
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %lu\n", (unsigned long)total);
    
    /* Also use inline asm to ensure values are live */
    asm volatile (
        "# Use the result\n\t"
        : 
        : "r"(total)
        : "memory"
    );
    
    return (total > 0) ? 0 : 1;
}
