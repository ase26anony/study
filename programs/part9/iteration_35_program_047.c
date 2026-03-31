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

/* Prevent inlining and optimization */
__attribute__((noinline)) 
static int use_variables(int start) {
    /* Declare MANY local variables of mixed types */
    volatile int v1 = start + 1;
    volatile int v2 = start + 2;
    volatile int v3 = start + 3;
    volatile int v4 = start + 4;
    volatile int v5 = start + 5;
    
    volatile float f1 = start * 1.1f;
    volatile float f2 = start * 2.2f;
    volatile float f3 = start * 3.3f;
    volatile float f4 = start * 4.4f;
    
    volatile double d1 = start * 1.111;
    volatile double d2 = start * 2.222;
    volatile double d3 = start * 3.333;
    
    /* Vector variables - use many vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f1};
    v4sf vec3 = {f3, f4, f1, f2};
    
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d2, d3};
    v2df dvec3 = {d3, d1};
    
    v4si ivec1 = {v1, v2, v3, v4};
    v4si ivec2 = {v2, v3, v4, v5};
    v4si ivec3 = {v3, v4, v5, v1};
    
    /* Pointer variables */
    int* p1 = (int*)&v1;
    int* p2 = (int*)&v2;
    float* p3 = &f1;
    float* p4 = &f2;
    double* p5 = &d1;
    double* p6 = &d2;
    
    /* Complex computations to create data dependencies */
    vec1 = vec1 + vec2 * vec3;
    vec2 = vec2 - vec3 / vec1;
    vec3 = vec1 * vec2 + vec3;
    
    dvec1 = dvec1 + dvec2 * dvec3;
    dvec2 = dvec2 - dvec3 / dvec1;
    dvec3 = dvec1 * dvec2 + dvec3;
    
    ivec1 = ivec1 + ivec2 * ivec3;
    ivec2 = ivec2 - ivec3 / ivec1;
    ivec3 = ivec1 * ivec2 + ivec3;
    
    /* Force register pressure with scalar computations */
    v1 = v1 * v2 + v3 - v4 / v5;
    v2 = v2 * v3 + v4 - v5 / v1;
    v3 = v3 * v4 + v5 - v1 / v2;
    v4 = v4 * v5 + v1 - v2 / v3;
    v5 = v5 * v1 + v2 - v3 / v4;
    
    f1 = f1 * f2 + f3 - f4 / f1;
    f2 = f2 * f3 + f4 - f1 / f2;
    f3 = f3 * f4 + f1 - f2 / f3;
    f4 = f4 * f1 + f2 - f3 / f4;
    
    d1 = d1 * d2 + d3 - d1 / d2;
    d2 = d2 * d3 + d1 - d2 / d3;
    d3 = d3 * d1 + d2 - d3 / d1;
    
    /* Pointer arithmetic */
    p1 = p1 + (v1 % 16);
    p2 = p2 + (v2 % 16);
    p3 = p3 + ((int)f1 % 16);
    p4 = p4 + ((int)f2 % 16);
    p5 = p5 + ((int)d1 % 16);
    p6 = p6 + ((int)d2 % 16);
    
    /* Clobber integer registers before call */
    asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    
    /* External call - forces caller-save */
    clobber_func1();
    
    /* Clobber vector registers after call */
    asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                 "xmm4", "xmm5", "xmm6", "xmm7",
                 "xmm8", "xmm9", "xmm10", "xmm11",
                 "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* More computations to keep variables live */
    vec1 = vec1 * 2.0f + vec2;
    vec2 = vec2 * 3.0f - vec3;
    vec3 = vec3 * 4.0f + vec1;
    
    dvec1 = dvec1 * 2.0 + dvec2;
    dvec2 = dvec2 * 3.0 - dvec3;
    dvec3 = dvec3 * 4.0 + dvec1;
    
    ivec1 = ivec1 * 2 + ivec2;
    ivec2 = ivec2 * 3 - ivec3;
    ivec3 = ivec3 * 4 + ivec1;
    
    /* Clobber different registers */
    asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
    
    clobber_func2();
    
    asm volatile("" ::: "memory", "rcx", "rdx", "xmm3", "xmm4", "xmm5");
    
    /* Final computations mixing all variable types */
    int result = v1 + v2 + v3 + v4 + v5;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += (int)d1 + (int)d2 + (int)d3;
    
    /* Extract elements from vectors */
    int vec_sum = 0;
    for (int i = 0; i < 4; i++) {
        vec_sum += ivec1[i] + ivec2[i] + ivec3[i];
    }
    
    result += vec_sum;
    
    clobber_func3();
    
    /* Final clobber to force save/restore */
    asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                 "xmm0", "xmm1", "xmm2", "xmm3");
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 3;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 10) iterations = 10;
        if (iterations < 1) iterations = 1;
    }
    
    int total = 0;
    
    /* Loop with conditional to create complex CFG */
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            /* Branch 1: More register pressure */
            total += use_variables(i * 10);
        } else {
            /* Branch 2: Different pattern */
            int temp = i * 20;
            
            /* Declare additional variables in this branch */
            volatile double extra_d1 = temp * 1.5;
            volatile double extra_d2 = temp * 2.5;
            volatile v4sf extra_vec = {1.0f, 2.0f, 3.0f, 4.0f};
            
            /* Clobber before call in this branch */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            
            clobber_func1();
            
            /* Different clobber pattern */
            asm volatile("" ::: "memory", "xmm2", "xmm3", "xmm4");
            
            /* Use the extra variables */
            extra_d1 = extra_d1 * extra_d2;
            extra_vec = extra_vec * 2.0f;
            
            total += (int)extra_d1 + (int)extra_d2 + i;
        }
        
        /* Inter-loop clobber */
        if (i < iterations - 1) {
            asm volatile("" ::: "memory", "rax", "xmm0");
        }
    }
    
    printf("Result: %d\n", total);
    return total;
}
