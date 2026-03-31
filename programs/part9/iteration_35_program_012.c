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

int main(int argc, char *argv[]) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE VARIABLES - Prevent optimization */
    volatile int vol_int = 42;
    volatile float vol_float = 3.14159f;
    volatile double vol_double = 2.71828;
    
    /* MANY LOCAL VARIABLES - Create register pressure */
    /* Integer variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Floating point variables */
    float fa = 1.1f, fb = 2.2f, fc = 3.3f, fd = 4.4f, fe = 5.5f;
    double da = 1.11, db = 2.22, dc = 3.33, dd = 4.44, de = 5.55;
    
    /* Pointer variables */
    int *ptr1 = &a, *ptr2 = &b, *ptr3 = &c, *ptr4 = &d;
    float *fptr1 = &fa, *fptr2 = &fb, *fptr3 = &fc;
    
    /* Vector variables - use all vector registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v2df dvec1 = {1.23, 4.56};
    v2df dvec2 = {7.89, 10.11};
    v4si ivec1 = {100, 200, 300, 400};
    v4si ivec2 = {500, 600, 700, 800};
    
    /* Initial computations to establish data dependencies */
    a = b + c;
    d = e * f - g;
    h = i ^ j | k;
    l = m << 2;
    n = o >> 1;
    
    fa = fb * fc + fd;
    da = db / dc - dd;
    
    vec1 = vec1 + vec2 * vec3;
    vec4 = vec4 - vec1;
    dvec1 = dvec1 * dvec2;
    ivec1 = ivec1 & ivec2;
    
    /* Use volatile variables in computation */
    vol_int = a + vol_int;
    vol_float = fa * vol_float;
    vol_double = da + vol_double;
    
    int result = 0;
    
    /* LOOP with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pre-call computations - keep variables live */
        int temp1 = a + b + c + d;
        int temp2 = e + f + g + h;
        float ftemp1 = fa + fb + fc;
        double dtemp1 = da + db + dc;
        
        v4sf vtemp1 = vec1 + vec2;
        v4sf vtemp2 = vec3 * vec4;
        v2df dvtemp = dvec1 + dvec2;
        
        /* CLOBBER INTEGER REGISTERS before call */
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* CLOBBER VECTOR REGISTERS before call */
        asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7",
                      "xmm8", "xmm9", "xmm10", "xmm11",
                      "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* CLOBBER MEMORY */
        asm volatile ("" : : : "memory");
        
        /* EXTERNAL FUNCTION CALL - forces caller-save */
        clobber_func1();
        
        /* More clobbering after call */
        asm volatile ("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "memory");
        
        /* Post-call computations using pre-call values */
        int temp3 = temp1 + temp2 + i + j;
        float ftemp2 = ftemp1 * fd * fe;
        double dtemp2 = dtemp1 * dd * de;
        
        v4sf vtemp3 = vtemp1 + vtemp2;
        v2df dvtemp2 = dvtemp * dvec1;
        
        /* Second set of clobbering with different registers */
        asm volatile ("" : : : "rcx", "rdx", "xmm3", "xmm4", "xmm5", "memory");
        
        /* Another external call */
        clobber_func2();
        
        asm volatile ("" : : : "rsi", "rdi", "xmm6", "xmm7", "memory");
        
        /* Complex conditional inside loop */
        if (iter % 2 == 0) {
            /* Even iteration computations */
            a = temp3 + k + l;
            b = m + n + o + p;
            vec1 = vtemp3 * 2.0f;
            dvec1 = dvtemp2 / 2.0;
            
            /* Third clobber and call */
            asm volatile ("" : : : "r8", "r9", "r10", "xmm8", "xmm9", "memory");
            clobber_func3();
            asm volatile ("" : : : "r11", "r12", "xmm10", "xmm11", "memory");
            
            /* More computations */
            c = a ^ b;
            d = c << (iter + 1);
            vec2 = vec1 + vec3;
        } else {
            /* Odd iteration computations */
            a = temp3 - k - l;
            b = m * n - o * p;
            vec1 = vtemp3 / 2.0f;
            dvec1 = dvtemp2 * 2.0;
            
            /* Different clobber pattern */
            asm volatile ("" : : : "r13", "r14", "r15", "xmm12", "xmm13", "memory");
            clobber_func2();
            asm volatile ("" : : : "rax", "rbx", "xmm14", "xmm15", "memory");
            
            /* More computations */
            c = a | b;
            d = c >> (iter + 1);
            vec2 = vec1 - vec3;
        }
        
        /* Update volatile variables */
        vol_int += a + b + c + d;
        vol_float += fa + fb;
        vol_double += da + db;
        
        /* Aggregate results */
        result += a + b + c + d + (int)fa + (int)da;
        result += ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        
        /* Pointer arithmetic to keep pointers live */
        ptr1 = &a;
        ptr2 = &b;
        fptr1 = &fa;
        fptr2 = &fb;
        
        *ptr1 = *ptr1 + iter;
        *ptr2 = *ptr2 - iter;
        *fptr1 = *fptr1 * (iter + 1);
        *fptr2 = *fptr2 / (iter + 1);
    }
    
    /* Final aggregation and output */
    int final_result = result + vol_int + (int)vol_float + (int)vol_double;
    final_result += a + b + c + d + e + f + g + h;
    final_result += i + j + k + l + m + n + o + p;
    
    /* Use vector elements */
    final_result += vec1[0] + vec1[1] + vec1[2] + vec1[3];
    final_result += ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
