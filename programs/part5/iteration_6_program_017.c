/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed);

/* Non-inlineable helper to simulate calls */
static __attribute__((noinline, noipa))
void clobber_helper(void) {
    /* Empty but prevents optimization */
}

/* Vector types for SSE/MMX registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

int main(void) {
    int result = 0;
    
    /* Call multiple times with different arguments */
    result += test_caller_save(10, 1);
    result += test_caller_save(5, 42);
    result += test_caller_save(8, 123);
    
    printf("Final checksum: %d\n", result);
    return result != 0 ? 0 : 1;
}

static __attribute__((noinline))
int test_caller_save(int iterations, int seed) {
    /* Volatile variables to extend liveness */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile float vf1 = seed * 1.5f;
    volatile float vf2 = seed * 2.5f;
    
    /* Vector variables */
    v4si vec_int = {seed, seed + 1, seed + 2, seed + 3};
    v4sf vec_float = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    v2di vec_double = {seed * 5LL, seed * 6LL};
    
    int sum = 0;
    int i;
    
    /* Loop with runtime iteration count to prevent hoisting */
    for (i = 0; i < iterations; i++) {
        /* ===== BLOCK 1: Integer register pressure ===== */
        int r1 = vi1 * 3 + i;
        int r2 = vi2 * 7 - i;
        
        /* Use in computation before clobber */
        int temp1 = r1 * r2 + (r1 ^ r2);
        
        /* Clobber multiple integer registers - simulating a call */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
        );
        
        /* Use the value after clobber - forces save/restore */
        sum += temp1;
        
        /* ===== BLOCK 2: SSE register pressure ===== */
        /* Create dependency chain with SSE values */
        v4sf f1 = vec_float + (float)i;
        v4sf f2 = vec_float * 2.0f - (float)i;
        
        /* Use SSE values before clobber */
        v4sf f_temp = f1 * f2 + f1;
        float f_sum = f_temp[0] + f_temp[1] + f_temp[2] + f_temp[3];
        
        /* Clobber SSE registers - this asm should end a basic block */
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
        
        /* Label after clobber to create control flow edge */
        if (f_sum > 0.0f) {
            sum += (int)f_sum;
        } else {
            /* Alternative path to create basic block structure */
            sum -= (int)f_sum;
        }
        
        /* ===== BLOCK 3: Mixed register pressure with actual call ===== */
        /* Use integer and vector values */
        v4si v_temp = vec_int + i;
        int v_sum = v_temp[0] + v_temp[1] + v_temp[2] + v_temp[3];
        
        /* Actual function call that ends a basic block */
        clobber_helper();
        
        /* Use values after call - forces caller-save insertion AFTER call */
        sum += v_sum * 2;
        
        /* ===== BLOCK 4: MMX and more SSE ===== */
        /* Use MMX-style operations (8-byte vectors) */
        long long mmx_val = vec_double[0] + i;
        long long mmx_val2 = vec_double[1] - i;
        
        /* Clobber MMX registers */
        asm volatile (
            "# Clobber MMX regs\n\t"
            "emms\n\t"  /* Empty MMX state */
            "movq %0, %%mm0\n\t"
            "movq %1, %%mm1\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            :
            : "r"(mmx_val), "r"(mmx_val2)
            : "mm0", "mm1", "memory"
        );
        
        /* Conditional jump to create basic block boundary */
        if (sum & 1) {
            /* Update volatile to prevent optimization */
            vi1 = sum % 100;
        } else {
            vi2 = sum % 50;
        }
        
        /* ===== BLOCK 5: Another call with live values ===== */
        /* More computations with different registers */
        double d1 = (double)vi1 * 1.234;
        double d2 = (double)vi2 * 5.678;
        
        /* Use in computation */
        double d_result = d1 * d2 - d1 / (d2 + 1.0);
        
        /* Another clobbering asm that could be at block end */
        asm volatile (
            "# Clobber more regs\n\t"
            "xchg %%rax, %%rax\n\t"  /* No-op but volatile */
            :
            :
            : "rax", "rbx", "rcx", "xmm6", "xmm7", "xmm8", "xmm9", "memory"
        );
        
        /* Use result after clobber */
        sum += (int)d_result;
        
        /* Update vectors for next iteration */
        vec_int += 1;
        vec_float += 1.0f;
        vec_double[0] += i;
        vec_double[1] -= i;
    }
    
    return sum;
}

/* Additional non-inlineable functions to increase call pressure */
static __attribute__((noinline, noipa))
void dummy_call_1(int a) {
    asm volatile ("# dummy1 %0" : : "r"(a) : "memory");
}

static __attribute__((noinline, noipa))
void dummy_call_2(float f) {
    asm volatile ("# dummy2 %xmm0" : : "x"(f) : "xmm0", "memory");
}
