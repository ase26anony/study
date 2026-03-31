#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions to force calls */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force non-inline behavior */
__attribute__((noinline)) static void use_vars(
    int a, int b, int c, int d, int e, int f,
    float fa, float fb, float fc, float fd,
    double da, double db, double dc,
    v4sf* v1, v4sf* v2, v4si* vi,
    void* p1, void* p2, void* p3
) {
    /* Prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
    asm volatile("" : : "f"(fa), "f"(fb), "f"(fc), "f"(fd));
    asm volatile("" : : "f"(da), "f"(db), "f"(dc));
    asm volatile("" : : "x"(*v1), "x"(*v2), "x"(*vi));
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3));
}

int main(int argc, char** argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE variables to prevent optimization */
    volatile int start_val = argc;
    
    /* Many integer variables - will compete for integer registers */
    int var1 = start_val + 1;
    int var2 = start_val * 2;
    int var3 = start_val | 0xFF;
    int var4 = start_val ^ 0xAA;
    int var5 = start_val << 3;
    int var6 = ~start_val;
    int var7 = start_val + 100;
    int var8 = start_val - 50;
    int var9 = start_val * start_val;
    int var10 = start_val % 17;
    int var11 = start_val & 0x5555;
    int var12 = start_val | 0xAAAA;
    int var13 = start_val ^ 0x1234;
    int var14 = start_val + 0x1000;
    int var15 = start_val * 3;
    
    /* Floating point variables - compete for FP registers */
    float f1 = start_val * 1.1f;
    float f2 = start_val * 2.2f;
    float f3 = start_val * 3.3f;
    float f4 = start_val * 4.4f;
    float f5 = start_val * 5.5f;
    float f6 = start_val * 6.6f;
    
    /* Double variables - more FP pressure */
    double d1 = start_val * 1.111;
    double d2 = start_val * 2.222;
    double d3 = start_val * 3.333;
    double d4 = start_val * 4.444;
    
    /* Vector variables - compete for vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f5, f6, f1, f2};
    v4sf vec3 = {f3, f4, f5, f6};
    v4df vecd1 = {d1, d2};
    v4df vecd2 = {d3, d4, d1, d2};
    v4si veci1 = {var1, var2, var3, var4};
    v4si veci2 = {var5, var6, var7, var8};
    
    /* Pointer variables - integer register pressure */
    int* ptr1 = &var1;
    int* ptr2 = &var2;
    int* ptr3 = &var3;
    float* ptr4 = &f1;
    float* ptr5 = &f2;
    double* ptr6 = &d1;
    double* ptr7 = &d2;
    v4sf* ptr8 = &vec1;
    v4sf* ptr9 = &vec2;
    
    /* Complex computation creating data dependencies */
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    v4sf sum_vec = {0, 0, 0, 0};
    
    /* Loop with conditional to create complex CFG */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        var1 = var2 + var3;
        var2 = var4 ^ var5;
        var3 = var6 & var7;
        var4 = var8 | var9;
        var5 = var10 + var11;
        var6 = var12 - var13;
        var7 = var14 * var15;
        
        f1 = f2 * f3;
        f2 = f4 + f5;
        f3 = f6 - f1;
        f4 = f2 * f3;
        
        d1 = d2 / d3;
        d2 = d4 * d1;
        d3 = d2 - d4;
        
        vec1 = vec2 + vec3;
        vec2 = vec1 * vec3;
        vec3 = vec2 - vec1;
        
        veci1 = veci1 + veci2;
        veci2 = veci1 | veci2;
        
        /* CLOBBER integer registers before call */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                     "r13", "r14", "r15");
        
        /* CLOBBER floating point registers */
        asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", 
                     "xmm4", "xmm5", "xmm6", "xmm7",
                     "xmm8", "xmm9", "xmm10", "xmm11",
                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* External function call - forces caller-save */
        if (i % 2 == 0) {
            clobber_func1();
        } else {
            clobber_func2();
        }
        
        /* Different clobber set after call */
        asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", 
                     "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* More computations keeping variables live */
        var8 = var9 - var10;
        var9 = var11 * var12;
        var10 = var13 / (var14 + 1);
        var11 = var15 ^ var1;
        var12 = var2 & var3;
        var13 = var4 | var5;
        var14 = var6 + var7;
        var15 = var8 * var9;
        
        f5 = f6 * f1;
        f6 = f2 + f3;
        
        d4 = d1 * d2;
        d1 = d3 + d4;
        
        vec1 = vec1 * 2.0f;
        vec2 = vec2 + 1.0f;
        vec3 = vec3 - 0.5f;
        
        /* Another external call with different context */
        if (i % 3 == 0) {
            /* Clobber different registers */
            asm volatile("" ::: "memory", "r12", "r13", "r14", "r15",
                         "xmm8", "xmm9", "xmm10", "xmm11");
            clobber_func3();
            asm volatile("" ::: "memory", "xmm6", "xmm7", "xmm8", "xmm9");
        }
        
        /* Accumulate results */
        sum_int += var1 + var2 + var3 + var4 + var5 + var6 + var7 + 
                  var8 + var9 + var10 + var11 + var12 + var13 + var14 + var15;
        
        sum_float += f1 + f2 + f3 + f4 + f5 + f6;
        sum_double += d1 + d2 + d3 + d4;
        
        v4sf temp_vec = {f1, f2, f3, f4};
        sum_vec = sum_vec + vec1 + vec2 + vec3 + temp_vec;
        
        /* Use all pointers to keep them live */
        *ptr1 = *ptr2 + *ptr3;
        *ptr4 = *ptr5 * 1.1f;
        *ptr6 = *ptr7 / 2.0;
        
        vec1 = *ptr8 + *ptr9;
    }
    
    /* Final aggregation to prevent dead code elimination */
    float vec_sum = sum_vec[0] + sum_vec[1] + sum_vec[2] + sum_vec[3];
    double total = sum_int + sum_float + sum_double + vec_sum;
    
    /* Use variables one more time */
    use_vars(var1, var2, var3, var4, var5, var6,
             f1, f2, f3, f4, d1, d2, d3,
             &vec1, &vec2, &veci1,
             ptr1, ptr6, ptr8);
    
    printf("Result: %f (argc=%d)\n", total, argc);
    
    return (int)total % 256;
}
