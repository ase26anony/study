#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    
    /* VOLATILE variables to prevent optimization */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f, vf5 = 5.5f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33, vd4 = 4.44, vd5 = 5.55;
    
    /* Pointer variables */
    int *p1 = &vi1, *p2 = &vi2, *p3 = &vi3;
    float *fp1 = (float*)&vf1, *fp2 = (float*)&vf2;
    double *dp1 = (double*)&vd1, *dp2 = (double*)&vd2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Additional integer variables */
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    
    /* Additional float variables */
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Main loop with complex control flow */
    for (int loop = 0; loop < iterations; loop++) {
        /* Pre-call computations creating data dependencies */
        vi1 = vi2 + vi3 * loop;
        vi2 = vi4 ^ vi5;
        vi3 = vi1 | vi2;
        
        vf1 = vf2 * vf3 + (float)loop;
        vf2 = vf4 / vf5;
        vf3 = vf1 - vf2;
        
        vd1 = vd2 + vd3 * loop;
        vd2 = vd4 - vd5;
        vd3 = vd1 * vd2;
        
        /* Vector operations */
        vec1 = vec2 + vec3 * (float)(loop + 1);
        vec2 = vec1 - vec3;
        vec3 = vec1 * vec2;
        
        dvec1 = dvec2 * (double)(loop + 2);
        dvec2 = dvec1 + dvec2;
        
        ivec1 = ivec2 + loop;
        ivec2 = ivec1 ^ ivec2;
        
        /* Pointer arithmetic */
        p1 = p2 + loop;
        p2 = p3 - loop;
        fp1 = fp2 + loop;
        dp1 = dp2 + loop;
        
        /* Scalar computations */
        i6 = i7 * i8 + loop;
        i7 = i9 ^ i10;
        i8 = i11 & i12;
        i9 = i13 | i14;
        i10 = i15 << loop;
        
        f6 = f7 * f8 + (float)loop;
        f7 = f9 / f10;
        f8 = f6 - f7;
        
        /* ASM to clobber specific registers BEFORE call */
        asm volatile ("" 
            : /* no outputs */
            : /* no inputs */
            : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* External call forcing caller-save */
        clobber_func1();
        
        /* ASM to clobber DIFFERENT registers AFTER call */
        asm volatile ("" 
            : /* no outputs */
            : /* no inputs */
            : "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12"
        );
        
        /* Conditional branch inside loop */
        if (loop % 2 == 0) {
            /* More computations keeping variables live */
            vi4 = vi5 + vi1;
            vi5 = vi2 * vi3;
            
            vf4 = vf5 + vf1;
            vf5 = vf2 * vf3;
            
            vd4 = vd5 + vd1;
            vd5 = vd2 * vd3;
            
            /* Second external call with different clobbers */
            asm volatile ("" 
                : /* no outputs */
                : /* no inputs */
                : "memory", "mm0", "mm1", "mm2", "mm3",
                  "xmm13", "xmm14", "xmm15"
            );
            
            clobber_func2();
            
            asm volatile ("" 
                : /* no outputs */
                : /* no inputs */
                : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)"
            );
        } else {
            /* Alternative path with different computations */
            vec1 = vec2 * vec3;
            vec2 = vec1 + vec3;
            
            dvec1 = dvec2 * 2.0;
            dvec2 = dvec1 / 3.0;
            
            ivec1 = ivec2 << 1;
            ivec2 = ivec1 >> 1;
            
            /* Third external call */
            asm volatile ("" 
                : /* no outputs */
                : /* no inputs */
                : "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2"
            );
            
            clobber_func3();
        }
        
        /* Post-call computations using all variables */
        int sum_int = vi1 + vi2 + vi3 + vi4 + vi5 + i6 + i7 + i8 + i9 + i10 +
                     i11 + i12 + i13 + i14 + i15;
        
        float sum_float = vf1 + vf2 + vf3 + vf4 + vf5 + f6 + f7 + f8 + f9 + f10;
        
        double sum_double = vd1 + vd2 + vd3 + vd4 + vd5;
        
        /* Vector reductions */
        v4sf temp_vec = vec1 + vec2 + vec3;
        float vec_sum = temp_vec[0] + temp_vec[1] + temp_vec[2] + temp_vec[3];
        
        v2df temp_dvec = dvec1 + dvec2;
        double dvec_sum = temp_dvec[0] + temp_dvec[1];
        
        v4si temp_ivec = ivec1 + ivec2;
        int ivec_sum = temp_ivec[0] + temp_ivec[1] + temp_ivec[2] + temp_ivec[3];
        
        /* Pointer dereferencing */
        sum_int += *p1 + *p2 + *p3;
        sum_float += *fp1 + *fp2;
        sum_double += *dp1 + *dp2;
        
        /* Accumulate results */
        total_result += (double)sum_int + (double)sum_float + sum_double + 
                       (double)vec_sum + dvec_sum + (double)ivec_sum;
        
        /* Force register pressure across loop iterations */
        vi1 = sum_int % 100;
        vf1 = (float)(sum_float / 100.0f);
        vd1 = sum_double / 100.0;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %f\n", total_result);
    
    return (int)total_result % 256;
}
