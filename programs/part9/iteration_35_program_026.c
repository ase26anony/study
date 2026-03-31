#include <stdio.h>
#include <stdlib.h>

/* External functions to force caller-save behavior */
extern void foo(void);
extern void bar(void);
extern void baz(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char **argv) {
    /* Force register pressure with many live variables */
    volatile int v1 = argc * 1;
    volatile int v2 = argc * 2;
    volatile int v3 = argc * 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc * 5;
    volatile int v6 = argc * 6;
    volatile int v7 = argc * 7;
    volatile int v8 = argc * 8;
    
    volatile float f1 = argc * 1.1f;
    volatile float f2 = argc * 2.2f;
    volatile float f3 = argc * 3.3f;
    volatile float f4 = argc * 4.4f;
    
    volatile double d1 = argc * 1.11;
    volatile double d2 = argc * 2.22;
    volatile double d3 = argc * 3.33;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Create data dependencies between variables */
    int sum_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    float sum_float = f1 + f2 + f3 + f4;
    double sum_double = d1 + d2 + d3;
    
    /* Vector operations */
    vec1 = vec1 + vec2;
    dvec1 = dvec1 * dvec2;
    ivec1 = ivec1 + ivec2;
    
    /* Conditional control flow based on argc */
    int iterations = 3;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 10) iterations = 10;
    }
    
    /* Loop to create complex basic block structure */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        v1 = v2 * v3 + i;
        v2 = v3 * v4 - i;
        v3 = v4 * v5 + i * 2;
        v4 = v5 * v6 - i * 2;
        
        f1 = f2 * f3 + i * 0.5f;
        f2 = f3 * f4 - i * 0.5f;
        
        d1 = d2 * d3 + i * 0.25;
        d2 = d3 * d1 - i * 0.25;
        
        /* Vector operations */
        vec1 = vec1 * vec2 + (v4sf){i, i, i, i};
        dvec1 = dvec1 + dvec2 * (v2df){i * 0.1, i * 0.2};
        ivec1 = ivec1 + ivec2 * (v4si){i, i, i, i};
        
        /* Pointer arithmetic */
        p1 = p1 + (i % 2);
        fp1 = fp1 + (i % 3);
        
        /* Clobber specific integer registers before call */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx");
        
        /* Function call forcing caller-save */
        foo();
        
        /* Clobber specific vector registers after call */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");
        
        /* More computations to keep variables live across calls */
        v5 = v6 * v7 + i * 3;
        v6 = v7 * v8 - i * 3;
        v7 = v8 * v1 + i * 4;
        v8 = v1 * v2 - i * 4;
        
        f3 = f4 * f1 + i * 0.75f;
        f4 = f1 * f2 - i * 0.75f;
        
        d3 = d1 * d2 + i * 0.5;
        
        /* More vector operations */
        vec2 = vec2 - vec1 * (v4sf){i * 0.1f, i * 0.2f, i * 0.3f, i * 0.4f};
        dvec2 = dvec2 - dvec1;
        ivec2 = ivec2 - ivec1;
        
        /* Conditional branch inside loop */
        if (i % 2 == 0) {
            /* Clobber different registers */
            asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9");
            bar();
            asm volatile ("" ::: "memory", "xmm5", "xmm6", "xmm7", "xmm8");
            
            /* More computations */
            sum_int += v1 + v3 + v5 + v7;
            sum_float += f1 + f3;
        } else {
            /* Alternative path with different register clobbering */
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13");
            baz();
            asm volatile ("" ::: "memory", "xmm9", "xmm10", "xmm11", "xmm12");
            
            /* Different computations */
            sum_int += v2 + v4 + v6 + v8;
            sum_float += f2 + f4;
        }
        
        /* Update sums with vector elements */
        sum_int += ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        sum_float += vec1[0] + vec1[1] + vec1[2] + vec1[3];
        sum_double += dvec1[0] + dvec1[1];
    }
    
    /* Aggregate results to prevent elimination */
    double final_result = sum_int + sum_float + sum_double;
    
    /* Use all variables one more time */
    final_result += *p1 + *p2 + *fp1 + *fp2;
    final_result += vec2[0] + dvec2[1] + ivec2[2];
    
    printf("Result: %f\n", final_result);
    return (int)final_result % 256;
}
