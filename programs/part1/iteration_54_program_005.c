/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to force register pressure for argument passing */
__attribute__((noinline, optimize("O0"))) 
int use_values(int a, int b, float c, double d, int e, int f, float g, double h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    return sink & 1;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double compute_more(double x, double y, float z, int w, int v, uint64_t u) {
    volatile double result;
    result = x * y + z / w - v + u;
    return result;
}

/* Vector type to consume SIMD registers */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_sink;
volatile float global_float_sink;
volatile double global_double_sink;

int main(void) {
    /* Initialize arrays with varying data */
    double arr_d[256];
    float arr_f[256];
    int arr_i[256];
    uint64_t arr_u64[256];
    
    for (int i = 0; i < 256; i++) {
        arr_d[i] = (i * 1.2345) / (i + 1);
        arr_f[i] = (i * 0.9876f) / (i + 2);
        arr_i[i] = i * 3 - 7;
        arr_u64[i] = (uint64_t)i * 123456789ULL;
    }
    
    /* Volatile pointers to force repeated dereferencing */
    volatile double *volatile ptr_d = arr_d;
    volatile float *volatile ptr_f = arr_f;
    volatile int *volatile ptr_i = arr_i;
    volatile uint64_t *volatile ptr_u64 = arr_u64;
    
    /* Accumulator to prevent dead code elimination */
    double total = 0.0;
    float ftotal = 0.0f;
    int itotal = 0;
    
    /* Nested loops to maximize register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent arithmetic operations creating temporaries */
            double t1 = ptr_d[i] * 1.234 + ptr_d[i+1];
            double t2 = ptr_d[i+2] / (ptr_d[i+3] + 0.001);
            double t3 = t1 - t2 * 0.789;
            
            float f1 = ptr_f[i] * 2.345f + ptr_f[i+1];
            float f2 = ptr_f[i+2] / (ptr_f[i+3] + 0.001f);
            float f3 = f1 - f2 * 0.456f;
            
            int i1 = ptr_i[i] * 3 - ptr_i[i+1];
            int i2 = ptr_i[i+2] / ((ptr_i[i+3] & 0xFF) + 1);
            int i3 = i1 + i2 * 2 - (i % 7);
            
            uint64_t u1 = ptr_u64[i] + ptr_u64[i+1];
            uint64_t u2 = ptr_u64[i+2] - ptr_u64[i+3];
            uint64_t u3 = (u1 * u2) >> (i % 16);
            
            /* More computations with mixed types */
            double t4 = t3 * f3 + i3 * 0.01 - u3 * 0.000001;
            float f4 = f3 * (float)t3 + (float)i3 * 0.1f - (float)u3 * 0.0001f;
            int i4 = i3 + (int)t3 * 2 - (int)f3 * 3 + (int)(u3 & 0xFFFF);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple integer and floating-point registers */
            asm volatile("# Force register pressure\n\t"
                         : /* no outputs */
                         : /* no inputs */
                         : "rax", "rbx", "rcx", "rdx", 
                           "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "xmm6", "xmm7",
                           "memory");
            
            /* Call function with many arguments - forces register allocation */
            int r1 = use_values(i1, i2, f1, t1, i3, i4, f2, t2);
            
            /* More computations after call (registers need to be reloaded) */
            double t5 = t4 * 1.5 + f4 * 0.5;
            float f5 = f4 * 2.0f - (float)t4 * 0.3f;
            int i5 = i4 * 2 + r1 * 3 - (outer % 5);
            
            /* Vector operations to consume SIMD registers */
            v4sf vec1 = {f1, f2, f3, f4};
            v4sf vec2 = {f4, f3, f2, f1};
            v4sf vec3 = vec1 * vec2 + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
            
            v4si ivec1 = {i1, i2, i3, i4};
            v4si ivec2 = {i4, i3, i2, i1};
            v4si ivec3 = ivec1 + ivec2 * 2;
            
            /* Extract elements from vectors */
            float vec_elem;
            memcpy(&vec_elem, &vec3[2], sizeof(float));
            int ivec_elem = ivec3[1];
            
            /* Another function call with different arguments */
            double r2 = compute_more(t3, t4, f3, i3, i4, u3);
            
            /* More inline assembly */
            asm volatile("# More clobbering\n\t"
                         : 
                         : 
                         : "r8", "r9", "r10", "r11",
                           "xmm8", "xmm9", "xmm10", "xmm11",
                           "xmm12", "xmm13", "xmm14", "xmm15",
                           "memory");
            
            /* Complex expression with many temporaries */
            double final_val = 
                (t5 * 0.33 + r2 * 0.67) * 
                (1.0 + (double)i5 * 0.001 - (double)ivec_elem * 0.0001) /
                (1.0 + vec_elem * 0.01) *
                (double)((u3 + i1 * i2) & 0xFFF);
            
            /* Volatile writes to prevent optimization */
            global_sink = i5;
            global_float_sink = f5;
            global_double_sink = final_val;
            
            /* Accumulate results */
            total += final_val * (1.0 + outer * 0.01 + i * 0.0001);
            ftotal += f5 * (1.0f + outer * 0.01f + i * 0.0001f);
            itotal += i5 + ivec_elem + r1;
        }
        
        /* Additional computation between outer loop iterations */
        if (outer % 10 == 0) {
            /* Force spill/reload around this block */
            asm volatile("# Loop boundary clobber\n\t"
                         :
                         :
                         : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                           "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                           "memory");
        }
    }
    
    /* Print results to prevent optimization */
    printf("Results: total=%f, ftotal=%f, itotal=%d\n", total, ftotal, itotal);
    
    return (int)(total + ftotal + itotal) & 0xFF;
}
