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

int main(int argc, char *argv[]) {
    /* Force many live variables across function calls */
    
    /* Integer variables */
    volatile int vi1 = argc + 1;
    int vi2 = argc * 2;
    int vi3 = argc + 3;
    int vi4 = argc * 4;
    int vi5 = argc + 5;
    int vi6 = argc * 6;
    int vi7 = argc + 7;
    int vi8 = argc * 8;
    
    /* Floating point variables */
    volatile float vf1 = argc * 1.1f;
    float vf2 = argc * 2.2f;
    double vd1 = argc * 3.3;
    double vd2 = argc * 4.4;
    volatile double vd3 = argc * 5.5;
    
    /* Pointer variables */
    char *ptr1 = argv[0];
    char *ptr2 = argv[argc > 1 ? 1 : 0];
    volatile char *ptr3 = ptr1 + 1;
    int *ptr4 = &vi1;
    float *ptr5 = &vf1;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    volatile v4sf vec3 = vec1 + vec2;
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Additional variables to increase pressure */
    long vl1 = argc * 10L;
    long vl2 = argc * 20L;
    volatile long vl3 = argc * 30L;
    short vs1 = argc * 40;
    short vs2 = argc * 50;
    
    /* Complex conditional control flow */
    int iterations = 3;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 10) iterations = 10;
    }
    
    int result = 0;
    
    /* Loop to create basic block structure */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        vi2 = vi1 + vi3 * i;
        vi4 = vi2 - vi5;
        vi6 = vi3 * vi7 + i;
        vi8 = vi4 ^ vi6;
        
        vf2 = vf1 * 1.5f + i;
        vd2 = vd1 * 2.0 + i * 0.1;
        
        /* Vector operations */
        vec1 = vec1 + vec3;
        vec2 = vec2 * vec1;
        dvec1 = dvec1 + dvec2;
        ivec1 = ivec1 + ivec2;
        
        /* Pointer arithmetic */
        ptr1 = ptr1 + i;
        ptr2 = ptr2 + (i % 2);
        *ptr4 = vi2;
        *ptr5 = vf2;
        
        /* Long and short operations */
        vl2 = vl1 + vl3 * i;
        vs2 = vs1 + i * 2;
        
        /* Force register clobbering before call */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* Function call that forces caller-save */
        if (i % 2 == 0) {
            clobber_func1();
        } else {
            clobber_func2();
        }
        
        /* More register clobbering after call */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Post-call computations using all variables */
        vi1 = vi2 + vi8;
        vi3 = vi4 * vi6 - vi7;
        vi5 = vi1 ^ vi3;
        vi7 = vi5 + vi8 * 2;
        
        vf1 = vf2 * 2.0f - vd1;
        vd3 = vd2 * 3.0 + vf1;
        
        /* More vector operations */
        vec3 = vec1 * vec2 + vec3;
        dvec2 = dvec1 * 2.0 - dvec2;
        ivec2 = ivec1 | ivec2;
        
        /* Conditional call based on computation */
        if (vi1 > vi3) {
            asm volatile("" ::: "memory", "r10", "r11", "r12", "r13",
                         "xmm8", "xmm9", "xmm10", "xmm11");
            clobber_func3();
            asm volatile("" ::: "memory", "r14", "r15", 
                         "xmm12", "xmm13", "xmm14", "xmm15");
        }
        
        /* Complex expression using all variable types */
        result += vi1 + vi3 + (int)vf1 + (int)vd3 + 
                  ptr1[0] + ptr2[0] + (int)vec3[0] + 
                  (int)dvec1[0] + ivec1[0] + vl2 + vs2;
        
        /* Additional conditional to create more BB structure */
        if (result % 7 == 0) {
            vi2 = vi2 * 3;
            vf2 = vf2 / 2.0f;
            asm volatile("" ::: "memory");
        } else if (result % 5 == 0) {
            vi4 = vi4 | 0xFF;
            vd1 = vd1 * 1.5;
            asm volatile("" ::: "memory", "rax", "xmm0");
        }
    }
    
    /* Final aggregation to prevent optimization */
    int final_result = result + vi1 + vi2 + vi3 + vi4 + vi5 + vi6 + vi7 + vi8 +
                      (int)vf1 + (int)vf2 + (int)vd1 + (int)vd2 + (int)vd3 +
                      (int)vec1[0] + (int)vec2[0] + (int)vec3[0] +
                      (int)dvec1[0] + (int)dvec2[0] +
                      ivec1[0] + ivec2[0] +
                      vl1 + vl2 + vl3 + vs1 + vs2 +
                      (int)(ptr1 - argv[0]) + (int)(ptr2 - argv[0]);
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}

/* External function declarations (no definitions to prevent inlining) */
void clobber_func1(void) {
    /* Empty - forces caller to save registers */
}

void clobber_func2(void) {
    /* Empty - forces caller to save registers */
}

void clobber_func3(void) {
    /* Empty - forces caller to save registers */
}
