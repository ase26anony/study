#include <stdio.h>
#include <stdlib.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    /* Force many live variables across calls */
    volatile int v1 = argc * 1;
    volatile int v2 = argc * 2;
    volatile int v3 = argc * 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc * 5;
    volatile int v6 = argc * 6;
    volatile int v7 = argc * 7;
    volatile int v8 = argc * 8;
    
    volatile float f1 = argc * 1.1f;
    volatile float f2 = argc * 2.2f;
    volatile float f3 = argc * 3.3f;
    volatile float f4 = argc * 4.4f;
    
    volatile double d1 = argc * 1.11;
    volatile double d2 = argc * 2.22;
    volatile double d3 = argc * 3.33;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    
    /* Vector variables - use many to pressure SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Additional variables to ensure spilling */
    volatile long l1 = argc * 100L;
    volatile long l2 = argc * 200L;
    volatile short s1 = argc * 10;
    volatile short s2 = argc * 20;
    
    /* Complex computation creating data dependencies */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    volatile int result = 0;
    
    /* Loop with conditional control flow */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        v1 = v2 + v3 * i;
        v2 = v4 ^ v5;
        v3 = v6 | v7;
        v4 = v8 << 2;
        
        f1 = f2 * f3 + (float)i;
        f2 = f4 / (f3 + 1.0f);
        
        d1 = d2 * d3 - (double)i;
        d2 = d1 / (d3 + 1.0);
        
        /* Vector operations */
        vec1 = vec2 + vec3 * (float)(i + 1);
        vec2 = vec3 - vec4;
        vec3 = vec1 * vec2;
        
        dvec1 = dvec2 * 2.0;
        dvec2 = dvec1 + 1.0;
        
        ivec1 = ivec2 << 1;
        ivec2 = ivec1 | v4si{1, 1, 1, 1};
        
        /* Pointer arithmetic */
        p1 = p1 + 1;
        p2 = p2 - 1;
        fp1 = (float*)((char*)fp1 + 1);
        
        /* Conditional branch creating complex CFG */
        if (i % 2 == 0) {
            /* First clobber asm - force save of specific registers */
            asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3");
            
            /* External call - forces caller-save */
            clobber_func1();
            
            /* Second clobber asm - different registers */
            asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                         "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* More computations with live variables */
            v5 = v6 * v7 + v8;
            v6 = v1 ^ v2;
            
            f3 = f1 * f2 - f4;
            f4 = f3 / (f2 + 0.5f);
            
            d3 = d1 + d2 * 0.5;
            
            vec4 = vec1 + vec2 * vec3;
        } else {
            /* Alternative path with different clobbers */
            asm volatile("" ::: "memory", "r10", "r11", "r12", "r13",
                         "xmm8", "xmm9", "xmm10", "xmm11");
            
            clobber_func2();
            
            asm volatile("" ::: "memory", "r14", "r15", 
                         "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Different computations */
            v7 = v8 + v1 * v2;
            v8 = v3 & v4;
            
            vec1 = vec2 - vec3;
            vec2 = vec4 * 2.0f;
        }
        
        /* Common post-call computations */
        l1 = l2 * (i + 1);
        l2 = l1 >> 1;
        
        s1 = s2 + i;
        s2 = s1 * 2;
        
        /* Third external call with mixed clobbers */
        if (i % 3 == 0) {
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1",
                         "xmm8", "xmm9", "ymm0", "ymm1");
            
            clobber_func3();
            
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3",
                         "xmm10", "xmm11", "ymm2", "ymm3");
        }
        
        /* Final aggregation keeping all variables live */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        result += (int)(f1 + f2 + f3 + f4);
        result += (int)(d1 + d2 + d3);
        
        /* Vector reduction */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                       vec2[0] + vec2[1] + vec2[2] + vec2[3] +
                       vec3[0] + vec3[1] + vec3[2] + vec3[3] +
                       vec4[0] + vec4[1] + vec4[2] + vec4[3];
        result += (int)vec_sum;
        
        double dvec_sum = dvec1[0] + dvec1[1] + dvec2[0] + dvec2[1];
        result += (int)dvec_sum;
        
        int ivec_sum = ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3] +
                      ivec2[0] + ivec2[1] + ivec2[2] + ivec2[3];
        result += ivec_sum;
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional use of all variables at the end */
    volatile int final_check = 
        *p1 + *p2 + *fp1 + *fp2 + l1 + l2 + s1 + s2;
    
    return result % 256;
}
