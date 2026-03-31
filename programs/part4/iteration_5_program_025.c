#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_op(int x, double y, float z, int w) {
    volatile double res = (double)x * y + (double)z / (double)w;
    return res;
}

int main(void) {
    /* Phase 1: Declare many variables to create register pressure */
    
    /* Integer variables - many will need to stay live */
    register int r0 asm("ax") = rand();
    register int r1 asm("bx") = rand();
    volatile int v0 = rand();
    volatile int v1 = rand();
    int a0 = rand(), a1 = rand(), a2 = rand(), a3 = rand();
    int b0 = rand(), b1 = rand(), b2 = rand(), b3 = rand();
    int c0 = rand(), c1 = rand(), c2 = rand(), c3 = rand();
    int d0 = rand(), d1 = rand(), d2 = rand(), d3 = rand();
    
    /* Floating point variables - different register class */
    volatile float f0 = (float)rand() / RAND_MAX;
    volatile float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX, f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX, f5 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    volatile double dbl0 = (double)rand() / RAND_MAX;
    volatile double dbl1 = (double)rand() / RAND_MAX;
    double dbl2 = (double)rand() / RAND_MAX;
    double dbl3 = (double)rand() / RAND_MAX;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) array[i] = rand();
    
    volatile int* ptr0 = &array[0];
    volatile int* ptr1 = &array[128];
    
    /* Mixed type accumulators */
    volatile int int_acc = 0;
    volatile float float_acc = 0.0f;
    volatile double double_acc = 0.0;
    
    /* Phase 2: Loop with invariant spilling */
    volatile int loop_limit = 1000;
    
    for (volatile int iteration = 0; iteration < loop_limit; iteration++) {
        
        /* Complex expression 1: Mix integer operations with type conversions */
        int temp1 = a0 * b0 + c0 / (d0 + 1);
        float temp2 = (float)temp1 * f0 + f1 * (float)b1;
        double temp3 = (double)temp2 + dbl0 * (double)c1;
        
        /* Non-offsettable memory access - forces address reload */
        /* Large offset that may not fit in displacement field */
        int idx = a1 + b2 + (iteration & 0xFF);
        int mem_val = array[idx + 100];  /* Offset may be too large */
        
        /* Inline assembly that clobbers registers */
        /* Clobber both integer and floating point registers */
        asm volatile (
            "# Dummy assembly\n"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "memory"
        );
        
        /* Function call that forces register saves */
        int func_result = dummy_function(a2, b3, f2, dbl1, c2, f3);
        
        /* Complex expression 2: Bitwise and arithmetic mix */
        int temp4 = (a3 & b0) | (c3 << 2) + (d1 * 3);
        float temp5 = (float)temp4 / (f4 + 1.0f) * f5;
        
        /* Another non-simple memory access with type conversion */
        short short_val = (short)array[idx + 150];  /* Different size access */
        double temp6 = (double)short_val * dbl2 + dbl3;
        
        /* Second function call with different types */
        double func_result2 = complex_op(d2, temp6, temp5, mem_val);
        
        /* More inline assembly with different clobbers */
        asm volatile (
            "# More dummy assembly\n"
            : 
            : 
            : "r8", "r9", "r10", "r11", "xmm6", "xmm7", "xmm8", "xmm9",
              "xmm10", "xmm11", "memory"
        );
        
        /* Use register variables in conflicting contexts */
        /* r0 is bound to ax but used in float operation */
        float conflicted_float = (float)r0 * f0 + (float)r1;
        
        /* Complex addressing with pointer arithmetic */
        int* complex_ptr = &array[(a0 + b0 + iteration) & 0xFF];
        int complex_mem = *(complex_ptr + 50);  /* Non-simple offset */
        
        /* Type conversions between all types */
        double_acc += (double)func_result + func_result2;
        float_acc += (float)double_acc + conflicted_float;
        int_acc += (int)float_acc + complex_mem + temp1;
        
        /* Update some variables to keep them live */
        a0 = a0 * 1103515245 + 12345;
        b0 = b0 * 1103515245 + 12345;
        f0 = f0 * 1.1f;
        dbl0 = dbl0 * 1.01;
        
        /* Mix all variables in one giant expression */
        /* This creates many simultaneous live values */
        int giant_expr = 
            (a0 & a1) | (b0 ^ b1) +
            (int)(f0 * 100.0f) +
            (int)(dbl0 * 100.0) +
            (c0 * d0 - c1 * d1) +
            array[(a2 + b2) & 0xFF] +
            array[(a3 + b3 + 50) & 0xFF] +  /* Non-simple offset */
            (int)(f1 * f2 * 1000.0f) +
            (r0 % 100) + (r1 % 100);
            
        int_acc += giant_expr;
        
        /* Force spill by using all variables again */
        v0 = v0 + a0 + b0 + c0 + d0;
        v1 = v1 + a1 + b1 + c1 + d1;
        f1 = f1 + f2 + f3;
        dbl1 = dbl1 + dbl2 + dbl3;
    }
    
    /* Phase 3: Final complex expression using all variables */
    double final_result = 
        (double)int_acc + 
        (double)float_acc + 
        double_acc +
        (double)(a0 + a1 + a2 + a3) +
        (double)(b0 + b1 + b2 + b3) +
        (double)(c0 + c1 + c2 + c3) +
        (double)(d0 + d1 + d2 + d3) +
        (double)f0 + (double)f1 + (double)f2 + (double)f3 +
        dbl0 + dbl1 + dbl2 + dbl3 +
        (double)r0 + (double)r1;
    
    printf("Final result: %f\n", final_result);
    printf("Accumulators: int=%d float=%f double=%f\n", 
           int_acc, float_acc, double_acc);
    
    return (int)final_result % 256;
}
