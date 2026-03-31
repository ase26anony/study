/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Non-inlineable function to force calls */
static __attribute__((noinline)) int external_func(int x) {
    return x * 3 + 7;
}

/* Another non-inlineable function */
static __attribute__((noinline)) double external_double(double x) {
    return x * 1.5 - 2.0;
}

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Target function with complex caller-save requirements */
static __attribute__((noinline, optimize("no-crossjumping"))) 
int test_caller_save(int iterations, int seed) {
    volatile int vol_int = seed;  /* Force memory operations */
    volatile double vol_double = seed * 1.5;
    
    /* Variables that will live across clobbering asm statements */
    int int_val1 = vol_int;
    int int_val2 = vol_int + 1;
    double fp_val1 = vol_double;
    double fp_val2 = vol_double * 2.0;
    v4si vec_int = {vol_int, vol_int + 1, vol_int + 2, vol_int + 3};
    v4sf vec_float = { (float)vol_double, (float)vol_double * 2.0f,
                       (float)vol_double * 3.0f, (float)vol_double * 4.0f };
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* --- BLOCK 1: Integer register pressure --- */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int_val1 = int_val1 * 1103515245 + 12345;
        int_val2 = int_val2 * 1664525 + 1013904223;
        
        /* Clobber integer registers - simulating a function call */
        asm volatile (
            "# Clobber integer registers\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += int_val1;
        result += int_val2;
        
        /* Real function call that ends a basic block */
        int_val1 = external_func(int_val1);
        
        /* --- BLOCK 2: Floating-point register pressure --- */
        /* Use xmm0-xmm5 (call-clobbered on x86-64 System V) */
        fp_val1 = fp_val1 * 1.6180339887 + 2.7182818284;
        fp_val2 = fp_val2 * 0.5772156649 - 1.4142135623;
        
        /* Clobber floating-point registers */
        asm volatile (
            "# Clobber xmm registers\n\t"
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
        
        /* Use FP values after clobber */
        result += (int)(fp_val1 * 1000.0);
        result += (int)(fp_val2 * 1000.0);
        
        /* Another function call - basic block boundary */
        fp_val1 = external_double(fp_val1);
        
        /* --- BLOCK 3: Vector register pressure --- */
        /* Use vector registers */
        vec_int = vec_int + (v4si){1, 2, 3, 4};
        vec_float = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber MMX/vector registers */
        asm volatile (
            "# Clobber mmx/vector registers\n\t"
            "emms\n\t"  /* Empty MMX state */
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            :
            :
            : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
              "xmm6", "xmm7", "memory"
        );
        
        /* Use vector values after clobber */
        result += vec_int[0] + vec_int[2];
        result += (int)vec_float[1];
        
        /* Conditional jump to create basic block structure */
        if (i % 3 == 0) {
            /* Function call at end of basic block */
            int_val2 = external_func(int_val2);
            /* Label to force block end update */
            __asm__ volatile ("# BB_END marker %0" : : "r"(i));
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# Alternative clobber\n\t"
                "mov $0, %%r8\n\t"
                "mov $0, %%r9\n\t"
                "mov $0, %%r10\n\t"
                :
                :
                : "r8", "r9", "r10", "memory"
            );
            fp_val2 = external_double(fp_val2);
        }
        
        /* Mix all values to create complex data flow */
        vol_int = result ^ int_val1 ^ int_val2;
        vol_double = (double)result / 1000.0 + fp_val1 - fp_val2;
    }
    
    return result;
}

/* Another function with different pattern */
static __attribute__((noinline)) 
int test_caller_save2(int iterations, int seed) {
    volatile long vol_long = seed;
    volatile __m128i vol_vec;
    
    long long_val = vol_long;
    __m128i vec_val = _mm_set_epi32(seed, seed+1, seed+2, seed+3);
    
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression using many temporaries */
        long_val = (long_val * 6364136223846793005UL) + 1442695040888963407UL;
        
        /* Multiple asm clobbers in sequence */
        asm volatile (
            "# Sequential clobber 1\n\t"
            "mov $0, %%r11\n\t"
            "mov $0, %%r12\n\t"
            :
            :
            : "r11", "r12", "memory"
        );
        
        result += (int)long_val;
        
        /* Function call */
        result = external_func(result);
        
        /* Vector operation */
        vec_val = _mm_add_epi32(vec_val, _mm_set1_epi32(i));
        
        asm volatile (
            "# Sequential clobber 2\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            :
            :
            : "xmm8", "xmm9", "memory"
        );
        
        /* Extract and use vector elements */
        int vec_elems[4];
        _mm_storeu_si128((__m128i*)vec_elems, vec_val);
        result += vec_elems[0] + vec_elems[3];
        
        /* Another call at potential block end */
        if (i & 1) {
            result = external_func(result);
            /* Potential insertion point after block-end call */
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int seed = 42;
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int total = 0;
    
    /* Call target functions multiple times with different patterns */
    total += test_caller_save(iterations, seed);
    total += test_caller_save(iterations, seed + 1);
    total += test_caller_save2(iterations / 2, seed + 2);
    total += test_caller_save(iterations / 3, seed + 3);
    total += test_caller_save2(iterations / 4, seed + 4);
    
    printf("Result: %d\n", total);
    printf("Checksum: 0x%08x\n", total ^ 0xDEADBEEF);
    
    return total != 0 ? 0 : 1;
}
