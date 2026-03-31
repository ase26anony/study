#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);
extern void clobber_func4(void);

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
    
    /* Non-volatile variables for computations */
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400, i5 = 500;
    float f1 = 100.1f, f2 = 200.2f, f3 = 300.3f, f4 = 400.4f, f5 = 500.5f;
    double d1 = 100.11, d2 = 200.22, d3 = 300.33, d4 = 400.44, d5 = 500.55;
    
    /* Pointer variables */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    float *fp1 = &f1, *fp2 = &f2, *fp3 = &f3;
    double *dp1 = &d1, *dp2 = &d2, *dp3 = &d3;
    
    /* Vector variables - separate register bank pressure */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {13.0, 14.0};
    v2df dvec2 = {15.0, 16.0};
    v4si ivec1 = {17, 18, 19, 20};
    v4si ivec2 = {21, 22, 23, 24};
    
    /* Additional variables for more pressure */
    long long ll1 = 1000, ll2 = 2000, ll3 = 3000;
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 10, s2 = 20, s3 = 30;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Loop to create basic block structure */
    for (int iter = 0; iter < iterations; iter++) {
        /* COMPUTATION BLOCK 1 - Make variables live */
        i1 = vi1 + iter * 10;
        i2 = vi2 + iter * 20;
        i3 = vi3 + iter * 30;
        i4 = vi4 + iter * 40;
        i5 = vi5 + iter * 50;
        
        f1 = vf1 * (iter + 1);
        f2 = vf2 * (iter + 2);
        f3 = vf3 * (iter + 3);
        f4 = vf4 * (iter + 4);
        f5 = vf5 * (iter + 5);
        
        d1 = vd1 + iter;
        d2 = vd2 + iter * 2;
        d3 = vd3 + iter * 3;
        d4 = vd4 + iter * 4;
        d5 = vd5 + iter * 5;
        
        /* Vector computations */
        vec1 = vec1 + vec2 * (float)(iter + 1);
        vec3 = vec3 - vec1;
        dvec1 = dvec1 + dvec2 * (double)iter;
        ivec1 = ivec1 + ivec2 * iter;
        
        /* Pointer arithmetic */
        *p1 = i1 + i2;
        *p2 = i3 + i4;
        *p3 = i5 + iter;
        
        *fp1 = f1 + f2;
        *fp2 = f3 + f4;
        *fp3 = f5 + (float)iter;
        
        *dp1 = d1 + d2;
        *dp2 = d3 + d4;
        *dp3 = d5 + (double)iter;
        
        /* Additional computations */
        ll1 = ll1 + i1;
        ll2 = ll2 + i2;
        ll3 = ll3 + i3;
        
        s1 = s1 + c1 + iter;
        s2 = s2 + c2 + iter * 2;
        s3 = s3 + c3 + iter * 3;
        
        /* ASM CLOBBER 1 - Force save of specific registers */
        asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* EXTERNAL CALL 1 - Forces caller-save */
        clobber_func1();
        
        /* ASM CLOBBER 2 - Different register set */
        asm volatile("" : : : "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* COMPUTATION BLOCK 2 - Use all variables again (keep them live) */
        int sum1 = i1 + i2 + i3 + i4 + i5;
        float sum2 = f1 + f2 + f3 + f4 + f5;
        double sum3 = d1 + d2 + d3 + d4 + d5;
        
        /* More vector ops */
        v4sf vec_sum = vec1 + vec2 + vec3;
        v2df dvec_sum = dvec1 + dvec2;
        v4si ivec_sum = ivec1 + ivec2;
        
        /* ASM CLOBBER 3 - Mix scalar and vector */
        asm volatile("" : : : "memory", "r10", "r11", "r12", "r13", "r14", "r15",
                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* EXTERNAL CALL 2 */
        clobber_func2();
        
        /* ASM CLOBBER 4 */
        asm volatile("" : : : "memory", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Conditional block to create more complex CFG */
        if (iter % 2 == 0) {
            /* Even iteration path */
            vec1 = vec_sum * 2.0f;
            dvec1 = dvec_sum * 1.5;
            
            /* ASM with specific integer registers */
            asm volatile("" : : : "memory", "rax", "rbx", "xmm0", "xmm1");
            
            clobber_func3();
            
            asm volatile("" : : : "memory", "rcx", "rdx", "xmm2", "xmm3");
            
            ivec1 = ivec_sum + v4si{1, 2, 3, 4};
        } else {
            /* Odd iteration path */
            vec2 = vec_sum * 3.0f;
            dvec2 = dvec_sum * 2.5;
            
            /* Different register clobber set */
            asm volatile("" : : : "memory", "rsi", "rdi", "xmm4", "xmm5");
            
            clobber_func4();
            
            asm volatile("" : : : "memory", "r8", "r9", "xmm6", "xmm7");
            
            ivec2 = ivec_sum + v4si{5, 6, 7, 8};
        }
        
        /* COMPUTATION BLOCK 3 - Final computations */
        double iter_result = 
            (double)sum1 + (double)sum2 + sum3 +
            (double)vec_sum[0] + (double)vec_sum[1] +
            (double)dvec_sum[0] + (double)dvec_sum[1] +
            (double)ivec_sum[0] + (double)ivec_sum[1];
        
        /* Use volatile variables to force memory accesses */
        vi1 = iter;
        vf1 = (float)iter_result;
        vd1 = iter_result;
        
        total_result += iter_result;
        
        /* Final asm clobber in loop */
        asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx",
                     "xmm0", "xmm1", "xmm2", "xmm3",
                     "xmm4", "xmm5", "xmm6", "xmm7");
    }
    
    /* Final result computation and output */
    double final = total_result + 
                  (double)vi1 + (double)vf1 + vd1 +
                  (double)vec1[0] + (double)vec2[1] +
                  (double)dvec1[0] + (double)dvec2[1] +
                  (double)ivec1[0] + (double)ivec2[1];
    
    printf("Result: %f\n", final);
    
    return (int)final % 256;
}
