#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
float __attribute__((noinline)) 
process_floats(float f1, float f2, float f3, float f4, 
               float f5, float f6, float f7, float f8) {
    volatile float temp = f1 * f2 + f3 * f4 - f5 * f6 + f7 * f8;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    return temp;
}

/* Function that takes mixed types */
static double __attribute__((noinline))
mixed_operations(int i1, float f1, double d1, int i2, float f2, double d2) {
    volatile double result = (double)i1 * f1 + d1 - (double)i2 * f2 + d2;
    __asm__ volatile ("" : : : "rax", "r10", "xmm4", "xmm5");
    return result;
}

/* Function that uses alloca to affect frame pointer */
void* __attribute__((noinline))
create_buffer(int size) {
    void *ptr = alloca(size);
    __asm__ volatile ("" : : : "rsp", "rbp");
    return ptr;
}

/* Function with many live values across calls */
static int __attribute__((noinline))
complex_calculation(int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile float *fp1 = &f1, *fp2 = &f2;
    
    int sum = 0;
    
    /* Create basic blocks with calls inside */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            /* Block 1: Integer computations and call */
            v1 = v2 * v3 + v4;
            v5 = v1 - v2;
            sum += compute_sum(v1, v2, v3, v4, v5, i, i+1, i+2, i+3, i+4);
            
            /* Force register clobbering between computations */
            __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            
            f1 = f2 * 2.0f + f3;
            sum += (int)f1;
        } 
        else if (i % 3 == 1) {
            /* Block 2: Float computations and call */
            f2 = f3 * f4 - f1;
            f4 = f2 / 2.0f;
            float fresult = process_floats(f1, f2, f3, f4, f1+1.0f, f2+1.0f, f3+1.0f, f4+1.0f);
            sum += (int)fresult;
            
            /* Clobber floating point registers */
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
            
            d1 = d2 + d3;
            sum += (int)d1;
        } 
        else {
            /* Block 3: Mixed operations and alloca */
            d2 = d3 * 2.0 - d1;
            double dresult = mixed_operations(v1, f1, d1, v2, f2, d2);
            sum += (int)dresult;
            
            /* Use alloca to affect frame pointer */
            void *buffer = create_buffer(64);
            *(volatile int*)buffer = sum;
            
            /* More computations to keep values live */
            v3 = v4 * v5 - v1;
            v2 = v3 + *p1;
            sum += v2;
        }
        
        /* Additional computations between iterations */
        *p1 = *p2 + i;
        *fp1 = *fp2 * 1.5f;
        d3 = d1 + d2;
        
        /* Force another register clobber */
        __asm__ volatile ("" : : : "r8", "r9", "r10", "r11", "xmm6", "xmm7");
    }
    
    return sum;
}

/* Another helper with different signature */
static long __attribute__((noinline))
process_array(int *arr, int size) {
    volatile long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* Clobber registers in loop */
        if (i % 4 == 0) {
            __asm__ volatile ("" : : : "rax", "rbx", "rcx");
        }
    }
    return sum;
}

/* Main function with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile float fa = 1.5f, fb = 2.5f, fc = 3.5f, fd = 4.5f, fe = 5.5f;
    volatile double da = 1.25, db = 2.25, dc = 3.25, dd = 4.25;
    volatile int *pa = &a, *pb = &b;
    volatile float *pfa = &fa, *pfb = &fb;
    
    int result = 0;
    
    /* Take addresses to affect frame pointer optimization */
    volatile int *addr_array[] = {&a, &b, &c, &d, &e, &f, &g, &h, &i, &j};
    
    /* Complex control flow with calls in basic blocks */
    for (int iteration = 0; iteration < 100; iteration++) {
        if (iteration % 5 == 0) {
            /* Block with integer call */
            a = b + c * d - e;
            f = g / (h + 1) + i;
            result += compute_sum(a, b, c, d, e, f, g, h, i, j);
            
            /* Keep values live across asm clobber */
            __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "r8", "r9");
            
            fa = fb * fc + fd;
            result += (int)fa;
        } 
        else if (iteration % 5 == 1 || iteration % 5 == 2) {
            /* Block with float call */
            fb = fc * 1.1f - fd;
            float fres = process_floats(fa, fb, fc, fd, fe, fa+1.0f, fb+1.0f, fc+1.0f);
            result += (int)fres;
            
            /* Clobber between computations */
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
            
            da = db + dc * dd;
            result += (int)da;
        } 
        else {
            /* Block with mixed call and alloca */
            dc = dd * 2.5 - da;
            double dres = mixed_operations(a, fa, da, b, fb, db);
            result += (int)dres;
            
            /* Use alloca */
            void *temp = create_buffer(32);
            *(volatile int*)temp = result;
            
            /* Array processing */
            int arr[8] = {a, b, c, d, e, f, g, h};
            long arr_sum = process_array(arr, 8);
            result += (int)(arr_sum % 1000);
        }
        
        /* Additional interleaved computations */
        *pa = *pb + iteration;
        *pfa = *pfb * (1.0f + iteration * 0.01f);
        db = dc + dd * 0.5;
        
        /* Complex calculation call */
        if (iteration % 7 == 0) {
            int complex_res = complex_calculation(3);
            result += complex_res;
        }
        
        /* Final register clobber in the loop */
        __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
