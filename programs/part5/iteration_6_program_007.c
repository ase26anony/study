/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int helper_function(int x, int y) {
    volatile int result = x * y + 37;
    /* Use result to prevent optimization */
    asm volatile("" : "+r" (result) : : "memory");
    return result;
}

/* Another non-inlineable function */
static __attribute__((noinline))
float float_helper(float a, float b) {
    volatile float res = a * b - 3.14f;
    asm volatile("" : "+x" (res) : : "memory");
    return res;
}

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Main test function with caller-save pressure */
static __attribute__((noinline, noipa))
int test_caller_save(int iterations, int seed) {
    volatile int counter = iterations;  /* Prevent hoisting */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    /* Vector variables - live across calls */
    v4si vec_int = {seed, seed + 1, seed + 2, seed + 3};
    v4sf vec_float = {seed * 0.1f, seed * 0.2f, seed * 0.3f, seed * 0.4f};
    
    /* MMX-style 64-bit integer vector (if supported) */
    long long mmx_val = (long long)seed * 0x12345678;
    
    /* Force many different registers to be live across calls */
    for (int i = 0; i < counter; i++) {
        /* ========== INTEGER REGISTER PRESSURE ========== */
        /* Compute values in call-clobbered registers */
        int r1 = sum * 3 + i;
        int r2 = r1 ^ 0xABCD;
        int r3 = r2 << 3;
        
        /* Clobber multiple integer registers with asm */
        asm volatile(
            "# Clobber integer regs\n\t"
            "movq $0, %%rax\n\t"
            "movq $0, %%rbx\n\t"
            "movq $0, %%rcx\n\t"
            "movq $0, %%rdx\n\t"
            "movq $0, %%rsi\n\t"
            "movq $0, %%rdi\n\t"
            "movq $0, %%r8\n\t"
            "movq $0, %%r9\n\t"
            "movq $0, %%r10\n\t"
            "movq $0, %%r11\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        sum += r1 + r2 + r3;
        
        /* ========== FLOATING POINT REGISTER PRESSURE ========== */
        float f1 = fsum * 1.1f + i;
        float f2 = f1 * 0.9f - 2.0f;
        
        /* Clobber SSE/AVX registers */
        asm volatile(
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            "pxor %%xmm12, %%xmm12\n\t"
            "pxor %%xmm13, %%xmm13\n\t"
            "pxor %%xmm14, %%xmm14\n\t"
            "pxor %%xmm15, %%xmm15\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        fsum += f1 + f2;
        
        /* ========== VECTOR REGISTER PRESSURE ========== */
        /* Vector operations that use call-clobbered registers */
        vec_int += (v4si){i, i*2, i*3, i*4};
        vec_float *= (v4sf){1.01f, 1.02f, 1.03f, 1.04f};
        
        /* Clobber more registers, then use vectors */
        asm volatile(
            "# More clobbering\n\t"
            "movq $0, %%rax\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            :
            :
            : "rax", "xmm0", "memory"
        );
        
        /* Extract values from vectors to force them to be live */
        int vi[4];
        memcpy(vi, &vec_int, sizeof(vi));
        sum += vi[0] + vi[1] + vi[2] + vi[3];
        
        /* ========== ACTUAL FUNCTION CALLS ========== */
        /* These create basic blocks ending with calls */
        if (i % 3 == 0) {
            /* Call that might make BB_END == insn */
            int call_result = helper_function(sum, i);
            sum = call_result ^ sum;  /* Use result, keep sum live */
            
            /* Label to create control flow edge after call */
            after_call_1:
            sum += 1;
        } else if (i % 3 == 1) {
            /* Another call with floating point */
            float fresult = float_helper(fsum, i * 0.5f);
            fsum = fresult + fsum;
            
            after_call_2:
            sum += 2;
        } else {
            /* Third path with more register pressure */
            mmx_val += i * 0x1000;
            
            /* Clobber MMX registers if supported */
            #ifdef __MMX__
            asm volatile(
                "# Clobber MMX\n\t"
                "pxor %%mm0, %%mm0\n\t"
                "pxor %%mm1, %%mm1\n\t"
                "pxor %%mm2, %%mm2\n\t"
                "pxor %%mm3, %%mm3\n\t"
                "emms\n\t"
                :
                :
                : "mm0", "mm1", "mm2", "mm3", "memory"
            );
            #endif
            
            after_call_3:
            sum += 3;
        }
        
        /* ========== COMPLEX CONTROL FLOW ========== */
        /* Create multiple basic blocks with calls at the end */
        switch (i % 4) {
            case 0: {
                int tmp = helper_function(sum, i + 1);
                sum += tmp;
                /* Fall through - creates edge */
            }
            case 1: {
                float tmp = float_helper(fsum, i + 2.0f);
                fsum += tmp;
                break;
            }
            case 2: {
                /* Another asm clobber */
                asm volatile(
                    "movq $0, %%r12\n\t"
                    "movq $0, %%r13\n\t"
                    :
                    :
                    : "r12", "r13", "memory"
                );
                sum += i * 7;
                break;
            }
            default: {
                /* Empty default to ensure all cases covered */
                sum ^= 0xDEADBEEF;
                break;
            }
        }
    }
    
    /* Final computation using all live values */
    int final_vec_sum;
    memcpy(&final_vec_sum, &vec_int, sizeof(int));
    
    return sum + (int)fsum + final_vec_sum + (int)(mmx_val & 0xFFFFFFFF);
}

/* Wrapper to ensure multiple calls with different arguments */
static __attribute__((noinline))
int run_test_suite(void) {
    int total = 0;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        int result = test_caller_save(5 + (i % 3), 1000 + i * 100);
        total += result;
        
        /* Prevent optimization across iterations */
        asm volatile("" : "+r" (total) : : "memory");
    }
    
    return total;
}

int main(void) {
    printf("Starting caller-save test...\n");
    
    int result = run_test_suite();
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
        return 1;
    }
    
    return 0;
}
