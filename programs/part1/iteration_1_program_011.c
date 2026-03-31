/* caller-save-test.c - ISO C99 compliant test for GCC caller-save register allocation */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force noinline to prevent unwanted optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with different signatures and calling conventions */
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

NOINLINE void* helper3(void* p1, void* p2, void* p3, void* p4,
                       void* p5, void* p6, void* p7, void* p8) {
    volatile uintptr_t sum = (uintptr_t)p1 + (uintptr_t)p2 + 
                            (uintptr_t)p3 + (uintptr_t)p4 +
                            (uintptr_t)p5 + (uintptr_t)p6 + 
                            (uintptr_t)p7 + (uintptr_t)p8;
    return (void*)(sum % 256);
}

NOINLINE double helper4(double a, double b, double c, double d,
                        double e, double f, double g, double h,
                        double i, double j, double k, double l) {
    volatile double result = a + b * c - d / e + f - g * h + i / j - k + l;
    return result;
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

/* Function with mixed register usage */
NOINLINE static long complex_calculation(int a, float b, double c, void* d) {
    volatile long result = a;
    result += (long)(b * 100.0f);
    result += (long)c;
    result += (long)(uintptr_t)d;
    
    /* Inline assembly to clobber registers */
    __asm__ volatile (
        "# Clobber caller-saved registers\n"
        "mov $0x12345678, %%eax\n"
        "mov $0x9ABCDEF0, %%edx\n"
        "mov $0x11111111, %%ecx\n"
        :
        : 
        : "eax", "edx", "ecx", "memory"
    );
    
    return result;
}

/* Main test function */
int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile void* p1 = (void*)0x1000, *p2 = (void*)0x2000;
    volatile void* p3 = (void*)0x3000, *p4 = (void*)0x4000;
    volatile long l1 = 0, l2 = 0, l3 = 0;
    volatile int checksum = 0;
    
    /* Take addresses to affect frame pointer decisions */
    int* addr_v1 = &v1;
    float* addr_f1 = &f1;
    
    /* Control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* First basic block with function calls */
            v1 = helper1(v1, v2, v3, v4, v5, v1, v2, v3);
            
            /* Inline assembly between computations to create live ranges */
            __asm__ volatile (
                "# Clobber more registers\n"
                "mov $0xAAAAAAAA, %%r10d\n"
                "mov $0xBBBBBBBB, %%r11d\n"
                :
                :
                : "r10", "r11", "memory"
            );
            
            f1 = helper2(f1, f2, f3, f4, f5, f1, f2, f3, f4, f5);
            
            /* More register pressure */
            d1 = d1 + d2 - d3 * d4 / d5;
            v2 = v2 * v3 - v4 + v5;
            
            p1 = helper3(p1, p2, p3, p4, p1, p2, p3, p4);
            
            /* Another inline assembly to force caller-save */
            __asm__ volatile (
                "# Clobber integer registers\n"
                "mov $0xCCCCCCCC, %%eax\n"
                "mov $0xDDDDDDDD, %%ebx\n"
                :
                :
                : "eax", "ebx", "memory"
            );
        } else {
            /* Second basic block with different calls */
            d2 = helper4(d1, d2, d3, d4, d5, d1, d2, d3, d4, d5, d1, d2);
            
            /* Complex computation keeping values live */
            f2 = f2 * f3 - f4 / f5 + f1;
            v3 = v3 + v4 * v5 - v1 + v2;
            
            /* Function using alloca affects frame pointer */
            v4 = use_alloca(v4 % 32 + 16);
            
            /* Mixed type function call */
            l1 = complex_calculation(v1, f1, d1, p1);
            
            /* More inline assembly */
            __asm__ volatile (
                "# Clobber floating point registers\n"
                "fldpi\n"
                "fstp %%st(0)\n"
                :
                :
                : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
            );
        }
        
        /* Cross-iteration dependencies to keep values live */
        v5 = v5 + iteration;
        f5 = f5 + iteration * 0.1f;
        d5 = d5 + iteration * 0.01;
        
        /* Pointer arithmetic */
        p2 = (void*)((uintptr_t)p2 + iteration * 16);
        p3 = (void*)((uintptr_t)p3 + iteration * 32);
        
        /* Compute intermediate checksum */
        checksum += v1 + (int)f1 + (int)d1 + (int)(uintptr_t)p1;
        checksum += v2 + (int)f2 + (int)d2 + (int)(uintptr_t)p2;
        checksum += v3 + (int)f3 + (int)d3 + (int)(uintptr_t)p3;
        checksum += v4 + (int)f4 + (int)d4 + (int)(uintptr_t)p4;
        checksum += v5 + (int)f5 + (int)d5;
        
        /* Additional control flow within loop */
        if (checksum % 7 == 0) {
            /* Nested basic block with another call */
            l2 = complex_calculation(v2, f2, d2, p2);
            
            /* More register pressure */
            f3 = helper2(f3, f4, f5, f1, f2, f3, f4, f5, f1, f2);
            
            __asm__ volatile (
                "# Final register clobber\n"
                "mov $0xEEEEEEEE, %%esi\n"
                "mov $0xFFFFFFFF, %%edi\n"
                :
                :
                : "esi", "edi", "memory"
            );
        }
    }
    
    /* Final computation using all variables */
    l3 = (long)v1 * (long)v2 * (long)v3 * (long)v4 * (long)v5;
    l3 += (long)(f1 * 1000.0f) + (long)(f2 * 1000.0f);
    l3 += (long)(d1 * 1000.0) + (long)(d2 * 1000.0);
    l3 += (long)(uintptr_t)p1 + (long)(uintptr_t)p2;
    l3 += (long)(uintptr_t)p3 + (long)(uintptr_t)p4;
    
    /* Use addresses to ensure they're needed */
    checksum += *addr_v1 + (int)*addr_f1;
    
    printf("Final checksum: %d\n", checksum);
    printf("Final long value: %ld\n", l3);
    
    return checksum != 0 ? 0 : 1;
}

/* Additional functions to increase compilation complexity */
NOINLINE static void extra_pressure1(int a, int b, int c, int d, int e) {
    volatile int r1 = a + b;
    volatile int r2 = c + d;
    volatile int r3 = e + r1;
    volatile int r4 = r2 + r3;
    
    __asm__ volatile (
        "# Extra clobber set 1\n"
        "mov $0x1234, %%ax\n"
        "mov $0x5678, %%bx\n"
        :
        :
        : "ax", "bx", "memory"
    );
    
    (void)r4;
}

NOINLINE static void extra_pressure2(float a, float b, float c, float d, float e) {
    volatile float r1 = a * b;
    volatile float r2 = c * d;
    volatile float r3 = e + r1;
    volatile float r4 = r2 + r3;
    
    __asm__ volatile (
        "# Extra clobber set 2\n"
        "mov $0x9ABC, %%cx\n"
        "mov $0xDEF0, %%dx\n"
        :
        :
        : "cx", "dx", "memory"
    );
    
    (void)r4;
}
