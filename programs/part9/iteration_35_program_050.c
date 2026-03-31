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
    int *p1 = &vi1, *p2 = &vi2, *p3 = &vi3;
    float *fp1 = &vf1, *fp2 = &vf2;
    double *dp1 = &vd1, *dp2 = &vd2;
    
    /* Vector variables - separate register bank pressure */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Accumulator for final result */
    double total = 0.0;
    
    /* Loop to create basic block structure */
    for (int loop = 0; loop < iterations; loop++) {
        /* COMPLEX COMPUTATIONS BEFORE CALL - Keep all variables live */
        
        /* Integer computations */
        vi1 = vi2 + vi3 * loop;
        vi2 = vi3 ^ vi4;
        vi3 = vi4 | vi5;
        vi4 = vi5 & vi1;
        vi5 = vi1 << 2;
        
        i6 = i7 * i8 - i9;
        i7 = i8 / (i9 + 1);
        i8 = i9 ^ i10;
        i9 = i10 + loop;
        i10 = i6 * loop;
        
        /* Float computations */
        vf1 = vf2 * vf3 + (float)loop;
        vf2 = vf3 - vf4 / 2.0f;
        vf3 = vf4 * vf5;
        vf4 = vf5 + vf1;
        vf5 = vf1 - 1.0f;
        
        f6 = f7 * f8;
        f7 = f8 + f9;
        f8 = f9 * f10;
        f9 = f10 / (f6 + 1.0f);
        f10 = f6 - f7;
        
        /* Double computations */
        vd1 = vd2 + vd3 * loop;
        vd2 = vd3 - vd4;
        vd3 = vd4 * vd5;
        vd4 = vd5 / (vd1 + 1.0);
        vd5 = vd1 * 2.0;
        
        d6 = d7 + d8;
        d7 = d8 * d9;
        d8 = d9 - d10;
        d9 = d10 / (d6 + 1.0);
        d10 = d6 * loop;
        
        /* Pointer arithmetic */
        p1 = p2 + loop;
        p2 = p3 - 1;
        p3 = &i6 + loop;
        
        fp1 = fp2 + loop;
        fp2 = &f6;
        
        dp1 = dp2;
        dp2 = &d6 + loop;
        
        /* Vector operations - separate register bank */
        vec1 = vec1 + vec2 * (float)loop;
        vec2 = vec2 - vec3;
        vec3 = vec3 * vec1;
        
        dvec1 = dvec1 + dvec2;
        dvec2 = dvec2 * 2.0;
        
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec2 << 1;
        
        /* ASM to clobber integer registers */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* ASM to clobber floating point/vector registers */
        asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations to keep variables live across calls */
        vi1 = vi2 * vi3;
        vi2 = vi4 + vi5;
        
        vf1 = vf2 - vf3;
        vf2 = vf4 * vf5;
        
        vd1 = vd2 / vd3;
        vd2 = vd4 + vd5;
        
        vec1 = vec2 + vec3;
        dvec1 = dvec1 - dvec2;
        
        /* Second ASM with different clobbers */
        asm volatile("" ::: "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Another external call */
        clobber_func2();
        
        /* ASM clobbering mixed registers */
        asm volatile("" ::: "memory", "rax", "xmm0", "xmm1", "rbx", "xmm2");
        
        /* More computations */
        i6 = i7 ^ i8;
        i7 = i9 + i10;
        
        f6 = f7 * f8;
        f7 = f9 - f10;
        
        d6 = d7 / d8;
        d7 = d9 + d10;
        
        /* Third external call */
        clobber_func3();
        
        /* Final ASM clobber */
        asm volatile("" ::: "memory", "xmm8", "xmm9", "xmm10", "xmm11", 
                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Aggregate results to prevent elimination */
        total += vi1 + vi2 + vi3 + vi4 + vi5;
        total += i6 + i7 + i8 + i9 + i10;
        total += vf1 + vf2 + vf3 + vf4 + vf5;
        total += f6 + f7 + f8 + f9 + f10;
        total += vd1 + vd2 + vd3 + vd4 + vd5;
        total += d6 + d7 + d8 + d9 + d10;
        
        /* Use vector results */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        total += vec_sum;
        total += dvec1[0] + dvec1[1];
        total += ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        
        /* Use pointers */
        total += *p1 + *p2;
        total += *fp1 + *fp2;
        total += *dp1 + *dp2;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    return (int)total % 256;
}
