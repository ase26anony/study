/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
#define NOINLINE __attribute__((noinline))

/* Vector types to use SSE/MMX registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Non-inlineable helper that clobbers registers */
NOINLINE static int dummy_call(int x) {
    return x + 1;
}

/* Non-inlineable function that uses MMX */
NOINLINE static void mmx_operation(void) {
    __asm__ volatile ("movq %0, %%mm0" : : "r"(0x1122334455667788ULL) : "mm0");
}

/* The main test function with caller-save pressure */
NOINLINE static unsigned long test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Prevent optimizations */
    volatile float vf = seed * 1.5f;
    volatile v4si vvec = {seed, seed + 1, seed + 2, seed + 3};
    volatile v4sf vfvec = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    
    unsigned long result = 0;
    int i;
    
    /* Loop to prevent hoisting of save/restore */
    for (i = 0; i < iterations; i++) {
        /* ===== INTEGER REGISTERS ===== */
        /* Use multiple integer registers with volatile to keep them live */
        volatile long rax_val = vi + i * 3;
        volatile long rbx_val = vi + i * 5;
        volatile long rcx_val = vi + i * 7;
        volatile long rdx_val = vi + i * 11;
        
        /* Clobber integer registers - simulating function call effects */
        __asm__ volatile (
            "mov %0, %%rax\n\t"
            "mov %1, %%rbx\n\t"
            "mov %2, %%rcx\n\t"
            "mov %3, %%rdx\n\t"
            "add $1, %%rax\n\t"
            "add $2, %%rbx\n\t"
            "add $3, %%rcx\n\t"
            "add $4, %%rdx"
            : 
            : "r"(rax_val), "r"(rbx_val), "r"(rcx_val), "r"(rdx_val)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        /* Use the values after clobbering - forces save/restore */
        result += rax_val + rbx_val + rcx_val + rdx_val;
        
        /* Real function call that ends a basic block */
        vi = dummy_call(vi);
        /* Basic block ends here, next instruction is jump back to loop start */
        
        /* ===== SSE REGISTERS ===== */
        /* Use SSE registers */
        v4sf fvec = vfvec;
        fvec = fvec * (vf + i);
        
        /* Clobber SSE registers */
        __asm__ volatile (
            "movaps %0, %%xmm0\n\t"
            "movaps %1, %%xmm1\n\t"
            "addps %%xmm1, %%xmm0"
            : 
            : "x"(fvec), "x"(vfvec)
            : "xmm0", "xmm1"
        );
        
        /* Use SSE result */
        float fsum = fvec[0] + fvec[1] + fvec[2] + fvec[3];
        result += (unsigned long)fsum;
        
        /* Another function call */
        if (i % 2 == 0) {
            vi = dummy_call(vi + 1);
        } else {
            /* Different path to create control flow */
            vi = dummy_call(vi - 1);
        }
        
        /* ===== MIXED REGISTERS ===== */
        /* Use both integer and vector registers */
        v4si ivec = vvec;
        ivec = ivec + i;
        
        /* Clobber multiple register types */
        __asm__ volatile (
            "movdqa %0, %%xmm2\n\t"
            "mov %1, %%r8\n\t"
            "mov %2, %%r9\n\t"
            "paddd %%xmm2, %%xmm2\n\t"
            "add $10, %%r8\n\t"
            "add $20, %%r9"
            : 
            : "x"(ivec), "r"(vi), "r"(i)
            : "xmm2", "r8", "r9", "cc"
        );
        
        /* Use the values */
        result += ivec[0] + ivec[1] + ivec[2] + ivec[3];
        
        /* Call that uses MMX */
        if (i % 3 == 0) {
            mmx_operation();
            /* Basic block may end here */
        }
        
        /* ===== ANOTHER INTEGER SET ===== */
        /* More integer register pressure */
        volatile long r8_val = result & 0xFF;
        volatile long r9_val = (result >> 8) & 0xFF;
        volatile long r10_val = (result >> 16) & 0xFF;
        volatile long r11_val = (result >> 24) & 0xFF;
        
        /* Clobber another set */
        __asm__ volatile (
            "mov %0, %%r8\n\t"
            "mov %1, %%r9\n\t"
            "mov %2, %%r10\n\t"
            "mov %3, %%r11\n\t"
            "xor $0xAA, %%r8\n\t"
            "xor $0x55, %%r9"
            : 
            : "r"(r8_val), "r"(r9_val), "r"(r10_val), "r"(r11_val)
            : "r8", "r9", "r10", "r11", "cc"
        );
        
        result += r8_val + r9_val + r10_val + r11_val;
        
        /* Final call in the loop iteration */
        vi = dummy_call(result & 0xFF);
    }
    
    return result;
}

/* Another function to create more call sites */
NOINLINE static unsigned long test_caller_save_2(int iterations, int seed) {
    volatile double d1 = seed * 1.234;
    volatile double d2 = seed * 5.678;
    unsigned long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Use and clobber xmm registers */
        double temp = d1 * i + d2;
        
        __asm__ volatile (
            "movsd %0, %%xmm3\n\t"
            "movsd %1, %%xmm4\n\t"
            "addsd %%xmm4, %%xmm3"
            : 
            : "x"(temp), "x"(d2)
            : "xmm3", "xmm4"
        );
        
        result += (unsigned long)temp;
        
        /* Call that might end basic block */
        if (i % 4 == 0) {
            result += dummy_call(i);
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int seed = 12345;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Testing caller-save insertion logic...\n");
    
    unsigned long total = 0;
    
    /* Multiple calls to increase pressure */
    total += test_caller_save(iterations, seed);
    total += test_caller_save_2(iterations / 2, seed + 1);
    total += test_caller_save(iterations / 3, seed + 2);
    total += test_caller_save_2(iterations / 4, seed + 3);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %lu\n", total);
    
    /* Verify with simple computation */
    unsigned long expected = 0;
    for (int i = 0; i < iterations; i++) {
        expected += seed + i;
    }
    printf("Expected approximate: %lu\n", expected * 2);
    
    return (total > 0) ? 0 : 1;
}
