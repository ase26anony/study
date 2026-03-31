/* caller-save-test.c - ISO C99 compliant test for GCC caller-save pass */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

NOINLINE static void* helper3(void* p1, void* p2, void* p3, 
                              void* p4, void* p5, void* p6) {
    volatile uintptr_t sum = (uintptr_t)p1 + (uintptr_t)p2 + 
                            (uintptr_t)p3 + (uintptr_t)p4 + 
                            (uintptr_t)p5 + (uintptr_t)p6;
    return (void*)(sum % 256);
}

NOINLINE static double helper4(double a, double b, double c, double d,
                               double e, double f, double g) {
    volatile double result = a / (b + 1.0) * c - d * e + f / g;
    return result;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static int helper5(int size) {
    char* buffer = (char*)alloca(size % 128 + 16);
    volatile int sum = 0;
    for (int i = 0; i < size % 128 + 16; i++) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    return sum;
}

/* External function to force call through PLT */
NOINLINE int external_helper(int x, int y, int z);

/* Simulated external function */
NOINLINE int external_helper(int x, int y, int z) {
    volatile int result = x * y + z;
    return result;
}

int main(void) {
    /* Many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    volatile void* p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4;
    volatile int result = 0;
    
    /* Take addresses to force stack usage and affect frame pointer */
    int* addr_array[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    /* Complex control flow creating multiple basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        v3 = v3 + v1 - v2;
        v4 = v4 ^ v3;
        
        /* Inline assembly to clobber specific registers */
        /* For x86_64 - clobber multiple call-clobbered registers */
        __asm__ volatile (
            "# Force clobber of call-clobbered registers\n"
            "mov $0x12345678, %%eax\n"
            "mov $0x87654321, %%edx\n"
            "mov $0x11111111, %%ecx\n"
            "mov $0x22222222, %%r10\n"
            "mov $0x33333333, %%r11\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "edx", "ecx", "r10", "r11", "memory"
        );
        
        /* Function call with many arguments - forces register pressure */
        int r1 = helper1(v1, v2, v3, v4, v5, v6, v7, v8);
        
        /* More computations keeping values live in registers */
        f1 = f1 * 1.5f + (float)r1;
        f2 = f2 / 1.5f - (float)v1;
        f3 = f3 + f1 - f2;
        
        /* Another inline assembly clobber */
        __asm__ volatile (
            "# Another clobber sequence\n"
            "mov $0xAAAAAAAA, %%r8\n"
            "mov $0xBBBBBBBB, %%r9\n"
            "mov $0xCCCCCCCC, %%rsi\n"
            "mov $0xDDDDDDDD, %%rdi\n"
            : /* no outputs */
            : /* no inputs */
            : "r8", "r9", "rsi", "rdi", "memory"
        );
        
        /* Conditional creating new basic block */
        if (iteration % 2 == 0) {
            /* Float function with many arguments */
            float r2 = helper2(f1, f2, f3, f4, f5, 
                              f1 * 2.0f, f2 / 2.0f, f3 + 1.0f,
                              f4 - 1.0f, f5 * 0.5f);
            
            /* Pointer function */
            void* r3 = helper3(p1, p2, p3, p4, &f1, &f2);
            
            /* Mix computations */
            v5 = v5 + (int)(intptr_t)r3;
            f4 = f4 + r2;
            
            /* External function call */
            v6 = external_helper(v5, v6, (int)r2);
        } else {
            /* Different path with double precision */
            double r4 = helper4(d1, d2, d3, d4, d1 * 1.1, d2 / 1.1, d3 + 1.1);
            
            /* Function with alloca affecting frame pointer */
            v7 = helper5(v7 + iteration);
            
            d4 = d4 + r4;
            v8 = v8 * 2 - (int)d4;
            
            /* More inline assembly */
            __asm__ volatile (
                "# Third clobber set\n"
                "mov $0xEEEEEEEE, %%rax\n"
                "mov $0xFFFFFFFF, %%rbx\n"
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "memory"
            );
        }
        
        /* Common tail computations */
        v9 = v9 + v1 + v2 + v3 + v4;
        v10 = v10 ^ v9;
        
        /* Another function call in tail position */
        int r5 = helper1(v9, v10, v1, v2, v3, v4, v5, v6);
        
        /* Final computations with all variables */
        result += r5 + v7 + v8 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)d1;
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Additional loop with nested conditionals */
    for (int i = 0; i < 5; i++) {
        volatile int temp = i * 17;
        
        if (temp % 3 == 0) {
            v1 = helper1(temp, v2, v3, v4, v5, v6, v7, v8);
            __asm__ volatile (
                "# Loop clobber\n"
                "mov $0x55555555, %%r12\n"
                "mov $0x66666666, %%r13\n"
                : /* no outputs */
                : /* no inputs */
                : "r12", "r13", "memory"
            );
        } else if (temp % 3 == 1) {
            f1 = helper2(f1, f2, f3, f4, f5, 
                        (float)temp, (float)v1, (float)v2,
                        (float)v3, (float)v4);
        } else {
            helper3(&temp, &v1, &v2, &v3, &v4, &v5);
        }
        
        /* Complex expression keeping many values live */
        result = result + v1 * 3 - v2 / 2 + v3 ^ v4 | v5 & v6;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    printf("Values: %d %d %d %d %d\n", v1, v2, v3, v4, v5);
    printf("Floats: %.2f %.2f %.2f\n", f1, f2, f3);
    
    return result != 0;
}
