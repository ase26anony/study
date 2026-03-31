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
    
    /* Vector variables - use all vector registers */
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
    
    /* Aggregation variable */
    double total = 0.0;
    
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
        
        i6 = i7 + i8 * loop;
        i7 = i9 ^ i10;
        i8 = i6 | i7;
        
        f6 = f7 * f8 + (float)loop;
        f7 = f9 / f10;
        f8 = f6 - f7;
        
        d6 = d7 + d8 * loop;
        d7 = d9 - d10;
        d8 = d6 * d7;
        
        /* Vector computations */
        vec1 = vec2 + vec3 * (float)loop;
        vec2 = vec4 - vec1;
        vec3 = vec1 * vec2;
        
        dvec1 = dvec2 + dvec3 * (double)loop;
        dvec2 = dvec1 - dvec3;
        
        ivec1 = ivec2 + ivec3 * loop;
        ivec2 = ivec1 ^ ivec3;
        
        /* Pointer arithmetic */
        p1 = p2 + loop;
        p2 = p3 - loop;
        fp1 = fp2 + loop;
        dp1 = dp2 + loop;
        
        /* ASM to clobber integer registers */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11");
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* ASM to clobber floating point/vector registers */
        asm volatile ("" ::: "memory", 
                      "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7",
                      "xmm8", "xmm9", "xmm10", "xmm11",
                      "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* COMPUTATION BLOCK 2 - Use all variables again to keep them live */
        vi4 = vi1 + vi2 * vi3;
        vi5 = vi4 ^ vi1;
        
        vf4 = vf1 + vf2 * vf3;
        vf5 = vf4 / vf1;
        
        vd4 = vd1 + vd2 * vd3;
        vd5 = vd4 - vd1;
        
        i9 = i6 + i7 * i8;
        i10 = i9 ^ i6;
        
        f9 = f6 + f7 * f8;
        f10 = f9 / f6;
        
        d9 = d6 + d7 * d8;
        d10 = d9 - d6;
        
        /* More vector ops */
        vec4 = vec1 + vec2 * vec3;
        dvec3 = dvec1 + dvec2;
        ivec3 = ivec1 + ivec2;
        
        /* Second external call with different clobbering */
        clobber_func2();
        
        /* Mixed clobber list */
        asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
        
        /* Third call in conditional block */
        if (loop % 2 == 0) {
            clobber_func3();
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm3", "xmm4");
        }
        
        /* COMPUTATION BLOCK 3 - Final computations */
        int temp1 = vi1 + vi2 + vi3 + vi4 + vi5;
        float temp2 = vf1 + vf2 + vf3 + vf4 + vf5;
        double temp3 = vd1 + vd2 + vd3 + vd4 + vd5;
        
        temp1 += i6 + i7 + i8 + i9 + i10;
        temp2 += f6 + f7 + f8 + f9 + f10;
        temp3 += d6 + d7 + d8 + d9 + d10;
        
        /* Vector reduction */
        for (int i = 0; i < 4; i++) {
            temp1 += ivec1[i] + ivec2[i] + ivec3[i];
            temp2 += vec1[i] + vec2[i] + vec3[i] + vec4[i];
        }
        for (int i = 0; i < 2; i++) {
            temp3 += dvec1[i] + dvec2[i] + dvec3[i];
        }
        
        /* Aggregate into total */
        total += temp1 + temp2 + temp3 + *p1 + *fp1 + *dp1 + loop;
    }
    
    /* Final output to prevent elimination */
    printf("Result: %f\n", total);
    
    /* Use all volatile variables one more time */
    asm volatile ("" :: "r"(vi1), "r"(vi2), "r"(vi3), "r"(vi4), "r"(vi5),
                  "r"(vf1), "r"(vf2), "r"(vf3), "r"(vf4), "r"(vf5),
                  "r"(vd1), "r"(vd2), "r"(vd3), "r"(vd4), "r"(vd5));
    
    return (int)total % 256;
}
