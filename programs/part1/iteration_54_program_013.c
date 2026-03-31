/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* No-inline function to force argument passing */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int use_values(int a, int b, float c, double d, int e, float f, double g, int h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + (int)f + (int)g + h;
    return sink & 1;
}

/* Another no-inline function with mixed types */
__attribute__((noinline))
float complex_math(float x, float y, double z, int i, int j) {
    volatile float result;
    result = (x * y) + (float)z + (float)(i * j) + (x / y) - (float)(i % 7);
    return result;
}

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_sink;
volatile float global_float_sink;

int main(void) {
    /* Initialize arrays with varying values */
    float fa[256];
    double da[256];
    int ia[256];
    
    for (int i = 0; i < 256; i++) {
        fa[i] = (i * 1.234f) / (i + 1);
        da[i] = (i * 2.345) / (i + 2);
        ia[i] = i * 3;
    }
    
    /* Volatile pointer to force repeated dereferencing */
    volatile float* volatile_fp = fa;
    volatile double* volatile_dp = da;
    volatile int* volatile_ip = ia;
    
    /* Result accumulator */
    int total_result = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent arithmetic operations creating temporaries */
            float t1 = volatile_fp[i] * 1.1f + (float)outer;
            float t2 = volatile_fp[i+1] / 2.0f - (float)i;
            double t3 = volatile_dp[i] * 2.2 + (double)outer;
            double t4 = volatile_dp[i+1] / 3.0 - (double)i;
            int t5 = volatile_ip[i] * 3 + outer;
            int t6 = volatile_ip[i+1] / 4 - i;
            
            /* More temporaries with mixed operations */
            float t7 = t1 * t2 + (float)t3 - (float)t4;
            double t8 = (double)t1 / (double)t2 * t3 + t4;
            int t9 = t5 * t6 + (int)t1 - (int)t2;
            
            /* Even more temporaries with unique expressions */
            float t10 = t7 * (float)i + (float)outer / (t2 + 1.0f);
            double t11 = t8 * (double)outer - (double)i / (t4 + 1.0);
            int t12 = t9 * i + outer % (t6 + 1);
            
            /* Vector operations to consume SIMD registers */
            v4si vec_int = {t5, t6, t9, t12};
            v4sf vec_float = {t1, t2, t7, t10};
            
            vec_int = vec_int + (v4si){i, outer, i*2, outer*3};
            vec_float = vec_float * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber general purpose and SSE registers */
            asm volatile("" 
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Function call with many arguments - forces register allocation */
            int call_result = use_values(
                t5, t6, 
                t7, t8,  /* float and double args */
                t9, t10, /* int and float args */
                t11, t12 /* double and int args */
            );
            
            /* More computations after call */
            float t13 = complex_math(t1, t2, t3, i, outer);
            double t14 = (double)t13 * t8 - t11;
            int t15 = (int)t14 * t12 + call_result;
            
            /* Volatile writes to prevent optimization */
            global_sink = t15;
            global_float_sink = t13 + t10;
            
            /* Use vector results */
            int vec_sum = vec_int[0] + vec_int[1] + vec_int[2] + vec_int[3];
            float vec_prod = vec_float[0] * vec_float[1] * vec_float[2] * vec_float[3];
            
            /* Another assembly clobber */
            asm volatile("" ::: "rax", "rbx", "xmm0", "xmm1", "memory");
            
            /* Complex expression that might be rematerialized */
            total_result += (t5 * 2 - t6 / 3 + (int)t7 % 5) ^ 
                           (t9 & 0xFF) | 
                           (call_result << 3) +
                           vec_sum - (int)vec_prod +
                           (int)(t13 * 100.0f) +
                           (int)t14 % 7 +
                           t15;
            
            /* More independent computations */
            float t16 = volatile_fp[i+2] * 5.5f + (float)t15;
            double t17 = volatile_dp[i+2] * 6.6 + (double)vec_sum;
            int t18 = volatile_ip[i+2] * 7 + (int)t13;
            
            /* Use these in another function call */
            call_result = use_values(
                t18, vec_sum,
                t16, t17,
                (int)t16, t13,
                t14, call_result
            );
            
            total_result += call_result;
        }
        
        /* Additional loop with different access pattern */
        for (int j = 0; j < 64; j++) {
            /* Different computation pattern */
            float a = fa[j * 2] * 1.5f + fa[j * 2 + 1];
            double b = da[j * 2] * 2.5 + da[j * 2 + 1];
            int c = ia[j * 2] * 3 + ia[j * 2 + 1];
            
            /* Chain of dependent operations */
            for (int k = 0; k < 3; k++) {
                a = a * 1.1f - (float)k;
                b = b / 1.2 + (double)k;
                c = c + k * 2;
                
                /* Assembly to break live ranges */
                if (k == 1) {
                    asm volatile("" ::: "rcx", "rdx", "xmm2", "xmm3", "memory");
                }
            }
            
            total_result += (int)a + (int)b + c;
        }
    }
    
    printf("Result: %d\n", total_result);
    return total_result != 0;
}
