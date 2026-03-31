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

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE VARIABLES - Prevent optimization */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f, vf5 = 5.5f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33, vd4 = 4.44, vd5 = 5.55;
    
    /* Non-volatile but heavily used variables */
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d6 = 6.66, d7 = 7.77, d8 = 8.88, d9 = 9.99, d10 = 10.1010;
    
    /* Pointer variables */
    int *p1 = &vi1, *p2 = &vi2, *p3 = &i6;
    float *fp1 = &vf1, *fp2 = &vf2, *fp3 = &f6;
    double *dp1 = &vd1, *dp2 = &vd2, *dp3 = &d6;
    
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
    
    /* Accumulator for final result */
    double total = 0.0;
    
    /* Main loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* COMPUTATION PHASE 1 - Create data dependencies */
        vi1 = vi2 + vi3 * iter;
        vi2 = vi3 ^ vi4 | vi5;
        vi3 = vi4 << 2;
        vi4 = vi5 >> 1;
        vi5 = vi1 & vi2;
        
        vf1 = vf2 * vf3 + (float)iter;
        vf2 = vf3 / vf4 - vf5;
        vf3 = vf4 + vf5 * 2.0f;
        vf4 = vf5 - vf1 / 3.0f;
        vf5 = vf1 + vf2;
        
        vd1 = vd2 * vd3 + (double)iter;
        vd2 = vd3 / vd4 - vd5;
        vd3 = vd4 + vd5 * 2.0;
        vd4 = vd5 - vd1 / 3.0;
        vd5 = vd1 + vd2;
        
        /* Scalar computations */
        i6 = i7 * i8 + iter;
        i7 = i8 ^ i9 | i10;
        i8 = i9 << 3;
        i9 = i10 >> 2;
        i10 = i6 & i7;
        
        f6 = f7 * f8 + (float)iter;
        f7 = f8 / f9 - f10;
        f8 = f9 + f10 * 2.5f;
        f9 = f10 - f6 / 4.0f;
        f10 = f6 + f7;
        
        d6 = d7 * d8 + (double)iter;
        d7 = d8 / d9 - d10;
        d8 = d9 + d10 * 2.5;
        d9 = d10 - d6 / 4.0;
        d10 = d6 + d7;
        
        /* Pointer arithmetic */
        *p1 = *p2 + *p3;
        *p2 = *p3 ^ iter;
        *p3 = *p1 & 0xFF;
        
        *fp1 = *fp2 * *fp3;
        *fp2 = *fp3 / (float)(iter + 1);
        *fp3 = *fp1 + 1.0f;
        
        *dp1 = *dp2 * *dp3;
        *dp2 = *dp3 / (double)(iter + 1);
        *dp3 = *dp1 + 1.0;
        
        /* Vector operations */
        vec1 = vec2 + vec3 * (float)(iter + 1);
        vec2 = vec3 - vec4 / (float)(iter + 2);
        vec3 = vec4 * vec1;
        vec4 = vec1 + vec2 - vec3;
        
        dvec1 = dvec2 + dvec3 * (double)(iter + 1);
        dvec2 = dvec3 - dvec4 / (double)(iter + 2);
        dvec3 = dvec4 * dvec1;
        dvec4 = dvec1 + dvec2 - dvec3;
        
        ivec1 = ivec2 + ivec3 * (iter + 1);
        ivec2 = ivec3 - ivec4 / (iter + 2);
        ivec3 = ivec4 * ivec1;
        ivec4 = ivec1 + ivec2 - ivec3;
        
        /* ASM CLOBBER 1 - Force save of specific registers */
        asm volatile ("" ::: 
            "memory", 
            "rax", "rbx", "rcx", "rdx", 
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* EXTERNAL CALL 1 - Forces caller-save */
        clobber_func1();
        
        /* ASM CLOBBER 2 - Different register set */
        asm volatile ("" ::: 
            "memory",
            "rsi", "rdi", "r8", "r9",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* COMPUTATION PHASE 2 - Keep variables live across calls */
        vi1 = vi5 + vi2 * 2;
        vi2 = vi3 | vi4;
        vi3 = vi1 ^ vi5;
        
        vf1 = vf5 * 3.0f + vf2;
        vf2 = vf3 / 2.0f - vf4;
        vf3 = vf1 + vf2 * vf5;
        
        vd1 = vd5 * 3.0 + vd2;
        vd2 = vd3 / 2.0 - vd4;
        vd3 = vd1 + vd2 * vd5;
        
        /* More vector ops */
        vec1 = vec4 * 2.0f + vec2;
        vec2 = vec3 / 3.0f - vec1;
        vec3 = vec2 + vec4;
        
        dvec1 = dvec4 * 2.0 + dvec2;
        dvec2 = dvec3 / 3.0 - dvec1;
        dvec3 = dvec2 + dvec4;
        
        ivec1 = ivec4 * 2 + ivec2;
        ivec2 = ivec3 / 3 - ivec1;
        ivec3 = ivec2 + ivec4;
        
        /* ASM CLOBBER 3 - Mix integer and vector */
        asm volatile ("" :::
            "memory",
            "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm4", "xmm8", "xmm12",
            "ymm0", "ymm4", "ymm8", "ymm12");
        
        /* EXTERNAL CALL 2 */
        clobber_func2();
        
        /* ASM CLOBBER 4 */
        asm volatile ("" :::
            "memory",
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "xmm1", "xmm5", "xmm9", "xmm13",
            "ymm1", "ymm5", "ymm9", "ymm13");
        
        /* COMPUTATION PHASE 3 */
        i6 = i10 + i7 * 3;
        i7 = i8 | i9;
        i8 = i6 ^ i10;
        
        f6 = f10 * 3.0f + f7;
        f7 = f8 / 2.0f - f9;
        f8 = f6 + f7 * f10;
        
        d6 = d10 * 3.0 + d7;
        d7 = d8 / 2.0 - d9;
        d8 = d6 + d7 * d10;
        
        /* Conditional to create more complex CFG */
        if (iter % 2 == 0) {
            /* ASM with different clobbers in branch */
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            clobber_func3();
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
            
            vec1 = vec1 * 1.5f;
            dvec1 = dvec1 * 1.5;
            ivec1 = ivec1 * 2;
        } else {
            /* Different computation in else branch */
            vec1 = vec1 / 1.5f;
            dvec1 = dvec1 / 1.5;
            ivec1 = ivec1 / 2;
        }
        
        /* Accumulate results - ensures all variables contribute */
        total += (double)vi1 + vf1 + vd1 + i6 + f6 + d6;
        total += vec1[0] + dvec1[0] + (double)ivec1[0];
        total += *p1 + *fp1 + *dp1;
    }
    
    /* Final aggregation and output */
    double final_result = total;
    final_result += vec2[1] + dvec2[1] + (double)ivec2[1];
    final_result += vec3[2] + dvec3[2] + (double)ivec3[2];
    final_result += vec4[3] + dvec4[3] + (double)ivec4[3];
    
    printf("Result: %f\n", final_result);
    return (int)final_result % 256;
}
