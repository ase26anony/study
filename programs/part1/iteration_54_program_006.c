/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Non-inline function with many arguments */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int use_many_args(int a, int b, int c, float d, double e, 
                  long f, short g, char h, v4si vi, v4sf vf) {
    volatile int sink = 0;
    sink = a + b + c + (int)d + (int)e + f + g + h;
    /* Force vector usage */
    vi += (v4si){1, 2, 3, 4};
    vf *= (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
    return sink + vi[0] + (int)vf[0];
}

/* Another non-inline function for variety */
__attribute__((noinline))
double complex_math(double x, double y, double z, int i, int j) {
    /* Complex expression with many temporaries */
    double t1 = x * y + z;
    double t2 = x / (y + 1.0);
    double t3 = t1 * t2 - (x + y + z);
    double t4 = (t3 * t3) / (t1 + 1.0);
    double t5 = t4 + (i * j) / 1000.0;
    return t5 * 0.5 + sin(t1) * cos(t2);
}

int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int seed = 42;
    volatile double sink_global = 0.0;
    
    /* Arrays to force memory operations */
    double arr1[256];
    float arr2[256];
    int arr3[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = (i * 1.234) / (i + 1);
        arr2[i] = (i * 0.987f) / (i + 2);
        arr3[i] = i * 3;
    }
    
    /* Main computational kernel with high register pressure */
    double total = 0.0;
    
    /* Outer loop */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner loop with many independent computations */
        for (int i = 0; i < 128; i++) {
            /* MANY independent arithmetic operations creating temporaries */
            /* Each computation is slightly unique to avoid CSE */
            
            /* Group 1: Integer computations */
            int a = arr3[i] + outer * 7;
            int b = a * 3 - (i << 2);
            int c = b / (outer + 1) + (i % 16);
            int d = c ^ (a & b) | (i * outer);
            int e = (d << 3) + (a >> 2) - (b % 5);
            int f = e * e - d * c + b - a + i + outer;
            
            /* Group 2: Floating-point computations */
            double x = arr1[i] * 1.1 + outer * 0.01;
            double y = arr1[(i + 1) % 256] * 0.9 - outer * 0.02;
            double z = x * y + (x / (y + 1.0)) - (x + y);
            double w = z * z / (x + y + 1.0) + sin(x) * cos(y);
            double v = w * 0.5 + tanh(z) * 0.3 + exp(-fabs(x - y));
            
            /* Group 3: More mixed computations */
            float f1 = arr2[i] * 2.0f + outer * 0.1f;
            float f2 = arr2[(i + 3) % 256] * 1.5f - outer * 0.05f;
            float f3 = f1 * f2 + f1 / (f2 + 0.001f) - (f1 + f2);
            float f4 = f3 * f3 * 0.25f + sqrtf(fabs(f1 - f2));
            
            /* Group 4: Vector operations */
            v4si vec_i = {a, b, c, d};
            v4sf vec_f = {f1, f2, f3, f4};
            vec_i = vec_i * (v4si){2, 3, 4, 5} + (v4si){i, outer, i+outer, i*outer};
            vec_f = vec_f * (v4sf){1.1f, 1.2f, 1.3f, 1.4f} + 
                    (v4sf){0.1f * i, 0.2f * outer, 0.3f * (i+outer), 0.4f};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber multiple registers */
            asm volatile(
                "# Force register pressure\n"
                "movq $0, %%rax\n"
                "movq $0, %%rbx\n"
                "movq $0, %%rcx\n"
                "movq $0, %%rdx\n"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "memory"
            );
            
            /* More computations after assembly */
            double t1 = x + y + z + w + v;
            double t2 = f1 + f2 + f3 + f4;
            double t3 = (a + b + c + d + e + f) * 0.01;
            
            /* Call non-inline function with many arguments */
            int result = use_many_args(a, b, c, f1, x, 
                                      (long)(t1 * 1000), 
                                      (short)(t2 * 100),
                                      (char)(t3 * 10),
                                      vec_i, vec_f);
            
            /* Complex function call */
            double complex_result = complex_math(x, y, z, i, outer);
            
            /* Another assembly clobber */
            asm volatile(
                "# Clobber more registers\n"
                "pxor %%xmm0, %%xmm0\n"
                "pxor %%xmm1, %%xmm1\n"
                "pxor %%xmm2, %%xmm2\n"
                "pxor %%xmm3, %%xmm3\n"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            /* Volatile memory write to prevent optimization */
            volatile double temp_sink = 0.0;
            temp_sink = t1 + t2 + t3 + result + complex_result;
            
            /* More independent computations */
            double u1 = temp_sink * 0.3 + sin(i * 0.1) * 0.2;
            double u2 = u1 * u1 - cos(outer * 0.05) * 0.1;
            double u3 = sqrt(fabs(u2)) + log(fabs(u1) + 1.0);
            double u4 = u3 * 0.7 + tanh(u2 * 0.3) * 0.4;
            
            /* Final aggregation with many live values */
            total += u1 + u2 + u3 + u4 + 
                    (a * 0.01) + (b * 0.02) + (c * 0.03) +
                    (x * 0.04) + (y * 0.05) + (z * 0.06) +
                    (f1 * 0.07) + (f2 * 0.08) + (f3 * 0.09);
            
            /* Another assembly to break live ranges */
            asm volatile(
                "# Break live ranges\n"
                "mov $0, %%eax\n"
                "mov $0, %%ebx\n"
                "mov $0, %%ecx\n"
                "mov $0, %%edx\n"
                "mov $0, %%esi\n"
                "mov $0, %%edi\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
            );
        }
        
        /* Additional computations between inner loops */
        double loop_acc = 0.0;
        for (int j = 0; j < 16; j++) {
            loop_acc += arr1[(outer + j) % 256] * arr2[(outer * 2 + j) % 256];
        }
        total += loop_acc * 0.01;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Additional verification computation */
    volatile double verify = total;
    for (int i = 0; i < 1000; i++) {
        verify = verify * 1.0001 + i * 0.001;
    }
    printf("Verified: %f\n", verify);
    
    return 0;
}
