#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions to force caller-save behavior */
extern void foo(void);
extern void bar(void);
extern void baz(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force memory clobbering and register spilling */
#define CLOBBER_INT_REGS asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")
#define CLOBBER_FLOAT_REGS asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15")

int main(int argc, char **argv) {
    /* Declare many local variables of mixed types to create register pressure */
    
    /* Integer variables */
    volatile int i1 = 1;
    volatile int i2 = 2;
    volatile int i3 = 3;
    volatile int i4 = 4;
    volatile int i5 = 5;
    volatile int i6 = 6;
    volatile int i7 = 7;
    volatile int i8 = 8;
    volatile int i9 = 9;
    volatile int i10 = 10;
    
    /* Floating point variables */
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile float f5 = 5.5f;
    
    /* Double precision variables */
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    volatile double d5 = 5.55;
    
    /* Pointer variables */
    volatile int *p1 = &i1;
    volatile int *p2 = &i2;
    volatile int *p3 = &i3;
    volatile float *p4 = &f1;
    volatile double *p5 = &d1;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df vecd1 = {1.0, 2.0};
    v2df vecd2 = {3.0, 4.0};
    v4si veci1 = {1, 2, 3, 4};
    v4si veci2 = {5, 6, 7, 8};
    
    /* Additional variables to increase pressure */
    volatile long long ll1 = 100;
    volatile long long ll2 = 200;
    volatile long long ll3 = 300;
    volatile long long ll4 = 400;
    
    volatile char c1 = 'a';
    volatile char c2 = 'b';
    volatile char c3 = 'c';
    
    /* Use command line argument to control loop iterations */
    int iterations = 3;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10) iterations = 10;
    }
    
    /* Result accumulator */
    volatile double result = 0.0;
    
    /* Complex loop with conditional control flow */
    for (int loop = 0; loop < iterations; loop++) {
        /* Pre-call computations to make variables live */
        if (loop % 2 == 0) {
            /* Even iteration path */
            i1 = i2 + i3;
            i4 = i5 * i6;
            f1 = f2 + f3;
            f4 = f5 * 2.0f;
            d1 = d2 + d3;
            d4 = d5 * 2.0;
            
            /* Vector operations */
            vec1 = vec1 + vec2;
            vec3 = vec3 * 2.0f;
            vecd1 = vecd1 + vecd2;
            veci1 = veci1 + veci2;
            
            /* Pointer arithmetic */
            *p1 = *p2 + *p3;
            *p4 = *p4 + 1.0f;
            *p5 = *p5 + 1.0;
            
            ll1 = ll2 + ll3;
            ll4 = ll4 * 2;
            
            /* Clobber integer registers before call */
            CLOBBER_INT_REGS;
            
            /* Function call that forces caller-save */
            foo();
            
            /* Clobber float registers after call */
            CLOBBER_FLOAT_REGS;
            
            /* Post-call computations to keep variables live */
            i2 = i1 - i4;
            i5 = i6 / i7;
            f2 = f1 - f4;
            f5 = f3 * 3.0f;
            d2 = d1 - d4;
            d5 = d3 * 3.0;
            
            vec2 = vec1 - vec3;
            veci2 = veci1 * 2;
            
            result += i1 + i2 + i3 + i4 + i5 + f1 + f2 + f3 + f4 + f5 + d1 + d2 + d3 + d4 + d5;
        } else {
            /* Odd iteration path - different pattern */
            i6 = i7 + i8;
            i9 = i10 * i1;
            f3 = f4 + f5;
            d3 = d4 + d5;
            
            vec1 = vec2 * vec3;
            vecd1 = vecd2 * 2.0;
            veci1 = veci2 + 1;
            
            *p2 = *p3 + *p1;
            *p4 = *p4 * 2.0f;
            *p5 = *p5 * 2.0;
            
            ll2 = ll3 + ll4;
            ll1 = ll1 * 3;
            
            /* Clobber different registers */
            CLOBBER_FLOAT_REGS;
            
            /* Different function call */
            bar();
            
            CLOBBER_INT_REGS;
            
            /* More computations */
            i7 = i6 - i9;
            i10 = i1 / i2;
            f4 = f3 - f1;
            d4 = d3 - d1;
            
            vec3 = vec1 - vec2;
            vecd2 = vecd1 / 2.0;
            
            result += i6 + i7 + i8 + i9 + i10 + f3 + f4 + d3 + d4;
        }
        
        /* Additional nested conditional with another call */
        if (loop % 3 == 0) {
            /* Mix all variable types */
            volatile int temp = i1 + i2 + i3;
            volatile float ftemp = f1 + f2 + f3;
            volatile double dtemp = d1 + d2 + d3;
            
            /* Vector cross-computation */
            v4sf vtemp = vec1 + vec2 + vec3;
            v2df vdtemp = vecd1 + vecd2;
            v4si itemp = veci1 + veci2;
            
            /* Clobber everything */
            CLOBBER_INT_REGS;
            CLOBBER_FLOAT_REGS;
            
            /* Third function call */
            baz();
            
            /* Use all results */
            result += temp + ftemp + dtemp;
            for (int j = 0; j < 4; j++) {
                result += vtemp[j];
                if (j < 2) result += vdtemp[j];
                result += itemp[j];
            }
        }
        
        /* Additional computations to increase register pressure across loop iterations */
        c1 = c2 + 1;
        c2 = c3 - 1;
        c3 = c1 + c2;
        
        /* Force spill/reload by using all variables */
        result += ll1 + ll2 + ll3 + ll4 + c1 + c2 + c3;
    }
    
    /* Final aggregation and output to prevent optimization */
    printf("Result: %f\n", result);
    
    /* Use all variables one more time to ensure they stay live */
    volatile int final_check = 
        i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
        *p1 + *p2 + *p3 + (int)*p4 + (int)*p5 +
        (int)ll1 + (int)ll2 + (int)ll3 + (int)ll4 +
        c1 + c2 + c3;
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
