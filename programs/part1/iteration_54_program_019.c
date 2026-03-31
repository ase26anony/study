/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fdump-rtl-all -fdump-rtl-early_remat -c early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent function inlining to force argument passing */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
int use_values(int a, int b, float c, double d, 
               long e, short f, unsigned g, char h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + g + h;
    return sink & 1;
}

/* Another noinline function with different signature */
__attribute__((noinline))
float compute_more(float x, float y, float z, 
                   double a, double b, int i, int j) {
    volatile float vsink;
    vsink = x * y + z / a - b * (i % 16) + (j & 7);
    return vsink;
}

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_sink;
volatile float float_sink;

int main(void) {
    /* Initialize arrays with varying values */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = (i * 37) & 0xFF;
        array_float[i] = (i * 0.12345f);
        array_double[i] = (i * 0.98765);
    }
    
    /* Volatile pointer to force repeated dereferencing */
    volatile int* vptr = array_int;
    volatile float* vfptr = array_float;
    
    /* Accumulator to prevent dead code elimination */
    long long total = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating temporaries */
            int idx1 = (i + outer) & 0xFF;
            int idx2 = (i * 3 + outer) & 0xFF;
            int idx3 = (i * 5 + outer) & 0xFF;
            int idx4 = (i * 7 + outer) & 0xFF;
            
            /* Load volatile values (cannot be optimized away) */
            int v1 = vptr[idx1];
            int v2 = vptr[idx2];
            float f1 = vfptr[idx3];
            float f2 = vfptr[idx4];
            
            /* Complex expression with many temporaries */
            int t1 = v1 * v2 + idx1;
            int t2 = v1 / (v2 + 1) - idx2;
            int t3 = t1 ^ t2;
            int t4 = (t3 << 3) | (t3 >> 5);
            
            float ft1 = f1 * f2 + (float)idx3;
            float ft2 = f1 / (f2 + 1.0f) - (float)idx4;
            float ft3 = ft1 * ft2 - ft1 / (ft2 + 0.5f);
            float ft4 = ft3 * 2.0f + ft3 / 3.0f;
            
            double dt1 = (double)ft4 * 1.234567;
            double dt2 = dt1 / ((double)v1 + 0.001);
            double dt3 = dt2 * dt2 - dt1;
            double dt4 = sqrt(fabs(dt3)) + 0.5;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                     "xmm0", "xmm1", "xmm2", "xmm3",
                                     "xmm4", "xmm5", "memory");
            
            /* More computations after clobber */
            int t5 = t4 + (int)dt4;
            int t6 = t5 * 3 - (int)(ft4 * 10.0f);
            int t7 = t6 ^ (t5 >> 2);
            int t8 = t7 + (v1 & v2) | (idx1 ^ idx2);
            
            float ft5 = ft4 + (float)t8 * 0.01f;
            float ft6 = ft5 * ft5 - ft4;
            float ft7 = sinf(ft6) * cosf(ft5);
            float ft8 = ft7 * 100.0f + ft6;
            
            /* Call function with many arguments - forces register usage */
            int result = use_values(t1, t2, ft1, dt1, 
                                   (long)t3, (short)t4, 
                                   (unsigned)t5, (char)t6);
            
            /* Another function call */
            float fresult = compute_more(ft2, ft3, ft4, 
                                        dt2, dt3, t7, t8);
            
            /* Vector operations to consume SIMD registers */
            v4si vec_a = {t1, t2, t3, t4};
            v4si vec_b = {t5, t6, t7, t8};
            v4si vec_c = vec_a + vec_b;
            v4si vec_d = vec_a * vec_b - vec_c;
            
            v4sf vec_f1 = {ft1, ft2, ft3, ft4};
            v4sf vec_f2 = {ft5, ft6, ft7, ft8};
            v4sf vec_f3 = vec_f1 * vec_f2 + vec_f1 / (vec_f2 + 1.0f);
            
            /* More inline assembly between computations */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                                     "xmm6", "xmm7", "xmm8", "xmm9",
                                     "xmm10", "xmm11", "memory");
            
            /* Use vector results */
            int vec_sum = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
            float vec_fsum = vec_f3[0] + vec_f3[1] + vec_f3[2] + vec_f3[3];
            
            /* Volatile write to prevent optimization */
            global_sink = t8 + result;
            float_sink = ft8 + fresult + vec_fsum;
            
            /* Accumulate to total (prevents dead code elimination) */
            total += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + 
                    (int)ft1 + (int)ft2 + (int)ft3 + (int)ft4 +
                    (int)ft5 + (int)ft6 + (int)ft7 + (int)ft8 +
                    result + (int)fresult + vec_sum + (int)vec_fsum;
            
            /* Additional computations to increase pressure */
            double dt5 = dt4 * 1.1 + (double)total * 0.000001;
            double dt6 = dt5 * dt5 - dt4 * dt4;
            double dt7 = log(fabs(dt6) + 1.0);
            double dt8 = exp(dt7 * 0.1);
            
            int t9 = (int)(dt8 * 1000.0) & 0xFFF;
            int t10 = t9 * t8 - t7 * t6;
            int t11 = (t10 << 1) | (t10 >> 31);
            int t12 = t11 ^ (t9 & t10) | (t7 ^ t8);
            
            /* Final volatile write */
            global_sink = t12;
        }
        
        /* Outer loop computation */
        int outer_temp = outer * 17;
        float outer_float = sinf(outer * 0.1f);
        double outer_double = cos(outer * 0.01);
        
        total += outer_temp + (int)(outer_float * 100.0f) + 
                (int)(outer_double * 1000.0);
    }
    
    printf("Result: %lld\n", total);
    return (int)(total & 0x7FFFFFFF);
}
