#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
    return result;
}

/* Function with pointer arguments */
float __attribute__((noinline)) 
process_floats(float *arr, int len, float scale) {
    volatile float sum = 0.0f;
    for (int i = 0; i < len; i++) {
        sum += arr[i] * scale;
        /* Artificial register pressure */
        __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    }
    return sum;
}

/* Function that takes address of locals (affects frame pointer) */
static void __attribute__((noinline)) 
use_addresses(int *ptr1, float *ptr2, double *ptr3) {
    volatile int local1 = *ptr1 + 10;
    volatile float local2 = *ptr2 * 2.0f;
    volatile double local3 = *ptr3 / 2.0;
    
    /* Force register clobbering */
    __asm__ volatile ("" : : : "r10", "r11", "r12", "r13");
    
    *ptr1 = local1;
    *ptr2 = local2;
    *ptr3 = local3;
}

/* Function with mixed types */
double __attribute__((noinline))
mixed_computation(int a, float b, double c, int *d, float *e) {
    volatile double result = (double)a + (double)b + c;
    *d = (int)result;
    *e = (float)result;
    
    /* Clobber multiple register classes */
    __asm__ volatile ("" : : : "rax", "xmm3", "xmm4", "r14");
    
    return result;
}

/* Function using alloca to affect stack frame */
void* __attribute__((noinline))
create_temp_buffer(int size) {
    void *buf = alloca(size);
    volatile int *p = (int*)buf;
    for (int i = 0; i < size / sizeof(int); i++) {
        p[i] = i * 2;
    }
    return buf;
}

/* Main function with complex control flow and register pressure */
int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile float *fp1 = &f1, *fp2 = &f2;
    volatile double checksum = 0.0;
    
    /* Array to create more pressure */
    float arr[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Loop to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic block 1: computations then call */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        f1 = f1 * 1.5f;
        f2 = f2 + 0.5f;
        
        /* Inline assembly between computations to force live ranges */
        __asm__ volatile ("" : : : "eax", "ebx", "ecx", "xmm0", "xmm1");
        
        /* Function call with many arguments (exceeds register passing on x86) */
        int sum = compute_sum(v1, v2, v3, v4, v5, 
                             iteration, iteration*2, iteration*3,
                             iteration*4, iteration*5);
        
        /* Basic block 2: if statement creating control flow */
        if (sum > 50) {
            /* More computations before next call */
            d1 = d1 * 1.1;
            d2 = d2 / 1.1;
            
            __asm__ volatile ("" : : : "edx", "esi", "edi", "xmm2", "xmm3");
            
            /* Call with pointer arguments */
            float float_sum = process_floats(arr, 8, f1);
            f3 = float_sum;
            
            /* Use alloca to affect frame pointer */
            void *temp_buf = create_temp_buffer(64);
            (void)temp_buf; /* Use to avoid unused warning */
        } else {
            /* Alternative path with different calls */
            v3 = v3 * 3 - iteration;
            v4 = v4 + v3;
            
            __asm__ volatile ("" : : : "r8", "r9", "r10", "xmm4", "xmm5");
            
            /* Call function that takes addresses */
            use_addresses(&v1, &f1, &d1);
            
            /* Mixed type function call */
            double mixed = mixed_computation(v1, f1, d1, &v2, &f2);
            d3 = mixed;
        }
        
        /* Common code after if/else */
        checksum += (double)v1 + (double)v2 + (double)f1 + f2 + d1 + d2;
        
        /* Another inline assembly to create more live ranges */
        __asm__ volatile ("" : : : "r11", "r12", "r13", "xmm6", "xmm7");
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            v5 = v5 + j;
            f4 = f4 * (1.0f + j * 0.1f);
            
            /* Call within nested loop */
            int inner_sum = compute_sum(v5, v4, v3, v2, v1,
                                       j, j*2, j*3, j*4, j*5);
            checksum += inner_sum;
        }
    }
    
    /* Final computation and output */
    printf("Final checksum: %f\n", checksum);
    printf("v1=%d, v2=%d, v3=%d, v4=%d, v5=%d\n", v1, v2, v3, v4, v5);
    printf("f1=%f, f2=%f, f3=%f, f4=%f\n", f1, f2, f3, f4);
    printf("d1=%f, d2=%f, d3=%f\n", d1, d2, d3);
    
    return 0;
}
