#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
    /* Non-volatile variables for computations */
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400, i5 = 500;
    float f1 = 100.1f, f2 = 200.2f, f3 = 300.3f, f4 = 400.4f, f5 = 500.5f;
    double d1 = 100.11, d2 = 200.22, d3 = 300.33, d4 = 400.44, d5 = 500.55;
    
    /* Pointer variables */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    float *fp1 = &f1, *fp2 = &f2, *fp3 = &f3;
    double *dp1 = &d1, *dp2 = &d2, *dp3 = &d3;
    
    /* Vector variables - separate register bank pressure */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v2df dvec3 = {5.0, 6.0};
    
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    v4si ivec3 = {90, 100, 110, 120};
    
    /* Mixed computations to create data dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pre-call computations - keep many variables live */
        i1 = vi1 + i2 * i3 - i4 / (i5 + 1);
        f1 = vf1 + f2 * f3 - f4 / (f5 + 1.0f);
        d1 = vd1 + d2 * d3 - d4 / (d5 + 1.0);
        
        /* Pointer arithmetic */
        *p1 = *p2 + *p3;
        *fp1 = *fp2 + *fp3;
        *dp1 = *dp2 + *dp3;
        
        /* Vector operations */
        vec1 = vec1 + vec2 * vec3 - vec4;
        dvec1 = dvec1 + dvec2 * dvec3;
        ivec1 = ivec1 + ivec2 * ivec3;
        
        /* CLOBBER INTEGER REGISTERS before call */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                           "rsi", "rdi", "r8", "r9", "r10");
        
        /* Function call - forces caller-save */
        clobber_func1();
        
        /* CLOBBER VECTOR REGISTERS after call */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                                           "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations with live variables */
        i2 = i1 * 2 + vi2;
        f2 = f1 * 2.0f + vf2;
        d2 = d1 * 2.0 + vd2;
        
        vec2 = vec1 * 2.0f + vec2;
        dvec2 = dvec1 * 2.0 + dvec2;
        ivec2 = ivec1 * 2 + ivec2;
        
        /* CLOBBER DIFFERENT REGISTERS */
        asm volatile ("" ::: "memory", "r11", "r12", "r13", "r14", "r15",
                                           "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* Another function call */
        clobber_func2();
        
        /* Conditional to create basic block boundaries */
        if (iter % 2 == 0) {
            /* CLOBBER in conditional path */
            asm volatile ("" ::: "memory", "xmm12", "xmm13", "xmm14", "xmm15");
            
            i3 = i2 + i1 * vi3;
            f3 = f2 + f1 * vf3;
            d3 = d2 + d1 * vd3;
            
            vec3 = vec2 + vec1 * 3.0f;
            
            clobber_func3();
        } else {
            /* Alternative path with different clobbers */
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            
            i3 = i2 - i1 / (vi3 + 1);
            f3 = f2 - f1 / (vf3 + 1.0f);
            d3 = d2 - d1 / (vd3 + 1.0);
            
            vec3 = vec2 - vec1 / 3.0f;
        }
        
        /* Post-call computations ensuring all variables stay live */
        i4 = i3 + i2 + i1 + vi4;
        f4 = f3 + f2 + f1 + vf4;
        d4 = d3 + d2 + d1 + vd4;
        
        vec4 = vec3 + vec2 + vec1;
        dvec3 = dvec2 + dvec1;
        ivec3 = ivec2 + ivec1;
        
        /* Access volatile variables to force memory operations */
        vi1 = vi1 + 1;
        vf1 = vf1 + 1.0f;
        vd1 = vd1 + 1.0;
    }
    
    /* Aggregate results to prevent dead code elimination */
    double total = 0.0;
    total += i1 + i2 + i3 + i4 + i5;
    total += f1 + f2 + f3 + f4 + f5;
    total += d1 + d2 + d3 + d4 + d5;
    total += vi1 + vi2 + vi3 + vi4 + vi5;
    total += vf1 + vf2 + vf3 + vf4 + vf5;
    total += vd1 + vd2 + vd3 + vd4 + vd5;
    
    /* Vector reduction */
    for (int i = 0; i < 4; i++) {
        total += vec1[i] + vec2[i] + vec3[i] + vec4[i];
        total += ivec1[i] + ivec2[i] + ivec3[i];
    }
    for (int i = 0; i < 2; i++) {
        total += dvec1[i] + dvec2[i] + dvec3[i];
    }
    
    printf("Result: %f\n", total);
    return (int)total % 256;
}
