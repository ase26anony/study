/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h) {
    return a + b + (int)c + (int)d + e + f + (int)g + (int)h;
}

/* Another non-inline function with vector arguments */
NOINLINE v4si use_vector(v4si a, v4si b, v4si c, v4si d) {
    return a + b + c + d;
}

/* Function to create complex expressions */
NOINLINE double complex_expr(double base, int idx) {
    return (base * idx) / (idx + 1.0) + (idx % 7) - (idx / 3.0);
}

int main(void) {
    const int N = 1000;
    const int M = 50;
    int result = 0;
    
    /* Initialize some arrays */
    int* array1 = (int*)malloc(N * sizeof(int));
    float* array2 = (float*)malloc(N * sizeof(float));
    double* array3 = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3.14159f;
        array3[i] = i * 2.71828;
    }
    
    /* Volatile pointer to force memory operations */
    volatile int* volatile_ptr = array1;
    
    /* Main computational kernel with high register pressure */
    for (int outer = 0; outer < M; outer++) {
        /* Many temporary variables with different types */
        int t1 = outer * 3;
        int t2 = outer + 7;
        float t3 = outer * 1.5f;
        double t4 = outer * 2.5;
        int t5 = outer % 13;
        int t6 = outer / 5;
        float t7 = outer * 0.7f;
        double t8 = outer * 3.7;
        
        /* Nested loop to increase pressure */
        for (int inner = 0; inner < N; inner++) {
            /* Complex expressions creating many intermediate values */
            int idx = inner + outer;
            
            /* Multiple independent arithmetic chains */
            int a1 = array1[idx % N] * t1 + t2;
            int a2 = a1 / (t5 + 1) - t6;
            int a3 = a2 * (idx % 17) + (idx / 19);
            int a4 = a3 - t1 * t2 + t5 * t6;
            
            float b1 = array2[idx % N] * t3;
            float b2 = b1 + t7 - (float)idx * 0.3f;
            float b3 = b2 / (t3 + 1.0f) * (idx % 11);
            float b4 = b3 - t7 * 2.0f + (float)t1 * 0.5f;
            
            double c1 = array3[idx % N] * t4;
            double c2 = c1 + t8 - (double)idx * 0.7;
            double c3 = c2 / (t4 + 1.0) * (idx % 23);
            double c4 = c3 - t8 * 3.0 + (double)t2 * 1.5;
            
            /* More temporaries with mixed operations */
            int d1 = (a1 + a2) * (a3 - a4);
            float d2 = (b1 + b2) * (b3 - b4);
            double d3 = (c1 + c2) * (c3 - c4);
            
            int e1 = d1 % 31 + idx;
            float e2 = d2 * 1.1f + idx;
            double e3 = d3 * 1.3 + idx;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple registers */
            asm volatile(
                "# Force register pressure\n"
                "movq %%rax, %%rbx\n"
                "movq %%rcx, %%rdx\n"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "memory"
            );
            
            /* Another assembly block for floating point registers */
            asm volatile(
                "# Clobber FP/vector registers\n"
                "xorps %%xmm0, %%xmm0\n"
                "xorps %%xmm1, %%xmm1\n"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            /* Call function with many arguments - forces register moves */
            int func_result = use_values(
                a1, a2, b1, c1,
                a3, a4, b2, c2
            );
            
            /* More computations after function call */
            int f1 = func_result * e1;
            float f2 = (float)func_result * e2;
            double f3 = (double)func_result * e3;
            
            /* Complex expression with many operands */
            double g1 = complex_expr(f3, idx);
            double g2 = complex_expr(g1, outer);
            double g3 = complex_expr(g2, inner);
            
            /* Vector operations to consume SIMD registers */
            v4si vec1 = {a1, a2, a3, a4};
            v4si vec2 = {e1, func_result, idx, outer};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            
            v4si vec_result = use_vector(vec1, vec2, vec3, vec4);
            
            /* Volatile memory operations to prevent optimization */
            global_sink = vec_result[0];
            float_sink = f2;
            double_sink = g3;
            
            /* Force memory access through volatile pointer */
            int mem_val = volatile_ptr[idx % N];
            
            /* Final accumulation with complex expression */
            result += (f1 % 97) + (int)(f2 * 10.0f) + (int)(g3 * 5.0) 
                     + mem_val + vec_result[1] + vec_result[2];
            
            /* Another assembly clobber to break live ranges */
            asm volatile(
                "# Break register continuity\n"
                "movl $0, %%eax\n"
                "movl $0, %%ebx\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", "memory"
            );
        }
        
        /* Rotate temporaries to create varying patterns */
        int temp = t1;
        t1 = t2;
        t2 = t5;
        t5 = t6;
        t6 = temp;
        
        float ftemp = t3;
        t3 = t7;
        t7 = ftemp;
        
        double dtemp = t4;
        t4 = t8;
        t8 = dtemp;
    }
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
