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

/* Prevent optimization of critical variables */
static volatile int volatile_sink;

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* Declare MANY local variables of mixed types to maximize register pressure */
    
    /* Integer variables - 8 variables */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;
    volatile long vl1 = 1000, vl2 = 2000, vl3 = 3000, vl4 = 4000;
    
    /* Floating point variables - 8 variables */
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f;
    volatile double vd1 = 10.1, vd2 = 20.2, vd3 = 30.3, vd4 = 40.4;
    
    /* Pointer variables - 4 variables */
    volatile int *vp1 = &vi1, *vp2 = &vi2;
    volatile float *vfp1 = &vf1, *vfp2 = &vf2;
    
    /* Vector variables - 4 variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    
    /* Additional scalar variables for more pressure */
    volatile int vi5 = 5, vi6 = 6, vi7 = 7, vi8 = 8;
    volatile float vf5 = 5.5f, vf6 = 6.6f, vf7 = 7.7f, vf8 = 8.8f;
    volatile double vd5 = 50.5, vd6 = 60.6, vd7 = 70.7, vd8 = 80.8;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Loop to create control flow complexity */
    for (int i = 0; i < iterations; i++) {
        /* Complex computation before call to keep variables live */
        vi1 = vi2 * vi3 + vi4;
        vi2 = vi1 ^ vi3 | vi4;
        vi3 = vi2 + vi4 * vi1;
        vi4 = vi3 - vi2 / (vi1 + 1);
        
        vf1 = vf2 * vf3 + vf4;
        vf2 = vf1 / vf3 - vf4;
        vf3 = vf2 + vf4 * vf1;
        vf4 = vf3 - vf2 / (vf1 + 1.0f);
        
        vd1 = vd2 * vd3 + vd4;
        vd2 = vd1 / vd3 - vd4;
        vd3 = vd2 + vd4 * vd1;
        vd4 = vd3 - vd2 / (vd1 + 1.0);
        
        /* Vector operations */
        vec1 = vec1 + vec2 * 2.0f;
        vec2 = vec2 - vec1 / 3.0f;
        dvec1 = dvec1 * dvec2 + 1.5;
        dvec2 = dvec2 - dvec1 * 0.5;
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec2 - ivec1;
        
        /* Pointer arithmetic */
        vp1 = (int*)((char*)vp1 + 1);
        vp2 = (int*)((char*)vp2 - 1);
        vfp1 = (float*)((char*)vfp1 + 1);
        vfp2 = (float*)((char*)vfp2 - 1);
        
        /* ASM to clobber specific registers BEFORE call */
        asm volatile (
            "/* Clobber integer registers */\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* External function call - forces caller-save */
        clobber_func1();
        
        /* ASM to clobber DIFFERENT registers AFTER call */
        asm volatile (
            "/* Clobber more registers */\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx",
              "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7",
              "memory"
        );
        
        /* More computations to keep variables live across multiple calls */
        vi5 = vi6 * vi7 + vi8;
        vi6 = vi5 ^ vi7 | vi8;
        vi7 = vi6 + vi8 * vi5;
        vi8 = vi7 - vi6 / (vi5 + 1);
        
        vf5 = vf6 * vf7 + vf8;
        vf6 = vf5 / vf7 - vf8;
        vf7 = vf6 + vf8 * vf5;
        vf8 = vf7 - vf6 / (vf5 + 1.0f);
        
        vd5 = vd6 * vd7 + vd8;
        vd6 = vd5 / vd7 - vd8;
        vd7 = vd6 + vd8 * vd5;
        vd8 = vd7 - vd6 / (vd5 + 1.0);
        
        /* Second ASM clobber with different registers */
        asm volatile (
            "/* Clobber for second call */\n\t"
            : /* no outputs */
            : /* no inputs */
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
        
        /* Second external call */
        clobber_func2();
        
        /* Conditional control flow to create basic block boundaries */
        if (i % 2 == 0) {
            /* Third ASM with yet another register set */
            asm volatile (
                "/* Conditional clobber */\n\t"
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            clobber_func3();
            
            /* More vector ops in conditional path */
            vec1 = vec1 * 1.1f;
            vec2 = vec2 / 1.1f;
            dvec1 = dvec1 + 0.1;
            dvec2 = dvec2 - 0.1;
        } else {
            /* Alternative path with different computations */
            ivec1 = ivec1 * 2;
            ivec2 = ivec2 / 2;
        }
        
        /* Aggregate results to prevent elimination */
        total_result += vi1 + vi2 + vi3 + vi4 + vi5 + vi6 + vi7 + vi8;
        total_result += vf1 + vf2 + vf3 + vf4 + vf5 + vf6 + vf7 + vf8;
        total_result += vd1 + vd2 + vd3 + vd4 + vd5 + vd6 + vd7 + vd8;
        
        /* Extract and add vector elements */
        float vec1_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        float vec2_sum = vec2[0] + vec2[1] + vec2[2] + vec2[3];
        total_result += vec1_sum + vec2_sum;
        
        double dvec1_sum = dvec1[0] + dvec1[1];
        double dvec2_sum = dvec2[0] + dvec2[1];
        total_result += dvec1_sum + dvec2_sum;
        
        int ivec1_sum = ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        int ivec2_sum = ivec2[0] + ivec2[1] + ivec2[2] + ivec2[3];
        total_result += ivec1_sum + ivec2_sum;
        
        volatile_sink = i; /* Prevent loop optimization */
    }
    
    /* Final computation and output to prevent dead code elimination */
    double final_result = total_result / (iterations + 1);
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
