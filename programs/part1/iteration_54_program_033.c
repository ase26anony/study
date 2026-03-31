/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fdump-rtl-all -fdump-rtl-early_remat -c early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Non-inline function to force argument passing */
__attribute__((noinline)) 
int use_values(int a, int b, float c, double d, int e, int f, float g, double h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    return sink;
}

/* Another non-inline function with different signature */
__attribute__((noinline))
double compute_more(double x, double y, double z, int i, int j, int k) {
    volatile double sink;
    sink = x * y - z / (i + j - k);
    return sink;
}

/* Global volatile to prevent optimizations */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

int main(void) {
    /* Initialize arrays with volatile elements to force memory operations */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3.14159f;
        array3[i] = i * 2.71828;
    }
    
    /* Main computational kernel with high register pressure */
    double total_sum = 0.0;
    
    /* Outer loop to increase iteration count */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested loops to create complex control flow */
        for (int i = 0; i < 128; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int temp1 = array1[i] * 3 + outer;
            int temp2 = array1[i + 128] / 5 - outer;
            float temp3 = array2[i] * 2.5f + i;
            double temp4 = array3[i] / 1.618 + outer;
            
            /* More temporaries with unique expressions */
            int temp5 = temp1 * temp2 - i * outer;
            float temp6 = temp3 * array2[i + 64] / (i + 1);
            double temp7 = temp4 + array3[255 - i] * 0.314;
            
            /* Even more temporaries with mixed operations */
            int temp8 = (temp1 << 3) | (temp2 & 0xFF);
            float temp9 = temp6 * temp6 - temp3 * 2.0f;
            double temp10 = temp7 * temp7 / (temp4 + 1.0);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple general purpose and floating point registers */
            asm volatile("" 
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "memory");
            
            /* Vector operations to consume SIMD registers */
            v4si vec1 = {temp1, temp2, temp8, i};
            v4si vec2 = {outer, i * 2, i * 3, i * 4};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2 - vec3;
            
            /* Extract elements from vectors, creating more temporaries */
            int vec_temp1 = vec4[0] + vec4[1];
            int vec_temp2 = vec4[2] * vec4[3];
            
            /* Call non-inline function with many arguments */
            /* Arguments are expressions, not variables, encouraging rematerialization */
            int func_result = use_values(
                temp1 + vec_temp1,
                temp2 - vec_temp2,
                temp3 + temp9,
                temp4 + temp10,
                temp5 * 2,
                temp8 / 3,
                array2[i] * 0.5f,
                array3[i] * 0.25
            );
            
            /* Another function call with different arguments */
            double dbl_result = compute_more(
                temp4,
                temp10,
                array3[255 - i],
                temp1,
                temp2,
                func_result
            );
            
            /* More arithmetic with the results */
            double combined = dbl_result * func_result + temp7;
            
            /* Volatile memory write to prevent elimination */
            global_counter = i + outer;
            global_accumulator = combined;
            
            /* Accumulate to final result (prevents dead code elimination) */
            total_sum += combined + temp4 + temp10 + dbl_result;
            
            /* Additional complex expression chain */
            for (int j = 0; j < 4; j++) {
                /* Inner loop creates more pressure with unique expressions */
                double inner_temp = (temp4 * j) / (temp10 + 1.0);
                float inner_float = temp3 * j - temp9;
                int inner_int = temp1 * j + temp2 * (3 - j);
                
                /* Another assembly clobber */
                asm volatile("" ::: "r8", "r9", "r10", "r11",
                                  "xmm11", "xmm12", "xmm13", "xmm14", "memory");
                
                total_sum += inner_temp + inner_float + inner_int;
            }
        }
        
        /* Additional computations between outer loop iterations */
        double inter_temp = 0.0;
        for (int k = 0; k < 16; k++) {
            inter_temp += array3[k] * outer / (k + 1.0);
        }
        total_sum += inter_temp;
    }
    
    /* Print result to prevent elimination and verify execution */
    printf("Total sum: %f\n", total_sum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
