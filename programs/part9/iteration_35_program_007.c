#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);
extern void clobber_func4(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Prevent optimization of critical variables */
static volatile int global_counter = 0;

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* ===== DECLARE MANY LOCAL VARIABLES ===== */
    /* Integer variables - will compete for general purpose registers */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Floating point variables - will compete for x87/SSE registers */
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Vector variables - will compete for SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Pointer variables - additional register pressure */
    volatile int *p1 = &v1, *p2 = &v2, *p3 = &v3;
    volatile float *fp1 = &f1, *fp2 = &f2;
    volatile double *dp1 = &d1, *dp2 = &d2;
    
    /* Additional variables to ensure spill */
    volatile long l1 = 100, l2 = 200, l3 = 300;
    volatile short s1 = 10, s2 = 20, s3 = 30;
    volatile char c1 = 'a', c2 = 'b', c3 = 'c';
    
    /* ===== COMPUTATIONS TO KEEP VARIABLES LIVE ===== */
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        global_counter++;
        
        /* Complex computations before call - all variables must stay live */
        v1 = v2 + v3 * i;
        v4 = v5 ^ v6;
        v7 = v8 | v9;
        v10 = v11 & v12;
        v13 = v14 << 2;
        v15 = v1 >> 1;
        
        f1 = f2 * f3 + (float)i;
        f4 = f5 / f2;
        d1 = d2 + d3 * i;
        d4 = d5 - d1;
        
        /* Vector operations */
        vec1 = vec1 + vec2 * (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
        vec3 = vec3 - vec1;
        dvec1 = dvec1 * dvec2;
        ivec1 = ivec1 + ivec2;
        
        /* Pointer arithmetic */
        *p1 = *p2 + *p3;
        *fp1 = *fp2 * 2.0f;
        *dp1 = *dp2 / 2.0;
        
        /* Mixed type computations */
        l1 = (long)v1 * (long)v2;
        s1 = (short)(v3 + v4);
        c1 = (char)(v5 % 26) + 'a';
        
        /* ===== FORCE REGISTER CLOBBERING BEFORE CALL ===== */
        /* Clobber integer registers */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Clobber floating point/vector registers */
        asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                     "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                     "xmm12", "xmm13", "xmm14", "xmm15", "st", "st(1)",
                     "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* ===== EXTERNAL FUNCTION CALL ===== */
        /* This forces caller-save logic */
        if (i % 2 == 0) {
            clobber_func1();
        } else {
            clobber_func2();
        }
        
        /* ===== ADDITIONAL CLOBBERING AFTER CALL ===== */
        /* Different register clobbering to force restore */
        asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "xmm3");
        asm volatile("" : : : "memory");
        
        /* Another external call with different clobbering pattern */
        if (i % 3 == 0) {
            clobber_func3();
        } else {
            clobber_func4();
        }
        
        /* More aggressive clobbering */
        asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2",
                     "xmm3", "xmm4", "xmm5", "memory");
        
        /* ===== COMPUTATIONS AFTER CALLS ===== */
        /* Use all variables again to ensure they stay live across calls */
        v2 = v3 + v4 * i;
        v5 = v6 ^ v7;
        v8 = v9 | v10;
        v11 = v12 & v13;
        v14 = v15 << 1;
        v1 = v2 >> 2;
        
        f2 = f3 * f4 + (float)(i * 2);
        f5 = f1 / f3;
        d2 = d3 + d4 * i;
        d5 = d1 - d2;
        
        /* More vector operations */
        vec2 = vec2 + vec3 * (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
        vec1 = vec1 - vec2;
        dvec2 = dvec2 * dvec1;
        ivec2 = ivec2 + ivec1;
        
        /* More pointer operations */
        *p2 = *p3 + *p1;
        *fp2 = *fp1 * 3.0f;
        *dp2 = *dp1 / 3.0;
        
        /* Accumulate results to prevent elimination */
        sum_int += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15;
        
        /* Extract and sum vector elements */
        float vec1_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        float vec2_sum = vec2[0] + vec2[1] + vec2[2] + vec2[3];
        sum_float += f1 + f2 + f3 + f4 + f5 + vec1_sum + vec2_sum;
        
        sum_double += d1 + d2 + d3 + d4 + d5 +
                     dvec1[0] + dvec1[1] + dvec2[0] + dvec2[1];
        
        /* Additional mixed computations */
        l2 = (long)v6 * (long)v7;
        s2 = (short)(v8 + v9);
        c2 = (char)(v10 % 26) + 'a';
        
        /* Conditional to create complex CFG */
        if (sum_int > 1000) {
            /* More clobbering in conditional path */
            asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
            clobber_func1();
            asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
        }
    }
    
    /* Final aggregation and output to prevent dead code elimination */
    double final_result = (double)sum_int + (double)sum_float + sum_double;
    printf("Result: %f (counter: %d)\n", final_result, global_counter);
    
    /* Use all variables one more time */
    volatile int final_check = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                              v11 + v12 + v13 + v14 + v15;
    (void)final_check; /* Suppress unused warning */
    
    return (int)final_result % 256;
}
