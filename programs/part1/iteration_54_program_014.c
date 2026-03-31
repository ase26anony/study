/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile double fp_sink;

/* Non-inline function to force register usage for arguments */
__attribute__((noinline, noipa))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               v4si vec) {
    int sum = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    sum += vec[0] + vec[1] + vec[2] + vec[3];
    global_sink = sum;
    return sum & 1;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
double compute_more(double x, double y, double z,
                    int i, int j, int k,
                    float a, float b, float c) {
    double result = x * y + z / (i + 1) - j * k;
    result += (double)(a * b * c);
    fp_sink = result;
    return result;
}

int main(void) {
    /* Initialize arrays with volatile to force memory ops */
    volatile int array1[1024];
    volatile double array2[1024];
    volatile float array3[1024];
    
    for (int i = 0; i < 1024; i++) {
        array1[i] = i;
        array2[i] = i * 1.5;
        array3[i] = i * 0.75f;
    }
    
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Nested loops with many temporaries */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < ITERATIONS / 100; inner++) {
            /* Many independent computations creating register pressure */
            int idx = (outer * 37 + inner * 13) & 1023;
            
            /* Load volatile values (forces register use) */
            int v1 = array1[idx];
            double v2 = array2[idx];
            float v3 = array3[idx];
            
            /* Complex expression with many temporaries */
            int t1 = v1 * 3 + outer;
            int t2 = t1 / (inner + 1) - 7;
            int t3 = t2 * t2 - t1;
            int t4 = t3 + (inner << 2);
            int t5 = t4 ^ (v1 & 0xFF);
            int t6 = t5 * 17 - 31;
            
            /* Floating point computations */
            double ft1 = v2 * 2.5 + outer;
            double ft2 = ft1 / (inner + 2.0) - 3.14;
            double ft3 = ft2 * ft1 - v2;
            double ft4 = ft3 + (inner * 0.01);
            
            float ft5 = v3 * 1.5f + outer;
            float ft6 = ft5 / (inner + 1.0f) - 2.71f;
            float ft7 = ft6 * ft5 - v3;
            float ft8 = ft7 + (inner * 0.02f);
            
            /* More temporaries with mixed operations */
            int t7 = (int)(ft1 * 10) + t6;
            int t8 = t7 - (int)(ft2 * 5);
            int t9 = t8 * 3 + (int)(ft3 * 2);
            int t10 = t9 / (inner + 3) - (int)(ft4 * 7);
            
            double ft9 = (double)t1 * 0.25 + ft1;
            double ft10 = ft9 / ((double)t2 + 1.0) - ft2;
            double ft11 = ft10 * (double)t3 + ft3;
            double ft12 = ft11 - (double)t4 * 0.5 + ft4;
            
            /* Vector operations */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec3 * (v4si){2, 3, 4, 5};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", "r8", "r9", "r10", "r11",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Call function with many arguments - forces register allocation */
            int r1 = use_values(t1, t2, ft5, ft1,
                               t3, t4, ft6, ft2,
                               vec4);
            
            /* More computations between calls */
            int t11 = t10 + r1 * 11;
            double ft13 = ft12 * 1.1 + (double)r1;
            
            /* Another function call */
            double r2 = compute_more(ft9, ft10, ft11,
                                    t5, t6, t7,
                                    ft7, ft8, v3);
            
            /* Final computation using all temporaries */
            int final_val = t11 + (int)(ft13 * 100) + (int)(r2 * 50);
            
            /* More inline assembly to break live ranges */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "memory");
            
            /* Use result to prevent elimination */
            total += final_val & 1;
            
            /* Volatile write */
            global_sink = final_val;
        }
        
        /* Additional outer loop computation */
        int outer_temp = outer * 7;
        double outer_ftemp = (double)outer * 3.14159;
        
        for (int k = 0; k < 10; k++) {
            outer_temp = outer_temp * 3 - k;
            outer_ftemp = outer_ftemp / (k + 1.0) + 1.0;
            
            /* More register pressure */
            int temp1 = outer_temp + k;
            int temp2 = temp1 * 5 - 23;
            double ftemp1 = outer_ftemp * 2.0;
            double ftemp2 = ftemp1 / (temp2 + 1.0);
            
            asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
            
            total += (temp2 + (int)ftemp2) & 1;
        }
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
