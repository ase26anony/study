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
    
    /* Declare MANY local variables of mixed types to maximize register pressure */
    
    /* Integer variables - all initialized with distinct values */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    volatile int vi6 = 6, vi7 = 7, vi8 = 8, vi9 = 9, vi10 = 10;
    volatile int vi11 = 11, vi12 = 12, vi13 = 13, vi14 = 14, vi15 = 15;
    
    /* Floating point variables */
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f, vf5 = 5.5f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33, vd4 = 4.44, vd5 = 5.55;
    
    /* Pointer variables */
    int arr1[10], arr2[10], arr3[10];
    volatile int *vp1 = arr1, *vp2 = arr2, *vp3 = arr3;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 10; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Result accumulator */
    volatile double total = 0.0;
    
    /* Main loop with conditional control flow */
    for (int i = 0; i < iterations; i++) {
        /* Complex pre-call computations using all variables to keep them live */
        
        /* Integer computations */
        vi1 = vi2 + vi3 * vi4;
        vi5 = vi6 - vi7 / (vi8 + 1);
        vi9 = vi10 ^ vi11 | vi12 & vi13;
        vi14 = vi15 << (i % 4);
        
        /* Floating point computations */
        vf1 = vf2 * vf3 + vf4 - vf5;
        vd1 = vd2 / (vd3 + 0.001) * vd4 - vd5;
        
        /* Pointer arithmetic */
        vp1 = &arr1[(vi1 + i) % 10];
        vp2 = &arr2[(vi2 * i) % 10];
        vp3 = &arr3[(vi3 ^ i) % 10];
        
        /* Vector operations - these use SSE/AVX registers */
        vec1 = vec1 + vec2 * vec3;
        vec2 = vec2 - vec1;
        dvec1 = dvec1 * dvec2 + (v2df){0.5, 0.5};
        ivec1 = ivec1 + ivec2 * (v4si){i, i, i, i};
        
        /* First asm volatile - clobber integer and vector registers */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* External function call - forces caller-save */
        clobber_func1();
        
        /* Second asm volatile - clobber different registers */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations to keep variables live across calls */
        vi2 = vi3 + vi4 * vi5;
        vi6 = vi7 - vi8 / (vi9 + 1);
        vf2 = vf3 * vf4 + vf5 - vf1;
        vd2 = vd3 / (vd4 + 0.001) * vd5 - vd1;
        
        /* More vector operations */
        vec3 = vec3 + vec1;
        dvec2 = dvec2 * dvec1;
        ivec2 = ivec2 + ivec1;
        
        /* Third asm volatile - clobber more registers */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13",
                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* Another external call */
        clobber_func2();
        
        /* Fourth asm volatile */
        asm volatile("" ::: "memory", "r14", "r15", 
                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Conditional branch inside loop to create complex CFG */
        if (i % 2 == 0) {
            /* Even iteration: different computation path */
            vi3 = vi4 + vi5 * vi6;
            vf3 = vf4 * vf5 + vf1 - vf2;
            
            /* Another external call in the branch */
            clobber_func3();
            
            /* Clobber registers in the branch */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
        } else {
            /* Odd iteration: alternative path */
            vi4 = vi5 + vi6 * vi7;
            vf4 = vf5 * vf1 + vf2 - vf3;
            
            /* Different register clobbering */
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
        }
        
        /* Post-call computations using all variables */
        vi7 = vi8 + vi9 * vi10;
        vi11 = vi12 - vi13 / (vi14 + 1);
        vf5 = vf1 * vf2 + vf3 - vf4;
        vd3 = vd4 / (vd5 + 0.001) * vd1 - vd2;
        
        /* More vector operations */
        vec1 = vec2 + vec3 * (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        dvec1 = dvec2 + (v2df){i * 0.01, i * 0.02};
        
        /* Accumulate results to prevent elimination */
        total += vi1 + vi2 + vi3 + vi4 + vi5;
        total += vf1 + vf2 + vf3 + vf4 + vf5;
        total += vd1 + vd2 + vd3 + vd4 + vd5;
        total += *vp1 + *vp2 + *vp3;
        
        /* Extract and add vector elements */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        total += vec_sum;
    }
    
    /* Final computation and output to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    return (int)total % 256;
}
