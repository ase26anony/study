#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force specific calling conventions and prevent optimizations */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with varying signatures to create diverse call sites */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    return result;
}

NOINLINE static float helper2(float a, float b, float c, float d, 
                               float e, float f, float g, float h,
                               float i, float j) {
    volatile float result = a * b + c * d - e * f + g * h - i * j;
    return result;
}

NOINLINE static void* helper3(void* p1, void* p2, void* p3, 
                               int i1, int i2, float f1, float f2) {
    volatile uintptr_t addr = (uintptr_t)p1 + (uintptr_t)p2 - (uintptr_t)p3;
    return (void*)(addr + i1 + i2 + (int)(f1 * f2));
}

NOINLINE static double helper4(double a, double b, double c, double d,
                               double e, double f, double g, double h,
                               double i, double j, double k, double l) {
    /* Many arguments to force stack passing */
    volatile double result = a + b - c * d + e / f - g * h + i - j + k * l;
    return result;
}

NOINLINE static int helper5(int* arr, float* farr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] + (int)farr[i];
    }
    return sum;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static void use_alloca(int size) {
    volatile char* buf = alloca(size);
    for (int i = 0; i < size; i++) {
        buf[i] = (char)(i % 256);
    }
    /* Use inline asm to clobber registers */
    __asm__ volatile (
        "nop"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
}

/* Complex function with mixed operations */
NOINLINE static int complex_calculation(int a, float b, double c, void* d) {
    volatile int result = a;
    result += (int)(b * 100.0f);
    result += (int)(c * 10.0);
    result += (int)((uintptr_t)d % 1000);
    
    /* More inline asm with clobbers */
    __asm__ volatile (
        "mov %%rax, %%rcx\n\t"
        "add $1, %%rcx"
        : 
        : 
        : "rax", "rcx"
    );
    
    return result;
}

int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    volatile double* p3 = &d1;
    
    /* Additional variables for more pressure */
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    int result = 0;
    float fresult = 0.0f;
    double dresult = 0.0;
    
    /* Create control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Multiple function calls with live values */
            v1 = helper1(v1, v2, v3, v4, v5, v6, v7, v8);
            
            /* Inline asm that clobbers call-clobbered registers */
            __asm__ volatile (
                "mov $0x12345678, %%eax\n\t"
                "add $1, %%eax"
                : 
                : 
                : "eax", "ebx", "ecx", "edx", "esi", "edi"
            );
            
            fresult = helper2(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);
            
            /* Keep values live across calls */
            v2 = v1 + v3;
            f2 = f1 + f3;
            
            void* ptr_result = helper3((void*)&v1, (void*)&v2, (void*)&v3,
                                       v4, v5, f4, f5);
            
            /* More computations keeping values in registers */
            v4 = v1 * v2 - v3;
            f4 = f1 * f2 / f3;
            
            dresult = helper4(d1, d2, d3, d4, d5, 
                             (double)f1, (double)f2, (double)f3,
                             (double)f4, (double)f5, d1, d2);
        } else {
            /* Block 2: Different sequence of calls */
            int arr[5] = {v1, v2, v3, v4, v5};
            float farr[5] = {f1, f2, f3, f4, f5};
            
            v6 = helper5(arr, farr, 5);
            
            /* Another inline asm with different clobbers */
            __asm__ volatile (
                "xor %%r10, %%r10\n\t"
                "inc %%r10"
                : 
                : 
                : "r10", "r11", "r12", "r13", "r14", "r15"
            );
            
            use_alloca(64 + iteration * 16);
            
            /* Complex computation chain */
            v7 = complex_calculation(v1, f1, d1, (void*)&v2);
            v8 = complex_calculation(v2, f2, d2, (void*)&v3);
            v9 = complex_calculation(v3, f3, d3, (void*)&v4);
            
            /* Mix in more arithmetic */
            d1 = d2 * d3 - d4 + d5;
            f1 = f2 + f3 * f4 - f5;
        }
        
        /* Loop-carried dependencies to keep values live */
        v1 = v1 + iteration;
        f1 = f1 + (float)iteration;
        d1 = d1 + (double)iteration;
        
        /* Additional pressure with pointer arithmetic */
        p1 = p1 + 1;
        p2 = p2 + 1;
        p3 = p3 + 1;
        
        /* Conditional with nested calls */
        if (v1 > 100) {
            v2 = helper1(v2, v3, v4, v5, v6, v7, v8, v9);
            f2 = helper2(f2, f3, f4, f5, f6, f7, f8, f9, f10, f1);
        } else {
            v3 = helper1(v3, v4, v5, v6, v7, v8, v9, v10);
            use_alloca(32);
        }
    }
    
    /* Final computation using all variables */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    fresult = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    dresult = d1 + d2 + d3 + d4 + d5;
    
    /* Final checksum */
    int checksum = result + (int)fresult + (int)dresult;
    printf("Checksum: %d\n", checksum);
    
    /* Use addresses of locals to affect frame pointer optimization */
    int* addr_array[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    float* faddr_array[] = {&f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8, &f9, &f10};
    
    /* One more complex call with address-taking */
    helper3(addr_array[0], addr_array[5], faddr_array[0], 
            checksum, result, fresult, (float)dresult);
    
    return checksum % 256;
}
