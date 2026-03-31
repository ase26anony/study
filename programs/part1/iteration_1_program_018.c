#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force specific calling conventions and prevent optimizations */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with varying signatures to create diverse call sites */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    return result * 2;
}

NOINLINE float helper2(float a, float b, float c, float d, 
                       float e, float f, float g, float h,
                       float i, float j) {
    volatile float sum = a + b + c + d + e + f + g + h + i + j;
    return sum / 10.0f;
}

NOINLINE static void* helper3(void* p1, void* p2, void* p3, 
                              int i1, int i2, float f1, float f2) {
    volatile uintptr_t addr = (uintptr_t)p1 + (uintptr_t)p2 - (uintptr_t)p3;
    return (void*)(addr + i1 + i2 + (int)(f1 + f2));
}

NOINLINE double helper4(double a, double b, double c, double d,
                        double e, double f, double g) {
    volatile double prod = a * b * c * d * e * f * g;
    return prod / 1000.0;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static int use_alloca(int size) {
    char* buffer = alloca(size);
    volatile int sum = 0;
    for (int i = 0; i < size && i < 16; i++) {
        buffer[i] = i;
        sum += buffer[i];
    }
    return sum;
}

/* Function with inline assembly that clobbers registers */
NOINLINE static void clobber_registers(void) {
    /* Clobber multiple call-clobbered registers */
    __asm__ volatile (
        "# Clobber registers\n\t"
        "mov $0x12345678, %%eax\n\t"
        "mov $0x87654321, %%ecx\n\t"
        "mov $0x11111111, %%edx\n\t"
        "mov $0x22222222, %%r8\n\t"
        "mov $0x33333333, %%r9\n\t"
        "mov $0x44444444, %%r10\n\t"
        "mov $0x55555555, %%r11\n\t"
        :
        : 
        : "eax", "ecx", "edx", "r8", "r9", "r10", "r11", "memory"
    );
}

/* Complex function with mixed operations */
NOINLINE static int complex_calculation(int a, float b, double c, void* d) {
    volatile int vi = a;
    volatile float vf = b;
    volatile double vd = c;
    volatile uintptr_t vp = (uintptr_t)d;
    
    /* Force register pressure with many intermediate values */
    int r1 = vi * 2;
    int r2 = vi + 100;
    int r3 = vi - 50;
    int r4 = vi / 3;
    int r5 = vi % 7;
    
    float f1 = vf * 1.5f;
    float f2 = vf + 3.14f;
    float f3 = vf - 2.71f;
    float f4 = vf / 4.0f;
    
    double d1 = vd * 2.0;
    double d2 = vd + 1.618;
    double d3 = vd - 0.577;
    
    uintptr_t p1 = vp + 0x1000;
    uintptr_t p2 = vp - 0x800;
    uintptr_t p3 = vp ^ 0xFFFF;
    
    /* Use all values to prevent optimization */
    return (int)(r1 + r2 + r3 + r4 + r5 + 
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                 (int)d1 + (int)d2 + (int)d3 +
                 (int)p1 + (int)p2 + (int)p3);
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    volatile int* p3 = &v3;
    volatile float* fp1 = &f1;
    volatile float* fp2 = &f2;
    volatile double* dp1 = &d1;
    volatile double* dp2 = &d2;
    
    int result = 0;
    
    /* Create complex control flow with multiple basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        int temp1 = v1 * v2 + v3 - v4;
        float temp2 = f1 * f2 + f3 - f4;
        double temp3 = d1 * d2 + d3 - d4;
        
        /* Call with many arguments to exceed register passing limits */
        int r1 = helper1(v1, v2, v3, v4, v5, 
                        temp1, temp1 + 1, temp1 + 2);
        
        /* Inline assembly that clobbers registers between computations */
        clobber_registers();
        
        /* More computations keeping values live across calls */
        temp1 = temp1 * 2 + r1;
        temp2 = temp2 * 1.5f + (float)r1;
        temp3 = temp3 * 1.25 + (double)r1;
        
        /* Conditional to create basic block boundaries */
        if (iteration % 2 == 0) {
            /* Second basic block */
            float r2 = helper2(f1, f2, f3, f4, f5,
                             temp2, temp2 + 1.0f, temp2 + 2.0f,
                             temp2 * 0.5f, temp2 * 0.75f);
            
            /* More register pressure */
            void* r3 = helper3((void*)p1, (void*)p2, (void*)p3,
                              temp1, r1, temp2, r2);
            
            /* Use alloca to affect frame pointer */
            int alloca_result = use_alloca(32 + iteration * 8);
            
            temp1 = temp1 + (int)(intptr_t)r3 + alloca_result;
            temp2 = temp2 + r2 + (float)alloca_result;
        } else {
            /* Third basic block (alternative path) */
            double r4 = helper4(d1, d2, d3, d4, d5,
                              temp3, temp3 * 0.9);
            
            /* Complex calculation with mixed types */
            int r5 = complex_calculation(temp1, temp2, temp3, (void*)&temp1);
            
            temp1 = temp1 + r5;
            temp2 = temp2 + (float)r4;
            temp3 = temp3 + r4;
        }
        
        /* Another clobber between calls */
        __asm__ volatile (
            "# More register clobbering\n\t"
            "mov $0xAAAAAAAA, %%eax\n\t"
            "mov $0xBBBBBBBB, %%ebx\n\t"
            "mov $0xCCCCCCCC, %%r10\n\t"
            "mov $0xDDDDDDDD, %%r11\n\t"
            :
            :
            : "eax", "ebx", "r10", "r11", "memory"
        );
        
        /* Final computations in the loop */
        result += temp1 + (int)temp2 + (int)temp3;
        
        /* Modify variables for next iteration */
        v1 = v1 * 3 % 17;
        v2 = v2 * 5 % 19;
        v3 = v3 * 7 % 23;
        f1 = f1 * 1.3f;
        f2 = f2 * 1.7f;
        d1 = d1 * 1.11;
        d2 = d2 * 1.22;
    }
    
    /* Additional control flow outside loop */
    {
        volatile int final_check = result;
        
        /* One more call with address taken variable */
        int* ptr = &final_check;
        int final_result = helper1(*ptr, *ptr + 1, *ptr + 2, *ptr + 3,
                                  *ptr + 4, *ptr + 5, *ptr + 6, *ptr + 7);
        
        /* Force spill/reload around this call */
        __asm__ volatile (
            "# Final clobber\n\t"
            "mov $0xFFFFFFFF, %%eax\n\t"
            "mov $0xEEEEEEEE, %%ecx\n\t"
            "mov $0xDDDDDDDD, %%edx\n\t"
            :
            :
            : "eax", "ecx", "edx", "memory"
        );
        
        result = final_result;
    }
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
