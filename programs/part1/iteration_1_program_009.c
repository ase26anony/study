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

/* Function with pointer arguments and float operations */
static float __attribute__((noinline))
process_floats(float f1, float f2, float f3, float f4, float f5, 
               float f6, float f7, float f8, int* counter) {
    volatile float sum = f1 + f2 + f3 + f4;
    volatile float product = f5 * f6 * f7 * f8;
    
    /* Force register pressure with intermediate calculations */
    float temp1 = sum * 2.0f;
    float temp2 = product / 3.0f;
    float temp3 = temp1 + temp2;
    
    (*counter)++;
    
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");
    return temp3;
}

/* Function that takes mixed types and uses alloca to affect frame pointer */
int __attribute__((noinline)) 
complex_calculation(int base, float factor, char modifier) {
    /* Use alloca to force frame pointer usage */
    int* dynamic_array = (int*)alloca(sizeof(int) * 8);
    
    for (int i = 0; i < 8; i++) {
        dynamic_array[i] = base + i * (int)factor + modifier;
    }
    
    volatile int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dynamic_array[i];
    }
    
    __asm__ volatile ("" : : : "r10", "r11", "r12", "r13");
    return sum;
}

/* Function with variable arguments (simulated) */
static double __attribute__((noinline))
mixed_operations(int a, float b, double c, long d, short e) {
    volatile double result = (double)a + (double)b + c + (double)d + (double)e;
    
    /* Create register pressure with multiple intermediate values */
    double temp1 = result * 1.5;
    double temp2 = temp1 / 2.0;
    double temp3 = temp2 + 100.0;
    double temp4 = temp3 - 50.0;
    
    __asm__ volatile ("" : : : "xmm5", "xmm6", "xmm7", "xmm8", "xmm9");
    return temp4;
}

/* Another function to create more call sites */
static void __attribute__((noinline))
update_pointers(int** ptr1, float** ptr2, volatile int* counter) {
    static int static_data[16];
    static float static_floats[16];
    
    for (int i = 0; i < 8; i++) {
        static_data[i] = (*ptr1)[i] + *counter;
        static_floats[i] = (*ptr2)[i] * 0.5f;
    }
    
    *ptr1 = static_data;
    *ptr2 = static_floats;
    
    __asm__ volatile ("" : : : "r14", "r15", "rax", "rbx");
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 10.1, d2 = 20.2, d3 = 30.3;
    volatile long l1 = 100, l2 = 200, l3 = 300;
    volatile short s1 = 10, s2 = 20;
    
    /* Pointer variables to create aliasing and inhibit optimizations */
    int* ptr_int = &v1;
    float* ptr_float = &f1;
    volatile int call_counter = 0;
    
    /* Array to store intermediate results */
    int results[20];
    int result_index = 0;
    
    /* Control flow to create basic block boundaries */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic Block 1: Integer computations and call */
        if (iteration % 2 == 0) {
            /* Force many live values across calls */
            int sum1 = compute_sum(v1, v2, v3, v4, v5, 
                                  v1 + 1, v2 + 2, v3 + 3, v4 + 4, v5 + 5);
            
            /* Inline assembly between computations to force caller-save */
            __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            
            /* Use the result in further computations before next call */
            v1 = sum1 % 100;
            v2 = (v1 * v2) / 3;
            v3 = v2 + v3 - v1;
            
            results[result_index++] = sum1;
        }
        
        /* Basic Block 2: Floating point computations and call */
        float float_result = process_floats(f1, f2, f3, f4, f5,
                                           f1 * 1.1f, f2 * 1.2f, f3 * 1.3f,
                                           &call_counter);
        
        /* More inline assembly with different clobbers */
        __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                         "xmm4", "xmm5", "rax", "rcx");
        
        /* Update floating point values */
        f1 = float_result * 0.9f;
        f2 = f1 + f2;
        f3 = f2 * f3;
        f4 = f3 / f4;
        
        /* Basic Block 3: Mixed operations with conditional */
        if (call_counter > 0) {
            double mixed_result = mixed_operations(v1, f1, d1, l1, s1);
            
            __asm__ volatile ("" : : : "r8", "r9", "r10", "r11", 
                             "xmm6", "xmm7", "xmm8");
            
            d1 = mixed_result;
            d2 = d1 * 1.1;
            d3 = d2 + d3;
            
            results[result_index++] = (int)mixed_result;
        }
        
        /* Basic Block 4: Complex calculation with alloca */
        int complex_result = complex_calculation(v2, f2, (char)v3);
        
        __asm__ volatile ("" : : : "r12", "r13", "r14", "r15", 
                         "rbp", "rsp", "xmm9", "xmm10");
        
        v4 = complex_result % 256;
        v5 = v4 ^ v5;
        
        /* Basic Block 5: Pointer operations */
        update_pointers(&ptr_int, &ptr_float, &call_counter);
        
        /* Use pointer results in computations */
        if (ptr_int && ptr_float) {
            v1 += ptr_int[0];
            f1 += ptr_float[0];
        }
        
        /* Loop creates back edge, forming a non-trivial CFG */
        call_counter++;
    }
    
    /* Final computation using all variables */
    int final_checksum = 0;
    final_checksum += v1 + v2 + v3 + v4 + v5;
    final_checksum += (int)(f1 + f2 + f3 + f4 + f5);
    final_checksum += (int)(d1 + d2 + d3);
    final_checksum += (int)(l1 + l2 + l3);
    final_checksum += s1 + s2;
    
    for (int i = 0; i < result_index; i++) {
        final_checksum += results[i];
    }
    
    final_checksum += call_counter;
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Call counter: %d\n", call_counter);
    
    return final_checksum != 0 ? 0 : 1;
}
