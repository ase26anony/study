/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions and updating BB_END
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed);

/* Non-inlineable helper to force register pressure */
static __attribute__((noinline, optimize("O0")))
void clobber_helper(void) {
    /* Empty function that gets called */
}

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* MMX type */
typedef long long mmx_t __attribute__((vector_size(8)));

int main(void) {
    int total = 0;
    
    /* Call multiple times with different arguments to ensure
     * caller-save logic is exercised in different contexts */
    total += test_caller_save(10, 1);
    total += test_caller_save(5, 42);
    total += test_caller_save(8, 123);
    total += test_caller_save(3, 999);
    
    printf("Final checksum: %d\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total > 1000) {
        printf("Result is large enough\n");
    }
    
    return 0;
}

static __attribute__((noinline))
int test_caller_save(int iterations, int seed) {
    /* Volatile variables to extend register liveness */
    volatile int vol_int = seed;
    volatile float vol_float = seed * 1.5f;
    volatile v4si vol_vec_int;
    volatile v4sf vol_vec_float;
    volatile mmx_t vol_mmx;
    
    int result = 0;
    int i;
    
    /* Loop to prevent hoisting of save/restore code */
    for (i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTERS ========== */
        /* Force RAX to hold live value across clobber */
        long rax_val = vol_int + i * 3;
        
        /* Clobber RAX and RBX - simulating function call */
        asm volatile (
            "# CLOBBER RAX,RBX\n\t"
            "movq $0x12345678, %%rax\n\t"
            "movq $0x87654321, %%rbx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "memory"
        );
        
        /* Use original RAX value - forces save/restore */
        result += (int)(rax_val & 0xFF);
        
        /* Call actual function to create basic block boundaries */
        clobber_helper();
        
        /* ========== SSE REGISTERS ========== */
        /* Force XMM0 to hold live value */
        v4sf xmm0_val = (v4sf){vol_float + i, vol_float + i + 1,
                               vol_float + i + 2, vol_float + i + 3};
        
        /* Clobber XMM0 and XMM1 */
        asm volatile (
            "# CLOBBER XMM0,XMM1\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "memory"
        );
        
        /* Use original XMM0 value */
        float sum = xmm0_val[0] + xmm0_val[1] + xmm0_val[2] + xmm0_val[3];
        result += (int)sum;
        
        /* Another call to create basic block ending with call */
        clobber_helper();
        
        /* ========== MIXED REGISTER PRESSURE ========== */
        /* Create simultaneous pressure on multiple register classes */
        long rcx_val = vol_int * i;
        v4si xmm2_val = (v4si){i, i+1, i+2, i+3};
        mmx_t mm0_val = (mmx_t){rcx_val, rcx_val + 1};
        
        /* Massive clobber of many registers */
        asm volatile (
            "# CLOBBER MANY REGISTERS\n\t"
            "movq $0, %%rcx\n\t"
            "movq $0, %%rdx\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rcx", "rdx", "xmm2", "xmm3", "mm0", "mm1", "memory"
        );
        
        /* Use all original values - forces multiple saves/restores */
        result += (int)(rcx_val & 0xFF);
        result += xmm2_val[0] + xmm2_val[1];
        result += (int)mm0_val[0];
        
        /* Store to volatile to ensure values are used */
        vol_vec_int = xmm2_val;
        vol_mmx = mm0_val;
        
        /* Conditional jump to create basic block structure */
        if (result % 2 == 0) {
            /* Call at end of basic block */
            clobber_helper();
            /* Label to ensure BB_END is after the call */
            asm volatile("# BB_END marker %0" : : "r"(result));
        } else {
            /* Alternative path with different register usage */
            long r8_val = result * 2;
            asm volatile (
                "# CLOBBER R8,R9\n\t"
                "movq $0, %%r8\n\t"
                "movq $0, %%r9\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "memory"
            );
            result += (int)(r8_val & 0xFF);
            /* Another call at potential BB end */
            clobber_helper();
        }
        
        /* Update volatile to prevent optimization */
        vol_int = result;
        vol_float = result * 0.5f;
    }
    
    /* Final computation using all volatile values */
    result += vol_int;
    result += (int)vol_float;
    
    /* One more call sequence with register pressure */
    long final_val = result * 3;
    asm volatile (
        "# FINAL CLOBBER\n\t"
        "movq $0, %%r10\n\t"
        "movq $0, %%r11\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r10", "r11", "xmm4", "xmm5", "memory"
    );
    
    result += (int)(final_val & 0xFF);
    
    return result;
}

/* Additional non-inlineable functions to increase call density */
static __attribute__((noinline)) void dummy1(int x) { asm volatile("# dummy1 %0" : : "r"(x)); }
static __attribute__((noinline)) void dummy2(float x) { asm volatile("# dummy2 %0" : : "f"(x)); }
static __attribute__((noinline)) void dummy3(v4si x) { asm volatile("# dummy3 %0" : : "x"(x)); }

/* Function that gets called in hot path */
static __attribute__((noinline))
void hot_path_helper(int *result) {
    /* Use various registers */
    long tmp = *result;
    v4sf vec = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    
    asm volatile (
        "# HOT PATH CLOBBER\n\t"
        "movq $0, %%r12\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r12", "xmm6", "memory"
    );
    
    *result += (int)tmp + (int)vec[0];
}
