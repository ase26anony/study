/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Non-inline function to force argument passing */
__attribute__((noinline, noipa))
int use_values(int a, int b, float c, double d, int e, float f, double g, int h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + (int)f + (int)g + h;
    return sink & 1;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
float process_vector(float a, float b, float c, float d, 
                     float e, float f, float g, float h) {
    volatile float vsink;
    vsink = a * b - c / d + e - f * g + h;
    return vsink;
}

/* Vector type to consume SIMD registers */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main() {
    /* Volatile variables to prevent optimization */
    volatile int vseed = 42;
    volatile float vfloat = 3.14159f;
    volatile double vdouble = 2.71828;
    
    /* Arrays to create memory pressure */
    int array1[256];
    float array2[256];
    double array3[256];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.5;
    }
    
    int total = 0;
    float ftotal = 0.0f;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating temporaries */
            int t1 = array1[i] * 3 + outer;
            int t2 = array1[i + 64] / 2 - outer;
            int t3 = t1 * t2 + i;
            int t4 = t1 - t2 * outer;
            int t5 = t3 + t4 * i;
            int t6 = t5 - t1 * t2;
            
            /* Floating point computations */
            float f1 = array2[i] * 2.0f + outer;
            float f2 = array2[i + 32] / 1.5f - outer;
            float f3 = f1 * f2 + i;
            float f4 = f1 - f2 * outer;
            float f5 = f3 + f4 * i;
            float f6 = f5 - f1 * f2;
            
            /* Double precision computations */
            double d1 = array3[i] * 1.25 + outer;
            double d2 = array3[i + 16] / 2.75 - outer;
            double d3 = d1 * d2 + i;
            double d4 = d1 - d2 * outer;
            double d5 = d3 + d4 * i;
            double d6 = d5 - d1 * d2;
            
            /* Vector operations */
            v4sf vec1 = {f1, f2, f3, f4};
            v4sf vec2 = {f5, f6, f1, f2};
            v4sf vec3 = vec1 + vec2 * 1.5f;
            v4sf vec4 = vec1 - vec2 / 2.0f;
            v4sf vec5 = vec3 * vec4 + vec1;
            
            /* Extract results from vectors */
            float vec_result = vec5[0] + vec5[1] + vec5[2] + vec5[3];
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7",
                                       "memory");
            
            /* More computations after clobber */
            int t7 = t6 * 2 + i * outer;
            float f7 = f6 * 1.25f + i * 0.5f;
            double d7 = d6 * 0.75 + i * 0.25;
            
            /* Complex expression with many operands */
            int t8 = (t7 * 3 + t6 / 2 - t5 * 4 + t4 / 3) * 
                     (outer + i) - (t3 << 2) + (t2 >> 1);
            
            float f8 = (f7 * 2.3f + f6 / 1.7f - f5 * 3.1f + f4 / 2.9f) *
                       (outer * 0.1f + i * 0.01f) - f3 * 1.5f + f2 / 2.5f;
            
            double d8 = (d7 * 1.7 + d6 / 2.3 - d5 * 2.9 + d4 / 3.1) *
                        (outer * 0.05 + i * 0.005) - d3 * 1.2 + d2 / 2.1;
            
            /* Call non-inline function with many arguments */
            int func_result = use_values(t8, t7, f8, d8, 
                                         t6, f7, d7, t5);
            
            /* Another function call */
            float fresult = process_vector(f8, f7, f6, f5,
                                           f4, f3, f2, f1);
            
            /* Volatile memory access */
            volatile int* volatile_ptr = &array1[i];
            *volatile_ptr = t8;
            
            volatile float* volatile_fptr = &array2[i];
            *volatile_fptr = f8;
            
            /* More inline assembly */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                                       "xmm8", "xmm9", "xmm10", "xmm11",
                                       "memory");
            
            /* Final computations aggregating results */
            total += func_result + (int)fresult + (int)vec_result;
            ftotal += fresult + vec_result + f8;
            
            /* Additional computations to increase pressure */
            int t9 = t8 + func_result * i;
            float f9 = f8 + fresult * i;
            double d9 = d8 + (double)func_result * i;
            
            int t10 = t9 * 2 - t8 / 3 + func_result;
            float f10 = f9 * 1.1f - f8 / 2.2f + fresult;
            double d10 = d9 * 0.9 - d8 / 3.3 + (double)func_result;
            
            /* Use results to prevent elimination */
            array1[(i + 1) & 255] ^= t10;
            array2[(i + 1) & 255] += f10;
            array3[(i + 1) & 255] += d10;
        }
        
        /* Outer loop computations */
        int outer_tmp = outer * 7;
        float outer_ftmp = outer * 3.14f;
        double outer_dtmp = outer * 1.68;
        
        for (int j = 0; j < 4; j++) {
            outer_tmp = outer_tmp * 3 - j;
            outer_ftmp = outer_ftmp * 1.1f + j;
            outer_dtmp = outer_dtmp * 0.9 - j;
            
            /* More assembly to break live ranges */
            asm volatile("" : : : "r12", "r13", "r14", "r15",
                                       "xmm12", "xmm13", "xmm14", "xmm15",
                                       "memory");
        }
        
        total += outer_tmp;
        ftotal += outer_ftmp;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, %f\n", total, ftotal);
    
    /* Additional verification computation */
    int final_check = 0;
    for (int i = 0; i < 256; i++) {
        final_check += array1[i] + (int)array2[i];
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
