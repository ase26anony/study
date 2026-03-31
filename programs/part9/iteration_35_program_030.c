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
    
    /* Pointer variables */
    int *p1 = &vi1, *p2 = &vi2, *p3 = &vi3;
    float *fp1 = &vf1, *fp2 = &vf2;
    double *dp1 = &vd1, *dp2 = &vd2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    
    /* Additional integer variables */
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    
    /* Additional float variables */
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Loop to create complex control flow */
    for (int loop = 0; loop < iterations; loop++) {
        /* COMPUTATION BLOCK 1 - Create data dependencies */
        vi1 = vi2 + vi3 * loop;
        vi2 = vi4 ^ vi5;
        vi3 = vi1 | vi2;
        
        vf1 = vf2 * vf3 + (float)loop;
        vf2 = vf4 / vf5;
        vf3 = vf1 - vf2;
        
        vd1 = vd2 + vd3 * loop;
        vd2 = vd4 - vd5;
        vd3 = vd1 * vd2;
        
        /* Pointer arithmetic */
        *p1 = *p2 + *p3;
        *fp1 = *fp2 * 2.0f;
        *dp1 = *dp2 / 2.0;
        
        /* Vector operations */
        vec1 = vec1 + vec2 * (float)(loop + 1);
        vec2 = vec1 - vec2;
        dvec1 = dvec1 * dvec2 + (double)loop;
        dvec2 = dvec1 / dvec2;
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec1 - ivec2;
        
        /* More scalar operations */
        i6 = i7 * i8 + loop;
        i7 = i9 ^ i10;
        i8 = i11 | i12;
        i9 = i13 & i14;
        i10 = i15 << 2;
        
        f6 = f7 * f8 + (float)loop;
        f7 = f9 / f10;
        f8 = f6 - f7;
        
        /* CLOBBER REGISTERS BEFORE CALL */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* EXTERNAL CALL 1 - Forces caller-save */
        clobber_func1();
        
        /* CLOBBER DIFFERENT REGISTERS */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* COMPUTATION BLOCK 2 - Keep variables live */
        vi4 = vi1 + vi2 * vi3;
        vi5 = vi4 ^ vi3;
        
        vf4 = vf1 * vf2 + vf3;
        vf5 = vf4 / vf1;
        
        vd4 = vd1 + vd2 * vd3;
        vd5 = vd4 - vd3;
        
        /* More vector operations */
        vec1 = vec2 * 3.14f;
        dvec1 = dvec2 * 2.71828;
        ivec1 = ivec2 << 1;
        
        /* CLOBBER AVX REGISTERS IF AVAILABLE */
        asm volatile("" ::: "memory", "ymm0", "ymm1", "ymm2", "ymm3");
        
        /* EXTERNAL CALL 2 */
        clobber_func2();
        
        /* CLOBBER MORE REGISTERS */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13", "r14", "r15",
                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* COMPUTATION BLOCK 3 */
        i11 = i6 + i7 * i8;
        i12 = i9 ^ i10;
        i13 = i11 | i12;
        i14 = i13 & i15;
        i15 = i14 << 1;
        
        f9 = f6 * f7 + f8;
        f10 = f9 / f6;
        
        /* Complex expression mixing all types */
        total_result += (double)vi1 + (double)vf1 + vd1 + 
                       (double)(i6 + i7 + i8 + i9 + i10) +
                       (double)(f6 + f7 + f8 + f9 + f10);
        
        /* Mix vector elements into result */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        double dvec_sum = dvec1[0] + dvec1[1];
        int ivec_sum = ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        
        total_result += (double)vec_sum + dvec_sum + (double)ivec_sum;
        
        /* Conditional within loop for more complex CFG */
        if (loop % 2 == 0) {
            /* CLOBBER IN BRANCH */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
            clobber_func3();
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm3", "xmm4", "xmm5");
            
            /* More computations in branch */
            vi1 = vi2 * 2;
            vf1 = vf2 * 3.0f;
            vec1 = vec1 * 2.0f;
        } else {
            /* Different computations in else branch */
            vi1 = vi3 / 2;
            vf1 = vf3 / 2.0f;
            vec1 = vec1 / 2.0f;
        }
        
        /* Final computation to keep everything live */
        total_result += (double)(*p1 + *p2 + *p3) +
                       (double)(*fp1 + *fp2) +
                       (*dp1 + *dp2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", total_result);
    
    return (int)total_result % 256;
}
