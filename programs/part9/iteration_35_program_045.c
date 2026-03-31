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
    
    /* Declare MANY local variables of mixed types to create register pressure */
    
    /* Integer variables */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    volatile int vi6 = 6, vi7 = 7, vi8 = 8, vi9 = 9, vi10 = 10;
    volatile int vi11 = 11, vi12 = 12, vi13 = 13, vi14 = 14, vi15 = 15;
    
    /* Floating point variables */
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f, vf5 = 5.5f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33, vd4 = 4.44, vd5 = 5.55;
    
    /* Pointer variables */
    volatile int *vp1 = &vi1, *vp2 = &vi2, *vp3 = &vi3;
    volatile float *vfp1 = &vf1, *vfp2 = &vf2;
    volatile double *vdp1 = &vd1, *vdp2 = &vd2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v2df dvec3 = {5.0, 6.0};
    
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    v4si ivec3 = {9, 10, 11, 12};
    
    /* Accumulator for final result */
    volatile double total = 0.0;
    
    /* Loop to create control flow complexity */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations using all variables to keep them live */
        vi1 = vi2 + vi3 * i;
        vi4 = vi5 ^ vi6;
        vi7 = vi8 | vi9;
        vi10 = vi11 & vi12;
        vi13 = vi14 - vi15;
        
        vf1 = vf2 * vf3 + (float)i;
        vf4 = vf5 / (vf1 + 1.0f);
        
        vd1 = vd2 + vd3 * (double)i;
        vd4 = vd5 - vd1;
        
        /* Pointer arithmetic to keep pointers live */
        *vp1 = *vp2 + *vp3;
        *vfp1 = *vfp2 * 2.0f;
        *vdp1 = *vdp2 / 2.0;
        
        /* Vector operations */
        vec1 = vec2 + vec3 * (float)(i + 1);
        vec4 = vec1 - vec2;
        
        dvec1 = dvec2 + dvec3 * (double)i;
        
        ivec1 = ivec2 & ivec3;
        ivec2 = ivec1 | ivec3;
        
        /* First asm volatile to clobber integer registers */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                     "rsi", "rdi", "r8", "r9", "r10");
        
        /* First external call - forces caller-save */
        clobber_func1();
        
        /* Second asm volatile to clobber floating point/vector registers */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                                     "xmm4", "xmm5", "xmm6", "xmm7",
                                     "ymm0", "ymm1", "ymm2", "ymm3");
        
        /* More computations between calls */
        vi2 = vi3 + vi4 * (i + 2);
        vi5 = vi6 ^ vi7;
        vi8 = vi9 | vi10;
        
        vf2 = vf3 * vf4 - (float)i;
        vf5 = vf1 / (vf2 + 1.0f);
        
        vd2 = vd3 + vd4 * (double)(i + 1);
        vd5 = vd1 - vd2;
        
        vec2 = vec3 + vec4 * (float)(i + 3);
        dvec2 = dvec3 + dvec1 * 0.5;
        ivec3 = ivec1 ^ ivec2;
        
        /* Third asm volatile with mixed clobbers */
        asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1",
                                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* Second external call */
        clobber_func2();
        
        /* Post-call computations */
        vi3 = vi4 + vi5 * (i + 3);
        vi6 = vi7 ^ vi8;
        vi9 = vi10 | vi11;
        
        vf3 = vf4 * vf5 + (float)(i * 2);
        vd3 = vd4 + vd5 * (double)(i + 2);
        
        vec3 = vec4 + vec1 * 0.25f;
        dvec3 = dvec1 + dvec2 * 0.75;
        
        /* Fourth asm volatile */
        asm volatile ("" ::: "memory", "rcx", "rdx", "xmm4", "xmm5",
                                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Third external call */
        clobber_func3();
        
        /* Final computations in the loop */
        vi12 = vi13 + vi14 * (i + 4);
        vi15 = vi1 ^ vi2;
        
        vf4 = vf5 * 3.14159f;
        vd4 = vd5 * 2.71828;
        
        vec4 = vec1 + vec2 + vec3;
        dvec1 = dvec2 + dvec3;
        ivec1 = ivec2 + ivec3;
        
        /* Accumulate to total to keep everything live */
        total += vi1 + vi2 + vi3 + vi4 + vi5 + vi6 + vi7 + vi8 + vi9 + vi10;
        total += vi11 + vi12 + vi13 + vi14 + vi15;
        total += vf1 + vf2 + vf3 + vf4 + vf5;
        total += vd1 + vd2 + vd3 + vd4 + vd5;
        
        /* Extract and add vector elements */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                       vec2[0] + vec2[1] + vec2[2] + vec2[3] +
                       vec3[0] + vec3[1] + vec3[2] + vec3[3] +
                       vec4[0] + vec4[1] + vec4[2] + vec4[3];
        
        total += vec_sum;
        
        double dvec_sum = dvec1[0] + dvec1[1] + dvec2[0] + dvec2[1] +
                         dvec3[0] + dvec3[1];
        total += dvec_sum;
        
        int ivec_sum = ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3] +
                      ivec2[0] + ivec2[1] + ivec2[2] + ivec2[3] +
                      ivec3[0] + ivec3[1] + ivec3[2] + ivec3[3];
        total += ivec_sum;
    }
    
    /* Print result to prevent optimization */
    printf("Total: %f\n", total);
    
    return (int)total % 256;
}
