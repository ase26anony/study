/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int helper_function(int x) {
    return x * 3 + 7;
}

/* Another non-inlineable function */
static __attribute__((noinline))
float float_helper(float x) {
    return x * 1.5f - 2.0f;
}

/* Vector type for SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* MMX type */
typedef long long mmx_t __attribute__((vector_size(8)));

/* Core function with caller-save pressure */
static __attribute__((noinline))
unsigned long test_caller_save(int iterations, int seed) {
    volatile int vol_var = seed;  /* Prevent optimizations */
    unsigned long checksum = 0;
    int i;
    
    /* Force many different values into call-clobbered registers */
    for (i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTERS ========== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        long rax_val = vol_var + i * 17;
        long rbx_val = vol_var * 3 - i * 11;
        long rcx_val = (vol_var << 3) | (i & 0xFF);
        long rdx_val = vol_var ^ (i * 0x1234);
        
        /* Clobber integer registers - simulating function call effects */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            : 
            : "a"(rax_val), "b"(rbx_val), "c"(rcx_val), "d"(rdx_val)
            : "memory"
        );
        
        /* Use values after clobber - forces save/restore */
        checksum += rax_val * 3;
        checksum ^= rbx_val + rcx_val;
        checksum += rdx_val << 2;
        
        /* Real function call that clobbers registers */
        int call_result = helper_function(vol_var + i);
        checksum += call_result;
        
        /* ========== FLOATING POINT / SSE REGISTERS ========== */
        /* Use xmm0-xmm3 (call-clobbered) */
        double xmm0_val = vol_var * 0.5 + i * 0.25;
        float xmm1_val = vol_var * 1.7f - i * 0.3f;
        v4sf xmm2_val = {vol_var * 1.1f, vol_var * 2.2f, 
                         vol_var * 3.3f, vol_var * 4.4f};
        v4si xmm3_val = {vol_var + i, vol_var - i, 
                         vol_var * i, vol_var ^ i};
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            : 
            : "x"(xmm0_val), "x"(xmm1_val), "x"(xmm2_val), "x"(xmm3_val)
            : "memory"
        );
        
        /* Use SSE values after clobber */
        checksum += (unsigned long)(xmm0_val * 100.0);
        checksum += (unsigned long)(xmm1_val * 50.0f);
        for (int j = 0; j < 4; j++) {
            checksum += (unsigned long)xmm2_val[j];
            checksum ^= xmm3_val[j];
        }
        
        /* Another function call - basic block boundary opportunity */
        float float_res = float_helper(xmm1_val);
        checksum += (unsigned long)(float_res * 1000.0f);
        
        /* ========== MMX REGISTERS ========== */
        /* Use mm0-mm1 (call-clobbered) */
        mmx_t mm0_val = {vol_var * 2LL + i};
        mmx_t mm1_val = {vol_var * 3LL - i * 5LL};
        
        /* Clobber MMX registers */
        asm volatile (
            "# CLOBBER MMX REGS\n\t"
            : 
            : "y"(mm0_val), "y"(mm1_val)
            : "memory"
        );
        
        /* Use MMX values - forces save/restore around asm */
        checksum += mm0_val[0] ^ 0xAAAA;
        checksum += mm1_val[0] & 0x5555;
        
        /* Critical: Create a basic block ending with a call */
        /* followed by label/jump for block boundary */
        if (checksum & 1) {
            /* Function call at potential block end */
            int extra_call = helper_function(checksum & 0xFF);
            checksum += extra_call * 7;
            /* Label/jump creates control flow edge */
            goto update_volatile;  /* Forces block boundary after call */
        } else {
            int extra_call = helper_function(checksum & 0x7F);
            checksum -= extra_call * 3;
        }
        
    update_volatile:
        /* Update volatile to extend liveness across iterations */
        vol_var = (vol_var * 1103515245U + 12345U) & 0x7FFFFFFF;
        
        /* Mix in another clobbering asm to create more insertion points */
        asm volatile (
            "# FINAL CLOBBER\n\t"
            : 
            : "r"(checksum)
            : "rax", "rbx", "rcx", "rdx", 
              "xmm0", "xmm1", "xmm2", "xmm3",
              "mm0", "mm1", "memory"
        );
    }
    
    return checksum;
}

/* Wrapper to create multiple call sites */
static __attribute__((noinline))
unsigned long test_wrapper(int base) {
    unsigned long total = 0;
    
    /* Multiple calls with different arguments */
    total += test_caller_save(10, base);
    total += test_caller_save(15, base + 1);
    total += test_caller_save(20, base + 2);
    total += test_caller_save(25, base + 3);
    
    return total;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    unsigned long final_checksum = 0;
    
    /* Multiple invocations to increase caller-save opportunities */
    for (int i = 0; i < iterations; i++) {
        final_checksum ^= test_wrapper(i * 17);
        final_checksum += test_caller_save(5, i * 23);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %lu\n", final_checksum);
    
    /* Also print to volatile memory to ensure execution */
    volatile unsigned long sink = final_checksum;
    (void)sink;
    
    return (final_checksum == 0) ? 1 : 0;
}
