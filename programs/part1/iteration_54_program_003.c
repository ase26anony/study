/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Force function calls to not be inlined */
__attribute__((noinline)) 
int use_values(int a, int b, float c, double d, int e, int f, float g, double h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    return sink;
}

/* Another noinline function to force register pressure */
__attribute__((noinline))
float complex_math(float a, float b, float c, float d, float e, float f) {
    return a * b + c / d - e * f + a / c + b * d - e / f;
}

/* Vector type to consume more registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

int main(void) {
    /* Initialize arrays with varying values */
    float fa[256], fb[256], fc[256];
    double da[256], db[256], dc[256];
    int ia[256], ib[256], ic[256];
    
    for (int i = 0; i < 256; i++) {
        fa[i] = i * 1.1f;
        fb[i] = i * 2.2f;
        fc[i] = i * 3.3f;
        da[i] = i * 1.5;
        db[i] = i * 2.5;
        dc[i] = i * 3.5;
        ia[i] = i * 4;
        ib[i] = i * 5;
        ic[i] = i * 6;
    }
    
    /* Volatile pointer to force memory operations */
    volatile float* vptr = fa;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Nested loops to create high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Many independent arithmetic operations creating temporaries */
            float t1 = fa[i] * 1.2345f + i * 0.9876f;
            float t2 = fb[i] / 3.14159f - i * 0.6543f;
            float t3 = fc[i] * 2.71828f + t1 * t2;
            float t4 = t1 / t2 - t3 * fa[i];
            float t5 = t2 * t3 + t4 / t1;
            float t6 = t3 - t4 * t5 + t1 / t2;
            
            double d1 = da[i] * 1.23456789 + i * 0.987654321;
            double d2 = db[i] / 3.14159265358979 - i * 0.654321;
            double d3 = dc[i] * 2.71828182845904 + d1 * d2;
            double d4 = d1 / d2 - d3 * da[i];
            double d5 = d2 * d3 + d4 / d1;
            double d6 = d3 - d4 * d5 + d1 / d2;
            
            int i1 = ia[i] * 7 + i * 11;
            int i2 = ib[i] / 3 - i * 13;
            int i3 = ic[i] * 5 + i1 * i2;
            int i4 = i1 / (i2 + 1) - i3 * ia[i];
            int i5 = i2 * i3 + i4 / (i1 + 1);
            int i6 = i3 - i4 * i5 + i1 / (i2 + 1);
            
            /* More operations mixing types */
            float t7 = t5 * d1 + i1 * 0.5f;
            double d7 = d5 * t2 + i2 * 0.25;
            int i7 = i5 + (int)t3 + (int)d3;
            
            /* Vector operations to consume wide registers */
            v4si vi1 = {i1, i2, i3, i4};
            v4si vi2 = {i5, i6, i7, i};
            v4si vi3 = vi1 + vi2 * 2;
            
            v4sf vf1 = {t1, t2, t3, t4};
            v4sf vf2 = {t5, t6, t7, fa[i]};
            v4sf vf3 = vf1 * vf2 + vf1 / (vf2 + 0.001f);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7",
                                       "memory");
            
            /* Force memory access with volatile */
            global_sink = i1;
            float_sink = t1;
            double_sink = d1;
            
            /* Access through volatile pointer */
            float temp = *(vptr + i);
            float_sink = temp;
            
            /* Call function with many arguments - forces register moves */
            int func_result = use_values(i1, i2, t1, d1, i3, i4, t2, d2);
            
            /* More computations after function call */
            float t8 = complex_math(t1, t2, t3, t4, t5, t6);
            double d8 = d1 * d2 + d3 * d4 - d5 * d6 + t8;
            int i8 = i1 * i2 - i3 * i4 + i5 * i6 + (int)t8;
            
            /* Another inline assembly to break live ranges */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                                       "xmm8", "xmm9", "xmm10", "xmm11",
                                       "memory");
            
            /* Accumulate results in different ways */
            total_result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
            total_result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
            total_result += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8;
            total_result += func_result;
            
            /* More volatile operations */
            global_sink = i8;
            *(vptr + ((i + 1) % 256)) = t8;
        }
        
        /* Additional computations between outer loop iterations */
        for (int j = 0; j < 10; j++) {
            float ftemp = outer * j * 0.123f;
            double dtemp = outer * j * 0.456;
            int itemp = outer * j * 789;
            
            ftemp = ftemp * 2.0f - ftemp / 3.0f + j * 0.789f;
            dtemp = dtemp * 1.5 - dtemp / 2.5 + j * 0.135;
            itemp = itemp * 3 - itemp / 4 + j * 246;
            
            total_result += ftemp + dtemp + itemp;
            
            /* Another assembly clobber */
            asm volatile("" : : : "r12", "r13", "r14", "r15",
                                       "xmm12", "xmm13", "xmm14", "xmm15",
                                       "memory");
        }
    }
    
    printf("Final result: %f\n", total_result);
    return (int)total_result % 256;
}
