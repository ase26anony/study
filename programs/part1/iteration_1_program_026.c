#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Force register pressure within the callee */
    int t1 = result * 2;
    int t2 = t1 + 17;
    int t3 = t2 - result;
    int t4 = t3 * 3;
    int t5 = t4 / 2;
    return t5;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float* f1, float* f2, float* f3, float* f4, float* f5) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5;
    /* Create internal register pressure */
    float tmp1 = sum * 1.5f;
    float tmp2 = tmp1 - 0.25f;
    float tmp3 = tmp2 * 2.0f;
    float tmp4 = tmp3 / 1.333f;
    return tmp4;
}

/* Function that clobbers registers via inline asm */
void __attribute__((noinline))
register_clobber(void) {
    /* Explicitly clobber call-clobbered registers */
    __asm__ volatile (
        "nop"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

/* Function using alloca to affect frame pointer */
int __attribute__((noinline))
use_alloca(int size) {
    char* buffer = (char*)alloca(size);
    volatile int sum = 0;
    for (int i = 0; i < size && i < 16; i++) {
        buffer[i] = (char)(i + 1);
        sum += buffer[i];
    }
    return sum;
}

/* Function with mixed types */
double __attribute__((noinline))
mixed_computation(int a, float b, double c, int* d, float* e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    
    /* Force floating point register pressure */
    double t1 = result * 1.234;
    double t2 = t1 + 5.678;
    double t3 = t2 - result;
    double t4 = t3 * 2.0;
    
    return t4;
}

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile double d1 = 5.55;
    volatile int* ptr1 = &v1;
    volatile int* ptr2 = &v2;
    
    int result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Take addresses to affect frame pointer decisions */
    int* addr1 = &v1;
    int* addr2 = &v2;
    float* addr3 = &f1;
    
    /* Complex control flow to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        global_counter++;
        
        if (iteration % 2 == 0) {
            /* Block 1: Multiple function calls with live values between them */
            
            /* Compute values that will stay in registers */
            int temp1 = v1 + v2 + v3;
            int temp2 = v4 * v5 - v6;
            float temp3 = f1 * f2 + f3;
            
            /* First call - many arguments */
            int call1_result = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
            
            /* Use the computed values before they're clobbered */
            temp1 += call1_result;
            temp2 -= v7;
            
            /* Inline asm that clobbers registers */
            register_clobber();
            
            /* Use the values again - they must be saved/restored */
            temp3 += (float)temp1 * 0.5f;
            
            /* Second call with pointer arguments */
            float call2_result = process_floats(&f1, &f2, &f3, &f4, &f1);
            
            /* More computations with live values */
            temp2 += (int)(call2_result * 10.0f);
            
            /* Third call using alloca */
            int call3_result = use_alloca(32 + iteration);
            
            /* Accumulate results */
            result += temp1 + temp2 + call3_result;
            float_result += temp3 + call2_result;
            
        } else {
            /* Block 2: Different sequence of calls */
            
            /* Different set of live values */
            int temp4 = v8 + v9 + v10;
            float temp5 = f2 * f3 - f4;
            double temp6 = d1 * 2.0;
            
            /* Call with mixed types */
            double call4_result = mixed_computation(v1, f1, d1, &v5, &f2);
            
            /* Use values before clobber */
            temp4 += (int)call4_result;
            temp5 += (float)(call4_result / 2.0);
            
            /* Another register clobber */
            register_clobber();
            
            /* More computations */
            temp6 += call4_result * 1.5;
            
            /* Another call with many arguments */
            int call5_result = compute_sum(v10, v9, v8, v7, v6, v5, v4, v3, v2, v1);
            
            /* Final computations in this block */
            temp4 += call5_result;
            
            /* Accumulate results */
            result += temp4;
            float_result += temp5;
            double_result += temp6 + call4_result;
        }
        
        /* Loop-carried dependencies to keep values live across iterations */
        v1 += 1;
        v2 += 2;
        f1 += 0.5f;
        f2 += 0.25f;
        
        /* Another call at loop end */
        if (iteration < 2) {
            int loop_call_result = compute_sum(
                v1, v2, v3, v4, v5, 
                v6, v7, v8, v9, v10
            );
            result += loop_call_result;
        }
    }
    
    /* Final computation and output */
    int final_result = result + (int)float_result + (int)double_result;
    
    /* Use the addresses to prevent dead store elimination */
    *addr1 = final_result % 100;
    *addr2 = final_result / 100;
    
    printf("Result: %d (float: %.2f, double: %.2f)\n", 
           final_result, float_result, double_result);
    
    return final_result != 0 ? 0 : 1;
}
