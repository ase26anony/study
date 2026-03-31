#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force calls */
extern void external_func1(void);
extern void external_func2(void);
extern void external_func3(void);

/* Vector types for SSE/AVX pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force register pressure with many live variables */
int main(int argc, char *argv[]) {
    /* Integer variables - all must stay live across calls */
    volatile int vi1 = argc * 1;
    volatile int vi2 = argc * 2;
    volatile int vi3 = argc * 3;
    volatile int vi4 = argc * 4;
    volatile int vi5 = argc * 5;
    volatile int vi6 = argc * 6;
    volatile int vi7 = argc * 7;
    volatile int vi8 = argc * 8;
    
    /* Floating point variables */
    volatile float vf1 = argc * 1.1f;
    volatile float vf2 = argc * 2.2f;
    volatile float vf3 = argc * 3.3f;
    volatile float vf4 = argc * 4.4f;
    
    /* Double precision variables */
    volatile double vd1 = argc * 1.11;
    volatile double vd2 = argc * 2.22;
    volatile double vd3 = argc * 3.33;
    volatile double vd4 = argc * 4.44;
    
    /* Pointer variables */
    volatile int *vp1 = &vi1;
    volatile int *vp2 = &vi2;
    volatile float *vfp1 = &vf1;
    volatile float *vfp2 = &vf2;
    
    /* Vector variables - use all vector registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v2df dvec3 = {5.0, 6.0};
    v2df dvec4 = {7.0, 8.0};
    
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    v4si ivec3 = {9, 10, 11, 12};
    v4si ivec4 = {13, 14, 15, 16};
    
    /* Complex computation to create data dependencies */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    float result_f = 0.0f;
    double result_d = 0.0;
    int result_i = 0;
    
    /* Loop with conditional control flow */
    for (int i = 0; i < iterations; i++) {
        /* Complex pre-call computations keeping all vars live */
        if (i % 2 == 0) {
            /* Integer computations */
            vi1 = vi2 + vi3 * vi4;
            vi5 = vi6 ^ vi7 | vi8;
            vi2 = vi1 - vi5;
            
            /* Floating computations */
            vf1 = vf2 * vf3 + vf4;
            vf3 = vf1 / vf2 - vf4;
            
            /* Vector operations - SSE/AVX pressure */
            vec1 = vec1 + vec2 * vec3;
            vec4 = vec4 - vec1;
            dvec1 = dvec1 * dvec2 + dvec3;
            ivec1 = ivec1 & ivec2 | ivec3;
            
            /* Clobber integer registers before call */
            asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                       "rsi", "rdi", "r8", "r9", "r10");
            
            /* Function call that forces caller-save */
            external_func1();
            
            /* Clobber floating point registers after call */
            asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
        } else {
            /* Different computation pattern */
            vd1 = vd2 * vd3 - vd4;
            vd2 = vd1 + vd3 / vd4;
            
            vec2 = vec3 * vec4 - vec1;
            dvec2 = dvec3 + dvec4 * dvec1;
            ivec2 = ivec3 ^ ivec4 & ivec1;
            
            /* Pointer arithmetic */
            *vp1 = *vp2 + vi3;
            *vfp1 = *vfp2 * vf3;
            
            /* Clobber mixed registers */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1",
                                       "xmm8", "xmm9", "xmm10", "xmm11");
            
            /* Another external call */
            external_func2();
            
            /* Clobber different registers */
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm4", "xmm5",
                                       "xmm12", "xmm13", "xmm14", "xmm15");
        }
        
        /* Post-call computations using all variables */
        vi3 = vi4 + vi5 * vi6;
        vi7 = vi8 ^ vi1 & vi2;
        
        vf2 = vf3 * vf4 - vf1;
        vf4 = vf1 + vf2 / vf3;
        
        vd3 = vd4 * vd1 + vd2;
        vd4 = vd2 - vd3 / vd1;
        
        /* More vector operations */
        vec3 = vec4 + vec1 * vec2;
        dvec3 = dvec4 - dvec1 * dvec2;
        ivec3 = ivec4 | ivec1 & ivec2;
        
        /* Pointer chasing */
        if (vp1 != vp2) {
            vi1 = *vp1 + *vp2;
        }
        
        /* Accumulate results */
        result_i += vi1 + vi2 + vi3 + vi4 + vi5 + vi6 + vi7 + vi8;
        result_f += vf1 + vf2 + vf3 + vf4;
        result_d += vd1 + vd2 + vd3 + vd4;
        
        /* Vector reductions */
        for (int j = 0; j < 4; j++) {
            result_f += vec1[j] + vec2[j] + vec3[j] + vec4[j];
            result_i += ivec1[j] + ivec2[j] + ivec3[j] + ivec4[j];
        }
        for (int j = 0; j < 2; j++) {
            result_d += dvec1[j] + dvec2[j] + dvec3[j] + dvec4[j];
        }
        
        /* Conditional call with different clobbering */
        if (i % 3 == 0) {
            asm volatile("" ::: "memory", "r11", "r12", "r13", "r14", "r15",
                                       "xmm0", "xmm1", "xmm2", "xmm3");
            
            external_func3();
            
            asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
        }
    }
    
    /* Final aggregation to prevent optimization */
    double final_result = result_i + result_f + result_d;
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
